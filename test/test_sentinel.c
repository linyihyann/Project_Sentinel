// 檔案位置: test/test_sentinel.c

#include "sentinel_core.h"  // 我們要測的目標
#include "unity.h"

void setUp(void) {
    // 每個測試執行前會跑這裡
}

void tearDown(void) {
    // 每個測試執行後會跑這裡
}

// 測試 1: 正常電壓
void test_System_Should_Be_Normal_At_3v3(void) {
    SentinelStatus status = Sentinel_CheckVoltage(3.3f);
    TEST_ASSERT_EQUAL(STATUS_OK, status);
}

// 測試 2: 低電壓邊界 (3.0V 應該還是 OK)
void test_System_Should_Be_Normal_At_3v0(void) {
    SentinelStatus status = Sentinel_CheckVoltage(3.0f);
    TEST_ASSERT_EQUAL(STATUS_OK, status);
}

// 測試 3: 低電壓觸發 (2.9V)
void test_System_Should_Alarm_Below_3v0(void) {
    SentinelStatus status = Sentinel_CheckVoltage(2.99f);
    TEST_ASSERT_EQUAL(STATUS_LOW_BATTERY, status);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_System_Should_Be_Normal_At_3v3);
    RUN_TEST(test_System_Should_Be_Normal_At_3v0);
    RUN_TEST(test_System_Should_Alarm_Below_3v0);
    return UNITY_END();
}