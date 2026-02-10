/* src/main.c */
#include <stdio.h>

#include "pico/stdlib.h"
#include "ring_buffer.h"

// 定義 Buffer 大小，必須是 2 的冪次方 (例如 32, 64, 128)
#define BUFFER_SIZE 32

// 全域變數
uint8_t raw_buffer[BUFFER_SIZE];
ring_buffer_t rb;

// 統計數據
volatile uint32_t tx_count = 0;
volatile uint32_t rx_count = 0;
volatile uint32_t drop_count = 0;

// --- 模擬 ISR (生產者) ---
// 這是一個 Timer Callback，代表外部中斷 (如 UART Rx)
bool timer_producer_callback(struct repeating_timer* t)
{
    static uint8_t counter = 0;

    // 嘗試將數據推入 Ring Buffer
    if (rb_push(&rb, counter))
    {
        tx_count++;
        counter++;  // 只有寫入成功才換下一個數字
    }
    else
    {
        drop_count++;  // Buffer 滿了，資料遺失 (這是預期行為，我們想觀察這個)
    }
    return true;  // 繼續重複 Timer
}

// --- Main (消費者) ---
int main()
{
    stdio_init_all();

    // 等待 USB 連接 (方便觀察)
    while (!stdio_usb_connected())
    {
        sleep_ms(100);
    }
    sleep_ms(2000);
    printf("=== Day 7: Lock-Free Ring Buffer Stress Test ===\n");

    // 1. 初始化
    if (!rb_init(&rb, raw_buffer, BUFFER_SIZE))
    {
        printf("[FATAL] Ring Buffer Init Failed! Check Size.\n");
        while (1);
    }
    printf("[OK] Buffer Initialized. Size: %d\n", BUFFER_SIZE);

    // 2. 啟動 Timer (每 50ms 產生一筆資料) -> 模擬高速 ISR
    struct repeating_timer timer;
    add_repeating_timer_ms(50, timer_producer_callback, NULL, &timer);
    printf("[OK] Producer Timer Started (50ms interval).\n");

    uint8_t data;
    while (true)
    {
        // 3. 消費者讀取
        // 我們故意加一點延遲，模擬主程式很忙碌，看看 Buffer 會不會爆

        if (rb_pop(&rb, &data))
        {
            rx_count++;
            // 可以在這裡印出 data，但太快了會洗版，我們改用統計數據
            // printf("Read: %d\n", data);
        }

        // 每秒印一次統計報告
        static uint64_t last_report_time = 0;
        uint64_t now = time_us_64();
        if (now - last_report_time > 1000000)
        {
            last_report_time = now;

            printf("Stats -> TX: %u | RX: %u | Drop: %u | Head: %u | Tail: %u\n", tx_count,
                   rx_count, drop_count, rb.head, rb.tail);

            // 如果你想測試 Drop，可以把下面的 sleep 打開，讓消費者變慢
            // sleep_ms(200);
        }

        // 讓 CPU 稍微休息，但在真實高負載下通常不 sleep
        sleep_ms(1);
    }
}