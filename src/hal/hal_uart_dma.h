/* src/hal/hal_uart_dma.h */
#ifndef HAL_UART_DMA_H
#define HAL_UART_DMA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// 定義緩衝區大小 (必須是 2 的次方，例如 256, 512，配合 DMA Ring 機制)
#define UART_DMA_BUFFER_SIZE 256

/**
 * @brief 初始化 UART 與 DMA 通道
 * @return true 初始化成功, false 資源不足
 */
bool hal_uart_dma_init(void);

/**
 * @brief 使用 DMA 發送數據 (非阻塞 / Non-blocking)
 * @details 這函式會立即返回，DMA 會在背景搬運數據
 * @param data 指向要發送的數據
 * @param len 數據長度
 */
void hal_uart_dma_send(const uint8_t* data, size_t len);

/**
 * @brief 檢查並讀取接收到的數據
 * @param buffer 用於存放讀取數據的緩衝區
 * @param max_len buffer 的最大容量
 * @return 實際讀取到的位元組數量
 */
size_t hal_uart_dma_read(uint8_t* buffer, size_t max_len);

#endif  // HAL_UART_DMA_H