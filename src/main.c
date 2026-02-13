#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"

// 引入各層模組
#include "hal_i2c.h"
#include "hal_uart.h"
#include "ring_buffer.h"
#include "sentinel_core.h"
#include "ssd1306_basic.h"

#ifndef PICO_DEFAULT_LED_PIN
#define LED_PIN 25
#else
#define LED_PIN PICO_DEFAULT_LED_PIN
#endif

typedef struct
{
    ring_buffer_t rx_rb;
    uint8_t storage[256];
} System_Ctx_t;

static System_Ctx_t sys_ctx;
static uart_handle_t h_uart;
static bool oled_is_inverted = false;

// UART RX ISR (監聽硬體 GP1)
void My_UART_Callback(void* ctx, uart_event_t event, void* data)
{
    System_Ctx_t* sys = (System_Ctx_t*)ctx;
    if (event == UART_EVENT_RX_COMPLETE)
    {
        uint8_t rx_byte = *(uint8_t*)data;
        rb_push(&sys->rx_rb, rx_byte);
    }
}

// 統一的指令處理函式
void Process_Command(SystemCmd_t cmd, uint32_t now)
{
    if (cmd == CMD_OLED_INVERT)
    {
        oled_is_inverted = true;
        printf("\n[APP] ✅ Command Executed: OLED INVERT\n");
    }
    else if (cmd == CMD_OLED_NORMAL)
    {
        oled_is_inverted = false;
        printf("\n[APP] ✅ Command Executed: OLED NORMAL\n");
    }
    else if (cmd == CMD_SYSTEM_PING)
    {
        printf("\n[APP] 🚀 System Alive! Uptime: %u ms\n", now);
    }
}

int main()
{
    stdio_init_all();
    sleep_ms(3000);  // 多等一下，讓你來得及開 Serial Monitor
    printf("\n\n==========================================\n");
    printf("🚀 Project Sentinel: Ultimate Integration\n");
    printf("👉 Type 'INV' or 'NORM' or 'PING' below:\n");
    printf("==========================================\n");

    rb_init(&sys_ctx.rx_rb, sys_ctx.storage, 256);
    HAL_UART_Init(&h_uart, 0);
    HAL_UART_RegisterCallback(&h_uart, My_UART_Callback, &sys_ctx);

    hal_i2c_init();
    ssd1306_init();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    uint32_t last_oled_time = 0;
    uint32_t last_heartbeat_time = 0;
    int oled_x_pos = 0;

    while (true)
    {
        uint32_t now = to_ms_since_boot(get_absolute_time());

        // ---------------------------------------------------
        // Task 1: 系統心跳 (每秒印一個點，證明沒當機)
        // ---------------------------------------------------
        if (now - last_heartbeat_time >= 1000)
        {
            last_heartbeat_time = now;
            printf(".");  // 輸出心跳
        }

        // ---------------------------------------------------
        // Task 2: 監聽 USB 鍵盤輸入 (USB CDC Bridge)
        // ---------------------------------------------------
        int usb_char = getchar_timeout_us(0);  // 非阻塞讀取 USB 鍵盤
        if (usb_char != PICO_ERROR_TIMEOUT)
        {
            // 💡 照妖鏡：印出你按下的每一個按鍵的 ASCII Hex 碼
            printf("[Key: %c (0x%02X)]", usb_char, usb_char);

            SystemCmd_t cmd = Sentinel_ParseChar((char)usb_char);
            Process_Command(cmd, now);
        }

        // ---------------------------------------------------
        // Task 3: 監聽硬體 UART (Day 8 Ring Buffer)
        // ---------------------------------------------------
        uint8_t rx_byte;
        if (rb_pop(&sys_ctx.rx_rb, &rx_byte))
        {
            SystemCmd_t cmd = Sentinel_ParseChar((char)rx_byte);
            Process_Command(cmd, now);
        }

        // ---------------------------------------------------
        // Task 4: OLED 動畫與防護
        // ---------------------------------------------------
        if (now - last_oled_time >= 20)
        {
            last_oled_time = now;
            ssd1306_clear();

            uint8_t pattern = oled_is_inverted ? 0xFF : 0x00;
            if (oled_is_inverted) ssd1306_fill(pattern);

            for (int y = 0; y < 32; y++)
            {
                ssd1306_draw_pixel(oled_x_pos, y, !oled_is_inverted);
            }
            ssd1306_show();
            oled_x_pos = (oled_x_pos + 1) % 128;
        }
    }
}