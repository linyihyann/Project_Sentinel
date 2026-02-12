#include <stdbool.h>
#include <string.h>  // for memset

#include "hal_uart.h"
#include "unity.h"

// ==========================================
// 1. 定義應用層物件 (Application Contexts)
// ==========================================

// 模擬 GPS 模組的資料結構
typedef struct
{
    char buffer[32];
    uint8_t index;
    bool fix_valid;
} GPS_Data_t;

// 模擬 WiFi 模組的資料結構
typedef struct
{
    char response[32];
    uint8_t len;
    bool connected;
} WiFi_Data_t;

// 全域實例 (Test Fixtures)
static GPS_Data_t my_gps;
static WiFi_Data_t my_wifi;

static uart_handle_t h_uart_gps;   // UART 0
static uart_handle_t h_uart_wifi;  // UART 1

// ==========================================
// 2. 實作 Callback (Observer Implementation)
// ==========================================

// GPS 的回調函式
void GPS_OnRxCallback(void* ctx, uart_event_t event, uint8_t data)
{
    // [關鍵] 將 void* 還原為 GPS 專用的結構指標
    GPS_Data_t* gps = (GPS_Data_t*)ctx;

    if (event == UART_EVENT_RX_COMPLETE)
    {
        if (gps->index < 32)
        {
            gps->buffer[gps->index++] = (char)data;
        }
        // 模擬：收到 '$' 代表 GPS 鎖定
        if (data == '$')
        {
            gps->fix_valid = true;
        }
    }
}

// WiFi 的回調函式
void WiFi_OnRxCallback(void* ctx, uart_event_t event, uint8_t data)
{
    // [關鍵] 將 void* 還原為 WiFi 專用的結構指標
    WiFi_Data_t* wifi = (WiFi_Data_t*)ctx;

    if (event == UART_EVENT_RX_COMPLETE)
    {
        if (wifi->len < 32)
        {
            wifi->response[wifi->len++] = (char)data;
        }
        // 模擬：收到 'K' 代表 OK
        if (data == 'K')
        {
            wifi->connected = true;
        }
    }
}

// ==========================================
// 3. 測試環境設定 (Setup & Teardown)
// ==========================================
void setUp(void)
{
    // 清空資料
    memset(&my_gps, 0, sizeof(GPS_Data_t));
    memset(&my_wifi, 0, sizeof(WiFi_Data_t));

    // 初始化兩個獨立的 UART Handle
    HAL_UART_Init(&h_uart_gps, 0);   // ID 0
    HAL_UART_Init(&h_uart_wifi, 1);  // ID 1
}

void tearDown(void) {}

// ==========================================
// 4. 測試案例 (Test Cases)
// ==========================================

// 測試案例 A: 驗證單一回調是否正常觸發
void test_single_callback_trigger(void)
{
    // 註冊 GPS
    HAL_UART_RegisterCallback(&h_uart_gps, GPS_OnRxCallback, &my_gps);

    // 模擬收到 'A'
    HAL_UART_SimulateISR(&h_uart_gps, 'A');

    // 驗證 GPS 有收到
    TEST_ASSERT_EQUAL_CHAR('A', my_gps.buffer[0]);
    TEST_ASSERT_EQUAL_UINT8(1, my_gps.index);
}

// 測試案例 B: [重點] 驗證雙實例隔離 (Dual Instance Isolation)
// 證明 GPS 的資料絕對不會跑到 WiFi 去，反之亦然
void test_dual_instance_isolation(void)
{
    // 1. 分別註冊
    HAL_UART_RegisterCallback(&h_uart_gps, GPS_OnRxCallback, &my_gps);
    HAL_UART_RegisterCallback(&h_uart_wifi, WiFi_OnRxCallback, &my_wifi);

    // 2. 對 GPS 通道輸入 '$'
    HAL_UART_SimulateISR(&h_uart_gps, '$');

    // 驗證 GPS 狀態變了
    TEST_ASSERT_TRUE(my_gps.fix_valid);
    TEST_ASSERT_EQUAL_CHAR('$', my_gps.buffer[0]);

    // [關鍵驗證] 確認 WiFi 完全沒受到影響 (應該還是空的)
    TEST_ASSERT_FALSE(my_wifi.connected);
    TEST_ASSERT_EQUAL_UINT8(0, my_wifi.len);

    // 3. 對 WiFi 通道輸入 'O', 'K'
    HAL_UART_SimulateISR(&h_uart_wifi, 'O');
    HAL_UART_SimulateISR(&h_uart_wifi, 'K');

    // 驗證 WiFi 狀態變了
    TEST_ASSERT_TRUE(my_wifi.connected);
    TEST_ASSERT_EQUAL_CHAR('O', my_wifi.response[0]);
    TEST_ASSERT_EQUAL_CHAR('K', my_wifi.response[1]);

    // [關鍵驗證] 確認 GPS 資料沒有被覆蓋 (應該還是只有 '$')
    TEST_ASSERT_EQUAL_UINT8(1, my_gps.index);
    TEST_ASSERT_EQUAL_CHAR('$', my_gps.buffer[0]);
}

// 測試案例 C: 驗證 NULL 安全性
void test_null_safety(void)
{
    // 不註冊任何 Callback，直接觸發中斷
    HAL_UART_SimulateISR(&h_uart_gps, 0xFF);

    // 程式不應該 Crash，且變數不應改變
    TEST_ASSERT_EQUAL_UINT8(0, my_gps.index);
}

// ==========================================
// 5. 測試執行入口 (Main Runner)
// ==========================================
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_single_callback_trigger);
    RUN_TEST(test_dual_instance_isolation);  // 這就是我們要的重點測試！
    RUN_TEST(test_null_safety);
    return UNITY_END();
}