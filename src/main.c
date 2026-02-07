// 檔案: src/main.c
#include <stdio.h>

#include "app/sentinel_core.h"  // ✨ 修正引用，指向新的核心
#include "pico/stdlib.h"

// 如果有 LED 驅動，也可以在這裡 include
// #include "hal/hal_led.h"

int main() {
    // 1. 硬體初始化
    stdio_init_all();

    // 初始化板載 LED (Pico 2 W 通常是 WL_GPIO 0，但這裡我們先用標準 API)
    // 注意: Pico W/2W 的 LED 需要透過 cyw43 控制，為簡化 Day 4，我們先用 print 代替
    // 如果你有外接 LED 在 GPIO 25，可以 uncomment 下面這行
    // gpio_init(25); gpio_set_dir(25, GPIO_OUT);

    printf("🚀 Project Sentinel V6.0 Started!\n");

    // 模擬感測器數據
    float dummy_voltage = 3.3f;

    while (true) {
        // 2. 執行核心邏輯 (這就是我們今天 TDD 測試過的那個函數！)
        SentinelStatus status = Sentinel_CheckVoltage(dummy_voltage);

        // 3. 根據邏輯結果執行硬體動作
        switch (status) {
            case STATUS_OK:
                printf("Voltage %.2fV [OK]\n", dummy_voltage);
                // gpio_put(25, 1); // 亮燈代表正常
                break;
            case STATUS_LOW_BATTERY:
                printf("Voltage %.2fV [LOW BATTERY WARNING!]\n", dummy_voltage);
                // gpio_put(25, 0); // 滅燈代表異常
                break;
            default:
                printf("System Error\n");
                break;
        }

        // 模擬電壓下降 (為了看效果)
        dummy_voltage -= 0.05f;
        if (dummy_voltage < 2.8f) {
            dummy_voltage = 3.3f;  // 重置
        }

        sleep_ms(1000);
    }

    return 0;
}