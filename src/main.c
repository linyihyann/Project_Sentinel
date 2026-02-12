#include <stdio.h>
#include <string.h>  //  修正：支援 strlen 函式

#include "hal_uart.h"
#include "pico/stdlib.h"
#include "ring_buffer.h"

//  修正：處理 RP2350 (Pico 2) 可能缺失的 LED 定義
#ifndef PICO_DEFAULT_LED_PIN
#define LED_PIN 25  // 預設 GPIO 25
#else
#define LED_PIN PICO_DEFAULT_LED_PIN
#endif

// 定義系統物件
typedef struct
{
    ring_buffer_t rb;
    uint8_t storage[256];  // 256 bytes buffer
} System_Ctx_t;

static System_Ctx_t sys_ctx;
static uart_handle_t h_uart;

// ==========================================
// Callback 實作 (Day 8 核心)
// ==========================================
//  當 UART 收到資料 (硬體 ISR) -> 呼叫這裡
void My_UART_Callback(void* ctx, uart_event_t event, void* data)
{
    System_Ctx_t* sys = (System_Ctx_t*)ctx;

    if (event == UART_EVENT_RX_COMPLETE)
    {
        //  修正：從 void* data 解引用取得正確數值並存入 Ring Buffer
        uint8_t actual_data = *(uint8_t*)data;
        rb_push(&sys->rb, actual_data);
    }
}

// ==========================================
// 主程式
// ==========================================
int main()
{
    // 1. 初始化 stdio (為了可以用 printf debug)
    stdio_init_all();
    sleep_ms(2000);  // 等待 USB 連線穩定
    printf("=== Day 8 Loopback Test Start ===\n");

    //  2. 初始化 Ring Buffer (修正：改用 rb_init)
    rb_init(&sys_ctx.rb, sys_ctx.storage, 256);

    // 3. 初始化 UART 並註冊 Callback
    HAL_UART_Init(&h_uart, 0);
    HAL_UART_RegisterCallback(&h_uart, My_UART_Callback, &sys_ctx);

    //  4. 設定 LED (用來當心跳燈)
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // 5. 主迴圈 (Loopback Verification)
    int send_counter = 0;
    while (true)
    {
        // --- 傳送端 (TX) ---
        char tx_msg[32];
        sprintf(tx_msg, "Ping %d\n", send_counter++);

        printf("[TX] Sending: %s", tx_msg);
        //  注意：確保 hal_uart.h 內已宣告 HAL_UART_Send
        HAL_UART_Send(&h_uart, (uint8_t*)tx_msg, (uint16_t)strlen(tx_msg));

        // --- 接收端 (RX) 檢查 ---
        sleep_ms(100);  // 等待硬體傳輸與 ISR 處理

        //  修正：改用 rb_is_empty 檢查
        if (!rb_is_empty(&sys_ctx.rb))
        {
            printf("[RX] Received: ");
            uint8_t rx_byte;
            //  修正：改用 rb_pop 取出資料
            while (rb_pop(&sys_ctx.rb, &rx_byte))
            {
                putchar(rx_byte);
            }

            // 閃爍 LED
            gpio_put(LED_PIN, 1);
            sleep_ms(50);
            gpio_put(LED_PIN, 0);
        }
        else
        {
            printf("[RX] No Data! (Check wiring GP0 to GP1)\n");
        }

        sleep_ms(900);
    }
}