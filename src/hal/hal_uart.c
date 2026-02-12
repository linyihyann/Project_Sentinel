#include "hal_uart.h"

#include <stddef.h>  // for NULL

// 初始化 UART Handle
void HAL_UART_Init(uart_handle_t* h, uint8_t id)
{
    if (h != NULL)
    {
        h->id = id;
        h->callback = NULL;
        h->user_ctx = NULL;
    }
}

// 註冊 Callback (Observer Pattern 核心)
void HAL_UART_RegisterCallback(uart_handle_t* h, uart_callback_t cb, void* ctx)
{
    if (h != NULL)
    {
        // Critical Section Start (在真實 MCU 需關中斷)
        h->callback = cb;
        h->user_ctx = ctx;
        // Critical Section End
    }
}

// 模擬真實硬體的中斷處理函式 (ISR)
// 這就是 linker 找不到的那個函式
void HAL_UART_SimulateISR(uart_handle_t* h, uint8_t rx_data)
{
    if (h == NULL) return;

    // Decoupling: 驅動層只負責 "Notify"，不負責 "Logic"
    if (h->callback != NULL)
    {
        h->callback(h->user_ctx, UART_EVENT_RX_COMPLETE, rx_data);
    }
}