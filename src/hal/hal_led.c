/**
 * @file hal_led.c
 * @brief HAL LED driver — pico/cyw43_arch implementation for Pico 2 W.
 */

#include "hal_led.h"

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

void hal_led_init(void) {
    if (cyw43_arch_init()) {
        return; /* caller may check; minimal init for demo */
    }
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
}

void hal_led_set(bool on) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on ? 1 : 0);
}

void hal_sleep_ms(uint32_t ms) {
    sleep_ms(ms);
}
