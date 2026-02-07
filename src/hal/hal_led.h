/**
 * @file hal_led.h
 * @brief HAL LED driver — hardware abstraction for Pico 2 W onboard LED (CYW43).
 *
 * This layer owns all pico/cyw43_arch interactions for the LED.
 */

#ifndef HAL_LED_H
#define HAL_LED_H

#include <stdbool.h>
#include <stdint.h>

/** Initialize CYW43 and LED. Call once before hal_led_set. */
void hal_led_init(void);

/** Set LED on (true) or off (false). */
void hal_led_set(bool on);

/** Block for approximately ms milliseconds (HAL-provided for injection). */
void hal_sleep_ms(uint32_t ms);

#endif /* HAL_LED_H */
