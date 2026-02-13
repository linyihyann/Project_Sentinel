/**
 * @file hal_i2c.c
 * @brief I2C Hardware Abstraction Layer Implementation
 */

#include "hal_i2c.h"

#include <stdio.h>

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"

// ==========================================
// I2C 硬體初始化
// ==========================================
void hal_i2c_init(void)
{
    // 1. 初始化 I2C 硬體與時脈 (400kHz)
    i2c_init(HAL_I2C_PORT, HAL_I2C_BAUDRATE);

    // 2. 設定腳位功能為 I2C
    gpio_set_function(HAL_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(HAL_I2C_SCL_PIN, GPIO_FUNC_I2C);

    // 3. 啟用內部上拉電阻 (I2C Open-Drain 必備)
    gpio_pull_up(HAL_I2C_SDA_PIN);
    gpio_pull_up(HAL_I2C_SCL_PIN);

    printf("[HAL] I2C Initialized on SDA:%d, SCL:%d at %d Hz\n", HAL_I2C_SDA_PIN, HAL_I2C_SCL_PIN,
           HAL_I2C_BAUDRATE);
}

// ==========================================
// [Day 9] I2C Recovery Logic
// ==========================================
void hal_i2c_recover(void)
{
    printf("[HAL] ⚠️ I2C Bus Hang detected! Starting recovery...\n");

    gpio_init(HAL_I2C_SDA_PIN);
    gpio_init(HAL_I2C_SCL_PIN);
    gpio_set_dir(HAL_I2C_SDA_PIN, GPIO_IN);
    gpio_set_dir(HAL_I2C_SCL_PIN, GPIO_OUT);

    for (int i = 0; i < 9; i++)
    {
        if (gpio_get(HAL_I2C_SDA_PIN))
        {
            printf("[HAL] ✅ SDA released at clock %d\n", i);
            break;
        }
        gpio_put(HAL_I2C_SCL_PIN, 0);
        sleep_us(10);
        gpio_put(HAL_I2C_SCL_PIN, 1);
        sleep_us(10);
    }

    gpio_set_dir(HAL_I2C_SDA_PIN, GPIO_OUT);
    gpio_put(HAL_I2C_SDA_PIN, 0);
    sleep_us(10);
    gpio_put(HAL_I2C_SCL_PIN, 1);
    sleep_us(10);
    gpio_put(HAL_I2C_SDA_PIN, 1);
    sleep_us(10);

    gpio_set_function(HAL_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(HAL_I2C_SCL_PIN, GPIO_FUNC_I2C);
    i2c_init(HAL_I2C_PORT, HAL_I2C_BAUDRATE);
    gpio_pull_up(HAL_I2C_SDA_PIN);
    gpio_pull_up(HAL_I2C_SCL_PIN);

    printf("[HAL] 🔄 Bus Recovery Complete.\n");
}

int hal_i2c_write_safe(uint8_t addr, const uint8_t* src, size_t len)
{
    // 50ms Timeout 機制
    int ret =
        i2c_write_blocking_until(HAL_I2C_PORT, addr, src, len, false, make_timeout_time_ms(50));

    if (ret == PICO_ERROR_TIMEOUT || ret == PICO_ERROR_GENERIC)
    {
        printf("[HAL] ❌ I2C Write Timeout! Error: %d\n", ret);
        hal_i2c_recover();
        return HAL_I2C_TIMEOUT;
    }
    return ret;  // 回傳成功寫入的 byte 數
}