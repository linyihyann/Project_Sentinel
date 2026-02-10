/* src/hal/hal_uart_dma.c */
#include "hal_uart_dma.h"

#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"

// --- 硬體參數設定 ---
#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0  // 請確認你的腳位
#define UART_RX_PIN 1

// --- 靜態變數 (內部狀態) ---
static int dma_tx_chan = -1;
static int dma_rx_chan = -1;

// RX 環形緩衝區：必須對齊其大小 (256 bytes)
static uint8_t __attribute__((aligned(UART_DMA_BUFFER_SIZE))) rx_ring_buffer[UART_DMA_BUFFER_SIZE];

// 軟體讀取指標 (追蹤我們讀到哪裡了)
static uint32_t rx_read_index = 0;

bool hal_uart_dma_init(void)
{
    // 1. 初始化 UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // 關鍵：關閉 Flow Control，開啟 FIFO (DMA 需要 FIFO DREQ 訊號)
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART_ID, true);

    // 2. 申請 DMA 通道
    dma_tx_chan = dma_claim_unused_channel(false);
    dma_rx_chan = dma_claim_unused_channel(false);

    if (dma_tx_chan < 0 || dma_rx_chan < 0) return false;

    // 3. 設定 RX DMA (Circular Mode)
    dma_channel_config c_rx = dma_channel_get_default_config(dma_rx_chan);
    channel_config_set_transfer_data_size(&c_rx, DMA_SIZE_8);
    channel_config_set_read_increment(&c_rx, false);                // 讀 UART FIFO (固定地址)
    channel_config_set_write_increment(&c_rx, true);                // 寫 RAM (遞增地址)
    channel_config_set_dreq(&c_rx, uart_get_dreq(UART_ID, false));  // 當 UART 有資料時觸發

    // 設定 Ring Buffer: 大小為 2^8 = 256 bytes
    channel_config_set_ring(&c_rx, true, 8);

    // 啟動 RX DMA (無限循環接收)
    dma_channel_configure(dma_rx_chan, &c_rx,
                          rx_ring_buffer,             // 寫入目的地
                          &uart_get_hw(UART_ID)->dr,  // 讀取來源 (UART Data Register)
                          UINT32_MAX,                 // 傳輸次數 (無限)
                          true                        // 立即啟動
    );

    return true;
}

void hal_uart_dma_send(const uint8_t* data, size_t len)
{
    // 簡單防護：如果上次還沒傳完，這裡可以選擇等待或捨棄
    // 實務上我們會用另一個 Ring Buffer 做 Queue
    dma_channel_wait_for_finish_blocking(dma_tx_chan);

    dma_channel_config c_tx = dma_channel_get_default_config(dma_tx_chan);
    channel_config_set_transfer_data_size(&c_tx, DMA_SIZE_8);
    channel_config_set_read_increment(&c_tx, true);                // 讀 RAM
    channel_config_set_write_increment(&c_tx, false);              // 寫 UART FIFO
    channel_config_set_dreq(&c_tx, uart_get_dreq(UART_ID, true));  // 當 UART 可寫時觸發

    dma_channel_configure(dma_tx_chan, &c_tx,
                          &uart_get_hw(UART_ID)->dr,  // 寫入目的地
                          data,                       // 讀取來源
                          len,                        // 長度
                          true                        // 啟動
    );
}

size_t hal_uart_dma_read(uint8_t* buffer, size_t max_len)
{
    // 取得 DMA 目前寫到哪裡了 (Hardware Write Pointer)
    // hw_addr->write_addr 會回傳絕對位址，我們需要轉成相對 index
    uintptr_t current_write_addr = (uintptr_t)dma_channel_hw_addr(dma_rx_chan)->write_addr;
    uintptr_t start_addr = (uintptr_t)rx_ring_buffer;

    // 計算硬體目前的 index (Producer Index)
    uint32_t rx_write_index = (current_write_addr - start_addr) % UART_DMA_BUFFER_SIZE;

    size_t count = 0;

    // 當 讀取指標 != 寫入指標，代表有新資料
    while (rx_read_index != rx_write_index && count < max_len)
    {
        buffer[count++] = rx_ring_buffer[rx_read_index];

        // 移動讀取指標 (Wrap around)
        rx_read_index = (rx_read_index + 1) % UART_DMA_BUFFER_SIZE;
    }

    return count;
}