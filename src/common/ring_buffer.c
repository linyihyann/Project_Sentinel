#include "ring_buffer.h"

// 檢查是否為 2 的冪次方 (Power of 2 check)
static bool is_power_of_two(uint32_t n)
{
    return (n > 0) && ((n & (n - 1)) == 0);
}

bool rb_init(ring_buffer_t* rb, uint8_t* buffer, uint32_t size)
{
    if (!rb || !buffer || !is_power_of_two(size))
    {
        return false;  // Fail if pointers are null or size is not power of 2
    }

    rb->buffer = buffer;
    rb->mask = size - 1;  // Example: Size 16 (10000), Mask 15 (01111)
    rb->head = 0;
    rb->tail = 0;

    return true;
}

bool rb_is_full(ring_buffer_t* rb)
{
    // Check if next head position equals tail
    // Masking handles the wrap-around automatically
    uint32_t next_head = (rb->head + 1) & rb->mask;
    return (next_head == rb->tail);
}

bool rb_is_empty(ring_buffer_t* rb)
{
    return (rb->head == rb->tail);
}

bool rb_push(ring_buffer_t* rb, uint8_t data)
{
    uint32_t next_head = (rb->head + 1) & rb->mask;

    // Critical Section: In SPSC, only Producer checks Full and modifies Head
    // No lock needed if only one writer exists.
    if (next_head == rb->tail)
    {
        return false;  // Buffer Full
    }

    rb->buffer[rb->head] = data;

    // Memory Barrier ensure data is written before head is updated
    // __sync_synchronize(); // Uncomment for strict OOO (Out of Order) architectures

    rb->head = next_head;
    return true;
}

bool rb_pop(ring_buffer_t* rb, uint8_t* data)
{
    // Critical Section: In SPSC, only Consumer checks Empty and modifies Tail
    if (rb->head == rb->tail)
    {
        return false;  // Buffer Empty
    }

    *data = rb->buffer[rb->tail];

    // Memory Barrier ensure data is read before tail is updated
    // __sync_synchronize();

    rb->tail = (rb->tail + 1) & rb->mask;
    return true;
}