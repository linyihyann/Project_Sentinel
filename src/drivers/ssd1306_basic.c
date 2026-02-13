/**
 * @file ssd1306_basic.c
 * @brief SSD1306 OLED Driver (Decoupled & Resilient Version)
 * @note 已經將底層寫入替換為具備 Timeout 與 Recovery 的 hal_i2c_write_safe
 */

#include "ssd1306_basic.h"

#include <string.h>  // for memset, memcpy

#include "hal_i2c.h"  // ✨ 關鍵引入：依賴我們自己的 HAL，而不是硬體 SDK

// 螢幕 Buffer: 128 * 32 / 8 = 512 bytes
static uint8_t buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

// 寫入指令輔助函式 (封裝了底層的安全寫入)
static void write_cmd(uint8_t cmd)
{
    uint8_t data[2] = {0x00, cmd};  // 0x00 = Co=0, D/C#=0 (Command)

    // ✨ 替換點 1：使用具備 Timeout 與 Recovery 的安全函式
    hal_i2c_write_safe(SSD1306_ADDR, data, 2);
}

void ssd1306_init(void)
{
    // 註：因為我們的 hal_i2c 內部已經綁定了 port，
    // 這裡傳入的 i2c 參數其實可以留作未來擴充 (或保持相容性)

    // 標準初始化序列 (針對 128x32)
    write_cmd(0xAE);  // Display OFF

    write_cmd(0xD5);  // Set Display Clock Divide Ratio
    write_cmd(0x80);  // Default 0x80

    write_cmd(0xA8);  // Set Multiplex Ratio
    write_cmd(0x1F);  // ✨ 關鍵: 128x32 要設 0x1F (31)

    write_cmd(0xD3);  // Set Display Offset
    write_cmd(0x00);  // 0 offset

    write_cmd(0x40);  // Set Start Line (0x40 | 0)

    write_cmd(0x8D);  // Charge Pump
    write_cmd(0x14);  // Enable (0x14)

    write_cmd(0x20);  // Memory Addressing Mode
    write_cmd(0x00);  // Horizontal Addressing Mode (自動換行)

    write_cmd(0xA1);  // Segment Re-map (A0=正常, A1=左右反轉)
    write_cmd(0xC8);  // COM Output Scan Direction (C0=正常, C8=上下反轉)

    write_cmd(0xDA);  // Set COM Pins Hardware Config
    write_cmd(0x02);  // ✨ 關鍵: 128x32 是 0x02

    write_cmd(0x81);  // Set Contrast
    write_cmd(0x8F);  // 對比度 (00-FF)

    write_cmd(0xD9);  // Set Pre-charge Period
    write_cmd(0xF1);

    write_cmd(0xDB);  // Set VCOMH Deselect Level
    write_cmd(0x40);

    write_cmd(0xA4);  // Entire Display ON (Resume)
    write_cmd(0xA6);  // Normal Display (A7=Invert)

    write_cmd(0xAF);  // Display ON

    // 清除畫面
    memset(buffer, 0, sizeof(buffer));
    ssd1306_show();
}

void ssd1306_show(void)
{
    // 為了傳輸效率，我們手動構建 payload
    // 格式: [0x40 (Data Byte), byte1, byte2, ... byte512]
    uint8_t payload[1 + sizeof(buffer)];
    payload[0] = 0x40;  // Co=0, D/C#=1 (Data)
    memcpy(&payload[1], buffer, sizeof(buffer));

    // ✨ 替換點 2：使用具備 Timeout 與 Recovery 的安全函式
    // 這裡送出 513 bytes，如果中間 I2C 被短路，這裡會被安全攔截並恢復！
    hal_i2c_write_safe(SSD1306_ADDR, payload, sizeof(payload));
}

void ssd1306_clear(void)
{
    memset(buffer, 0, sizeof(buffer));
}

void ssd1306_fill(uint8_t pattern)
{
    memset(buffer, pattern, sizeof(buffer));
}

void ssd1306_draw_pixel(int x, int y, bool on)
{
    if (x < 0 || x >= SSD1306_WIDTH || y < 0 || y >= SSD1306_HEIGHT) return;

    // SSD1306 的記憶體是 Page base，每 8 個垂直 pixel 是一個 byte
    int byte_idx = x + (y / 8) * SSD1306_WIDTH;
    uint8_t bit_mask = 1 << (y % 8);

    if (on)
        buffer[byte_idx] |= bit_mask;
    else
        buffer[byte_idx] &= ~bit_mask;
}