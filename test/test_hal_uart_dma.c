#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "unity.h"

// ==========================================
// 1. MOCK Definitions (模擬 Pico SDK)
// ==========================================

// 補上缺失的 Macro
#define UART_ID uart0
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define BAUD_RATE 115200

#define GPIO_FUNC_UART 2
#define UART_PARITY_NONE 0
#define DMA_SIZE_8 0

// [修正 1] 定義硬體結構，包含 dr (Data Register)
typedef struct
{
    volatile uint32_t dr;
} uart_hw_t;

typedef struct
{
    uint32_t write_addr;
} dma_channel_hw_t;

typedef struct
{
} uart_inst_t;
typedef struct
{
} dma_channel_config;

// 定義假變數
static uart_inst_t mock_uart_inst;
#define uart0 (&mock_uart_inst)

// 模擬硬體實體
dma_channel_hw_t mock_dma_hw[12];
static uart_hw_t mock_uart_hw_regs;  // [修正 1] 假的 UART 硬體暫存器

// 追蹤變數
int mock_dma_claim_count = 0;
bool mock_uart_fifo_enabled = false;

// ==========================================
// 2. STUB Functions (假函式實作)
// ==========================================

// [DMA 相關]
int dma_claim_unused_channel(bool required)
{
    return mock_dma_claim_count++;
}

dma_channel_hw_t* dma_channel_hw_addr(int channel)
{
    return &mock_dma_hw[channel];
}

dma_channel_config dma_channel_get_default_config(int channel)
{
    dma_channel_config c;
    return c;
}

void channel_config_set_transfer_data_size(dma_channel_config* c, int size) {}
void channel_config_set_read_increment(dma_channel_config* c, bool incr) {}
void channel_config_set_write_increment(dma_channel_config* c, bool incr) {}
void channel_config_set_dreq(dma_channel_config* c, int dreq) {}
void channel_config_set_ring(dma_channel_config* c, bool write, int ring_size) {}

void dma_channel_configure(int chan, dma_channel_config* c, void* write, const void* read,
                           int count, bool trigger)
{
    // Mock: 這裡可以做參數檢查
}

void dma_channel_wait_for_finish_blocking(int chan) {}

// [UART 相關]
void uart_init(uart_inst_t* uart, int baud) {}
void gpio_set_function(int gpio, int func) {}
void uart_set_hw_flow(uart_inst_t* uart, bool cts, bool rts) {}
void uart_set_format(uart_inst_t* uart, int data, int stop, int parity) {}

void uart_set_fifo_enabled(uart_inst_t* uart, bool enabled)
{
    mock_uart_fifo_enabled = enabled;
}

// [修正 2] 回傳 uart_hw_t* 而不是 void*
uart_hw_t* uart_get_hw(uart_inst_t* uart)
{
    return &mock_uart_hw_regs;
}

int uart_get_dreq(uart_inst_t* uart, bool is_tx)
{
    return 0;
}

// ==========================================
// 3. Source Inclusion (白箱測試)
// ==========================================
// 因為我們要測試內部 static 變數，所以直接 include .c 檔
// 但為了消除 "Warning: cast to smaller integer type"，我們在 include 前定義 uintptr_t 補丁
// 其實最好的解法是修改 src/hal/hal_uart_dma.c 使用 uintptr_t，但為了不改動 source，我們這裡忽略警告
#include "../src/hal/hal_uart_dma.c"

// ==========================================
// 4. Test Cases
// ==========================================

void setUp(void)
{
    mock_dma_claim_count = 0;
    mock_uart_fifo_enabled = false;
    rx_read_index = 0;
    memset(rx_ring_buffer, 0, UART_DMA_BUFFER_SIZE);
}

void tearDown(void) {}

void test_init_should_configure_peripherals(void)
{
    bool result = hal_uart_dma_init();
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(2, mock_dma_claim_count);
    TEST_ASSERT_TRUE(mock_uart_fifo_enabled);
}

void test_read_logic(void)
{
    hal_uart_dma_init();

    // 模擬 DMA 寫入了 3 個 bytes
    rx_ring_buffer[0] = 'H';
    rx_ring_buffer[1] = 'i';
    rx_ring_buffer[2] = '!';

    // [修正 3] 使用 uintptr_t 避免 macOS 64-bit 指標警告
    mock_dma_hw[dma_rx_chan].write_addr = (uintptr_t)rx_ring_buffer + 3;

    uint8_t buf[10];
    size_t len = hal_uart_dma_read(buf, 10);

    TEST_ASSERT_EQUAL_INT(3, len);
    TEST_ASSERT_EQUAL_UINT8('H', buf[0]);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_init_should_configure_peripherals);
    RUN_TEST(test_read_logic);
    return UNITY_END();
}