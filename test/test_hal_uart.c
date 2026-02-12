#include "hal_uart.h"
#include "unity.h"

uart_handle_t my_uart;
bool callback_triggered = false;
uint8_t received_byte = 0;
int my_app_state = 0;  // 模擬應用層的狀態

// 這是我們的 "Observer" (應用層邏輯)
void My_App_Callback(void* ctx, uart_event_t event, uint8_t data)
{
    int* state_ptr = (int*)ctx;

    if (event == UART_EVENT_RX_COMPLETE)
    {
        callback_triggered = true;
        received_byte = data;
        (*state_ptr)++;  // 修改外部狀態，證明 Context 傳遞成功
    }
}

void setUp(void)
{
    HAL_UART_Init(&my_uart, 0);
    callback_triggered = false;
    my_app_state = 100;
}

void tearDown(void) {}

void test_callback_should_trigger_when_isr_fires(void)
{
    // 1. 註冊 Callback，並傳入 my_app_state 的地址
    HAL_UART_RegisterCallback(&my_uart, My_App_Callback, &my_app_state);

    // 2. 模擬硬體收到 0xAB
    HAL_UART_SimulateISR(&my_uart, 0xAB);

    // 3. 驗證
    TEST_ASSERT_TRUE(callback_triggered);
    TEST_ASSERT_EQUAL_HEX8(0xAB, received_byte);
    TEST_ASSERT_EQUAL_INT(101, my_app_state);
}

void test_null_callback_safe(void)
{
    // 1. 註冊 NULL (防呆測試)
    HAL_UART_RegisterCallback(&my_uart, NULL, NULL);

    // 2. 觸發 ISR
    HAL_UART_SimulateISR(&my_uart, 0xFF);

    // 3. 驗證系統存活且無觸發
    TEST_ASSERT_FALSE(callback_triggered);
}

// =================================================================
// ⚠️ 關鍵修正：加入 Test Runner (Main Function)
// =================================================================
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_callback_should_trigger_when_isr_fires);
    RUN_TEST(test_null_callback_safe);
    return UNITY_END();
}