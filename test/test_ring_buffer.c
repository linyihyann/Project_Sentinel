#include <string.h>

#include "ring_buffer.h"  // 引用我們的待測目標
#include "unity.h"

// 定義一個測試用的 Buffer 空間
#define TEST_BUF_SIZE 8  // 小一點比較好測 Wrap around
uint8_t raw_buffer[TEST_BUF_SIZE];
ring_buffer_t rb;

// 每個測試跑之前會執行 (類似建構子)
void setUp(void)
{
    memset(raw_buffer, 0, TEST_BUF_SIZE);
    rb_init(&rb, raw_buffer, TEST_BUF_SIZE);
}

// 每個測試跑完後會執行 (類似解構子)
void tearDown(void) {}

// --- 測試案例 1: 初始化 ---
void test_RingBuffer_Init_Should_ResetIndices(void)
{
    TEST_ASSERT_EQUAL_UINT32(0, rb.head);
    TEST_ASSERT_EQUAL_UINT32(0, rb.tail);
    TEST_ASSERT_EQUAL_UINT32(TEST_BUF_SIZE - 1, rb.mask);
    TEST_ASSERT_TRUE(rb_is_empty(&rb));
}

// --- 測試案例 2: 正常寫入與讀取 ---
void test_RingBuffer_PushPop_Should_WorkNormally(void)
{
    TEST_ASSERT_TRUE(rb_push(&rb, 0xAA));
    TEST_ASSERT_TRUE(rb_push(&rb, 0xBB));

    uint8_t data;
    TEST_ASSERT_TRUE(rb_pop(&rb, &data));
    TEST_ASSERT_EQUAL_HEX8(0xAA, data);

    TEST_ASSERT_TRUE(rb_pop(&rb, &data));
    TEST_ASSERT_EQUAL_HEX8(0xBB, data);

    TEST_ASSERT_TRUE(rb_is_empty(&rb));
}

// --- 測試案例 3: 緩衝區滿 (Full Strategy) ---
void test_RingBuffer_Full_Should_RejectNewData(void)
{
    // 填滿 Buffer (注意：Ring Buffer 最多只能存 Size-1 個資料)
    for (int i = 0; i < TEST_BUF_SIZE - 1; i++)
    {
        TEST_ASSERT_TRUE(rb_push(&rb, (uint8_t)i));
    }

    TEST_ASSERT_TRUE(rb_is_full(&rb));

    // 嘗試再寫入一筆 -> 應該失敗
    TEST_ASSERT_FALSE(rb_push(&rb, 0xFF));

    // 讀出一筆後 -> 應該又要可以寫入
    uint8_t data;
    rb_pop(&rb, &data);
    TEST_ASSERT_TRUE(rb_push(&rb, 0xEE));
}

// --- 測試案例 4: 回繞機制 (Wrap Around) ---
// 這是最容易寫錯的地方，也是面試官最愛問的
void test_RingBuffer_WrapAround_Should_Work(void)
{
    // 1. 先填滿
    for (int i = 0; i < TEST_BUF_SIZE - 1; i++)
    {
        rb_push(&rb, i);
    }

    // 2. 全部讀出來 (讓 head 和 tail 都移到後面)
    uint8_t data;
    for (int i = 0; i < TEST_BUF_SIZE - 1; i++)
    {
        rb_pop(&rb, &data);
    }

    // 此時 buffer 是空的，但 head/tail 指標應該在後面 (例如 7)
    TEST_ASSERT_TRUE(rb_is_empty(&rb));

    // 3. 再寫入一筆 -> 這時候 head 應該要 wrap 回 0
    TEST_ASSERT_TRUE(rb_push(&rb, 0x99));

    // 驗證內部狀態 (White-box testing)
    // 假設 Size=8, Mask=7.
    // 若之前寫了 7 筆並讀了 7 筆，Head=7, Tail=7.
    // 再寫 1 筆 -> Head 應該變成 (7+1) & 7 = 0
    TEST_ASSERT_EQUAL_UINT32(0, rb.head);

    // 讀取驗證
    rb_pop(&rb, &data);
    TEST_ASSERT_EQUAL_HEX8(0x99, data);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_RingBuffer_Init_Should_ResetIndices);
    RUN_TEST(test_RingBuffer_PushPop_Should_WorkNormally);
    RUN_TEST(test_RingBuffer_Full_Should_RejectNewData);
    RUN_TEST(test_RingBuffer_WrapAround_Should_Work);
    return UNITY_END();
}