#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Ring Buffer Structure
 * @note  Size must be a power of 2 for efficient masking.
 */
typedef struct
{
    uint8_t* buffer;         // 資料存儲區
    uint32_t mask;           // 用於快速計算索引 (size - 1)
    volatile uint32_t head;  // 寫入位置 (由 ISR 修改)
    volatile uint32_t tail;  // 讀取位置 (由 Application 修改)
} ring_buffer_t;

/**
 * @brief Initialize the ring buffer
 * @param rb Pointer to ring buffer struct
 * @param buffer Pointer to the actual data array
 * @param size Size of the buffer (MUST be power of 2)
 * @return true if successful, false if size is not power of 2
 */
bool rb_init(ring_buffer_t* rb, uint8_t* buffer, uint32_t size);

/**
 * @brief Push data into buffer (Producer/ISR safe)
 */
bool rb_push(ring_buffer_t* rb, uint8_t data);

/**
 * @brief Pop data from buffer (Consumer/App safe)
 */
bool rb_pop(ring_buffer_t* rb, uint8_t* data);

/**
 * @brief Check if buffer is empty
 */
bool rb_is_empty(ring_buffer_t* rb);

/**
 * @brief Check if buffer is full
 */
bool rb_is_full(ring_buffer_t* rb);

#endif  // RING_BUFFER_H