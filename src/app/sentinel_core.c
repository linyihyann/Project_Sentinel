#include "sentinel_core.h"

SentinelStatus Sentinel_CheckVoltage(float voltage) {
    // 簡單的邊界判斷
    if (voltage < 3.0f) {
        return STATUS_LOW_BATTERY;
    }

    // 未來可以在這裡加入遲滯 (Hysteresis) 邏輯

    return STATUS_OK;
}