/**
 * @file ssd1306_basic.h
 * @brief SSD1306 OLED Driver Interface (Hardware Agnostic)
 */

#ifndef SSD1306_BASIC_H
#define SSD1306_BASIC_H

#include <stdbool.h>
#include <stdint.h>

// 0.91" OLED 是 128x32
#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 32
#define SSD1306_ADDR 0x3C

// ✨ 完美解耦：不傳入 i2c_inst_t，底層硬體細節交由 HAL 層處理
void ssd1306_init(void);

void ssd1306_clear(void);
void ssd1306_fill(uint8_t pattern);
void ssd1306_draw_pixel(int x, int y, bool on);
void ssd1306_show(void);  // 將 buffer 送出到螢幕

#endif