#ifndef SENTINEL_CORE_H
#define SENTINEL_CORE_H

// 定義狀態碼，方便測試與除錯
typedef enum { STATUS_OK, STATUS_LOW_BATTERY, STATUS_ERROR } SentinelStatus;

/**
 * @brief 檢查電池電壓狀態
 * @param voltage 輸入電壓 (伏特)
 * @return SentinelStatus 系統狀態
 */
SentinelStatus Sentinel_CheckVoltage(float voltage);

#endif