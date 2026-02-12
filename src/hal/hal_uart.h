#ifndef HAL_UART_H
#define HAL_UART_H

#include <stdbool.h>
#include <stdint.h>

// 1. 新增 DMA 事件
typedef enum
{
    UART_EVENT_RX_COMPLETE,      // 單字節接收 (Byte IRQ)
    UART_EVENT_RX_DMA_COMPLETE,  // DMA 區塊接收完成 (Block IRQ) <-- NEW
    UART_EVENT_TX_COMPLETE,
    UART_EVENT_ERROR
} uart_event_t;

typedef void (*uart_callback_t)(void* ctx, uart_event_t event, void* data);

typedef struct
{
    uint8_t id;
    uart_callback_t callback;
    void* user_ctx;

    // --- DMA 模擬參數 (Internal State) ---
    uint8_t* dma_rx_buffer;  // 目標記憶體地址
    uint16_t dma_rx_len;     // 預期接收長度
    bool dma_active;         // DMA 是否開啟中
} uart_handle_t;

void HAL_UART_Init(uart_handle_t* h, uint8_t id);
void HAL_UART_RegisterCallback(uart_handle_t* h, uart_callback_t cb, void* ctx);

// --- DMA API ---
// 告訴硬體：「請把接下來收到的資料，自動搬到 pData，搬完 Size 個才叫我」
void HAL_UART_Receive_DMA(uart_handle_t* h, uint8_t* pData, uint16_t Size);

// --- 模擬器 ---
void HAL_UART_SimulateISR(uart_handle_t* h, uint8_t rx_data);
// 模擬 DMA 搬運完成
void HAL_UART_SimulateDMA_Complete(uart_handle_t* h, const uint8_t* mock_data, uint16_t len);

void HAL_UART_Send(uart_handle_t* h, const uint8_t* data, uint16_t len);
#endif  // HAL_UART_H