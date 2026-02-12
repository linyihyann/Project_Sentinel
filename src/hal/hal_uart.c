#include "hal_uart.h"

#include <stddef.h>  // for NULL

// ==========================================
// 1. 硬體相依性隔離 (Include)
// ==========================================
#ifndef TEST_MODE
#include "hardware/irq.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"
#else
// 在測試模式下，我們需要自己定義這些常數，避免編譯錯誤
#define UART_ID 0
#define UART_PARITY_NONE 0
#endif

// ==========================================
// 2. 變數與 ISR 定義
// ==========================================
static uart_handle_t* g_uart0_handle = NULL;

// ISR 只在非測試模式下編譯，或者在測試模式下作為空函式
#ifndef TEST_MODE
void on_uart_rx()
{
    while (uart_is_readable(uart0))
    {
        uint8_t ch = uart_getc(uart0);
        if (g_uart0_handle && g_uart0_handle->callback)
        {
            g_uart0_handle->callback(g_uart0_handle->user_ctx, UART_EVENT_RX_COMPLETE, &ch);
        }
    }
}
#endif

// ==========================================
// 3. 函式實作 (Implementation)
// ==========================================

void HAL_UART_Init(uart_handle_t* h, uint8_t id)
{
    if (h == NULL) return;
    h->id = id;
    h->callback = NULL;
    h->user_ctx = NULL;
    g_uart0_handle = h;

    // 🟢 關鍵：只有在韌體模式下才呼叫硬體初始化
#ifndef TEST_MODE
    uart_init(uart0, 115200);
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);
    uart_set_hw_flow(uart0, false, false);
    uart_set_format(uart0, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(uart0, false);

    irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enables(uart0, true, false);
#endif
}

void HAL_UART_RegisterCallback(uart_handle_t* h, uart_callback_t cb, void* ctx)
{
    if (h)
    {
        h->callback = cb;
        h->user_ctx = ctx;
    }
}

void HAL_UART_Send(uart_handle_t* h, const uint8_t* data, uint16_t len)
{
#ifndef TEST_MODE
    uart_write_blocking(uart0, data, len);
#endif
}

void HAL_UART_Receive_DMA(uart_handle_t* h, uint8_t* pData, uint16_t Size)
{
#ifdef TEST_MODE
    // 在測試模式下，我們只需要記錄「使用者想把資料搬到哪裡」
    // 假設 uart_handle_t 結構裡有這些模擬欄位
    // 如果沒有，您需要在 hal_uart.h 的 struct 裡補上：
    // uint8_t* dma_rx_buffer;
    // uint16_t dma_rx_len;

    // 這裡為了讓 Linker 過關，如果您還沒加欄位，可以先留空，
    // 但為了功能驗證，建議補上：
    // h->dma_rx_buffer = pData;
    // h->dma_rx_len = Size;
#else
    // 真實硬體實作 (呼叫 dma_channel_config 等等)
    // 這裡可以先留空，或是呼叫 Pico SDK
#endif
}

// 模擬 DMA 完成中斷 (測試用)
#ifdef TEST_MODE
void HAL_UART_SimulateDMA_Complete(uart_handle_t* h, const uint8_t* mock_data, uint16_t len)
{
    // 1. 模擬硬體搬運 (memcpy)
    // if (h->dma_rx_buffer) memcpy(h->dma_rx_buffer, mock_data, len);

    // 2. 觸發 Callback
    if (h && h->callback)
    {
        // 注意：這裡傳入 len 的地址，對應測試裡的 *(uint16_t*)data
        h->callback(h->user_ctx, UART_EVENT_RX_DMA_COMPLETE, &len);
    }
}
#endif