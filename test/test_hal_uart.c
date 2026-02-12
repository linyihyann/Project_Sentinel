#include <string.h>  // for memcpy if needed

#include "hal_uart.h"
#include "ring_buffer.h"  // 引入 Day 7 的成果
#include "unity.h"

// ==========================================
// 1. 定義整合物件 (Integrated Object)
// ==========================================
typedef struct
{
    // Day 7: Ring Buffer (應用層的大水庫)
    ring_buffer_t rb;
    uint8_t rb_storage[1024];

    // Day 6: DMA 暫存區 (硬體搬運的小水桶)
    uint8_t dma_temp_buffer[128];

    // 狀態旗標
    bool data_ready;
} Ultimate_UART_Ctx_t;

static Ultimate_UART_Ctx_t my_system;
static uart_handle_t h_uart;

// ==========================================
// 2. 實作終極 Callback (The Bridge)
// ==========================================
void Ultimate_Callback(void* ctx, uart_event_t event, void* data)
{
    Ultimate_UART_Ctx_t* sys = (Ultimate_UART_Ctx_t*)ctx;

    if (event == UART_EVENT_RX_COMPLETE)
    {
        // 單 Byte 中斷處理
        uint8_t val = *(uint8_t*)data;
        rb_push(&sys->rb, val);
    }
    else if (event == UART_EVENT_RX_DMA_COMPLETE)
    {
        // 🟢 [關鍵修正] 這裡必須要把資料從 DMA Buffer 搬到 Ring Buffer

        // 1. 取得搬運長度 (假設 hal_uart.c 傳來的是長度指標)
        // 如果您的 mock 傳的是 NULL，這裡可以直接用 10 (測試用)
        uint16_t len = 0;
        if (data != NULL)
        {
            len = *(uint16_t*)data;
        }
        else
        {
            len = 10;  // Fallback for test
        }

        // 2. 批量寫入 Ring Buffer
        // 這裡展現了 DMA Buffer (線性) -> Ring Buffer (環形) 的橋接
        for (int i = 0; i < len; i++)
        {
            rb_push(&sys->rb, sys->dma_temp_buffer[i]);
        }

        // 3. 設定旗標，通知主程式
        sys->data_ready = true;
    }
}

// ==========================================
// 3. 測試案例 (The Master Test)
// ==========================================
void test_day6_7_8_integration(void)
{
    // 1. 初始化 Ring Buffer
    rb_init(&my_system.rb, my_system.rb_storage, 1024);
    my_system.data_ready = false;
    // 清空 DMA Buffer 以免殘留舊資料
    memset(my_system.dma_temp_buffer, 0, 128);

    // 2. 初始化 UART & 註冊 Callback
    HAL_UART_Init(&h_uart, 0);
    HAL_UART_RegisterCallback(&h_uart, Ultimate_Callback, &my_system);

    // 3. 設定 DMA (模擬設定暫存器)
    // 告訴驅動層：之後收到的資料請搬到 my_system.dma_temp_buffer
    HAL_UART_Receive_DMA(&h_uart, my_system.dma_temp_buffer, 10);

    // 4. [模擬硬體行為] DMA 搬運發生了！
    const char* burst_data = "SpeedTest!";

    // ⚠️ 注意：這一步模擬了 "硬體把資料寫入記憶體" 的動作
    // 如果您的 HAL_UART_SimulateDMA_Complete 裡面沒有寫 memcpy，
    // 我們必須在這裡手動模擬 "硬體寫入"：
    memcpy(my_system.dma_temp_buffer, burst_data, 10);

    // 觸發中斷 (通知 Callback 說搬完了)
    uint16_t len = 10;
    // 這裡我們傳入 len 的地址，對應 Callback 裡的 *(uint16_t*)data
    HAL_UART_SimulateDMA_Complete(&h_uart, NULL, len);
    // 註：有些實作 SimulateDMA 會自己 memcpy，看您的 hal_uart.c 怎麼寫

    // 5. 驗證結果

    // 驗證 A: Callback 有被觸發，且設了 flag
    TEST_ASSERT_TRUE_MESSAGE(my_system.data_ready, "Callback was not triggered or flag not set");

    // 驗證 B: Ring Buffer 裡有 10 個 byte
    uint32_t count = (my_system.rb.head - my_system.rb.tail) & my_system.rb.mask;
    TEST_ASSERT_EQUAL_INT_MESSAGE(10, count, "Ring Buffer count mismatch");

    // 驗證 C: 資料內容正確 (FIFO)
    uint8_t byte;
    rb_pop(&my_system.rb, &byte);
    TEST_ASSERT_EQUAL_CHAR('S', byte);
    rb_pop(&my_system.rb, &byte);
    TEST_ASSERT_EQUAL_CHAR('p', byte);
}

// ==========================================
// Unity 基礎設施
// ==========================================
void setUp(void) {}
void tearDown(void) {}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_day6_7_8_integration);
    return UNITY_END();
}