/**
 * @file hal_i2c.h
 * @brief Hardware Abstraction Layer for I2C with Bus Recovery
 * @author Your Name
 * @date 2026-02-13
 */

#ifndef HAL_I2C_H
#define HAL_I2C_H

#include <stdbool.h>
#include <stddef.h>  // for size_t
#include <stdint.h>

// --- 硬體設定 (Board Configuration) ---
// 你可以在這裡修改腳位，不用去改 .c 檔
#define HAL_I2C_PORT i2c0
#define HAL_I2C_SDA_PIN 4
#define HAL_I2C_SCL_PIN 5
#define HAL_I2C_BAUDRATE (400 * 1000)  // 400 kHz

// --- 錯誤碼定義 (Error Codes) ---
#define HAL_I2C_OK 0
#define HAL_I2C_ERR -1
#define HAL_I2C_TIMEOUT -2

/**
 * @brief 初始化 I2C 硬體與 GPIO
 * 同時啟用內部上拉電阻
 */
void hal_i2c_init(void);

/**
 * @brief 安全寫入 I2C (具備 Timeout 機制)
 * * @param addr 7-bit I2C Slave Address
 * @param src  要發送的資料指標
 * @param len  資料長度
 * @return int 寫入的 byte 數，或負值表示錯誤 (HAL_I2C_TIMEOUT)
 */
int hal_i2c_write_safe(uint8_t addr, const uint8_t* src, size_t len);

/**
 * @brief 執行 I2C 匯流排救援程序 (Bus Recovery)
 * 當偵測到 Bus Hang (SDA Low) 時呼叫此函式。
 * 會發送 9 個 Clock 來解鎖 Slave。
 */
void hal_i2c_recover(void);

#endif  // HAL_I2C_H