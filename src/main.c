/* src/main.c */
#include <stdio.h>
#include <string.h>

#include "hal/hal_uart_dma.h"
#include "pico/stdlib.h"

// 這是我們期待收到的測試字串
const char* TEST_PATTERN = "Sentinel V6.0 DMA Loopback Test - Alive!";
#define TEST_LEN 40  // 字串長度

int main()
{
    // 初始化 USB stdio，讓我們可以在電腦螢幕看到 printf
    stdio_init_all();

    // 等待 USB 連接，不然開機訊息會看不到 (開發階段專用)
    // 在真實車用韌體中不能這樣死等
    while (!stdio_usb_connected())
    {
        sleep_ms(100);
    }

    sleep_ms(2000);
    printf("=== Sentinel System DMA Self-Test ===\n");

    // 1. 初始化 DMA HAL
    if (!hal_uart_dma_init())
    {
        printf("[FATAL] DMA Init Failed!\n");
        while (1) tight_loop_contents();
    }
    printf("[OK] DMA Initialized.\n");
    printf("Please ensure GP0 is connected to GP1 (Loopback).\n");

    uint8_t rx_buf[128];
    uint32_t pass_count = 0;
    uint32_t error_count = 0;

    while (true)
    {
        memset(rx_buf, 0, sizeof(rx_buf));

        // --- 步驟 A: 發送測試資料 (Stimulus) ---
        // 這裡我們主動發送資料，而不是等待
        hal_uart_dma_send((const uint8_t*)TEST_PATTERN, strlen(TEST_PATTERN));

        // 給一點時間讓電子訊號在線路上跑 (其實 DMA 很快，但在 Loop 裡我們慢一點方便觀察)
        sleep_ms(100);

        // --- 步驟 B: 檢查接收緩衝區 (Verification) ---
        size_t len = hal_uart_dma_read(rx_buf, sizeof(rx_buf));

        if (len > 0)
        {
            // 加上字串結束符號以便 printf 顯示
            // 注意：這是為了 Demo 方便，嚴謹的二進制處理不應依賴 Null Terminator
            if (len < sizeof(rx_buf))
                rx_buf[len] = '\0';
            else
                rx_buf[sizeof(rx_buf) - 1] = '\0';

            // 比對資料是否正確
            if (strncmp((char*)rx_buf, TEST_PATTERN, strlen(TEST_PATTERN)) == 0)
            {
                pass_count++;
                printf("[PASS %u] Received: %s\n", pass_count, rx_buf);
            }
            else
            {
                error_count++;
                printf("[FAIL %u] Data Mismatch! Got: %s\n", error_count, rx_buf);
            }
        }
        else
        {
            // 如果發送了卻沒收到，通常是線沒接好
            printf("[WARN] No Data Received. Check Jump Wire (GP0 <-> GP1).\n");
        }

        sleep_ms(1000);  // 每秒測試一次
    }
}