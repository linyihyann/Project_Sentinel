#ifndef HAL_UART_H
#define HAL_UART_H

#include <stdbool.h>
#include <stdint.h>

// 定義 UART 事件類型
typedef enum
{
    UART_EVENT_RX_COMPLETE,
    UART_EVENT_TX_COMPLETE,
    UART_EVENT_ERROR
} uart_event_t;

// 定義 Callback 函式原型
// ctx: 用戶傳入的上下文 (User Context / Object Pointer)
// event: 發生什麼事
// data: 接收到的數據 (如果是 RX 事件)
typedef void (*uart_callback_t)(void* ctx, uart_event_t event, uint8_t data);

// UART 驅動的 Handle 結構 (模擬 OOP 物件)
typedef struct
{
    uint8_t id;                // UART ID (e.g., 0 or 1)
    uart_callback_t callback;  // 上層註冊的函式
    void* user_ctx;            // 上層傳入的 Context
} uart_handle_t;

// API
void HAL_UART_Init(uart_handle_t* h, uint8_t id);
void HAL_UART_RegisterCallback(uart_handle_t* h, uart_callback_t cb, void* ctx);

// 模擬 ISR 觸發 (用於 Host 端單元測試)
void HAL_UART_SimulateISR(uart_handle_t* h, uint8_t rx_data);

#endif  // HAL_UART_H