#ifndef SENTINEL_CORE_H
#define SENTINEL_CORE_H

#include <stdbool.h>
#include <stdint.h>

// ==========================================
// 模組 A：系統電壓監控 (Day 4/5 既有)
// ==========================================
typedef enum
{
    STATUS_OK,
    STATUS_LOW_BATTERY,
    STATUS_ERROR
} SentinelStatus;

SentinelStatus Sentinel_CheckVoltage(float voltage);
void sentinel_init(void);

// ==========================================
// 模組 B：系統指令解析器 (Day 6~9 整合新增)
// ==========================================
typedef enum
{
    CMD_NONE = 0,
    CMD_OLED_NORMAL,
    CMD_OLED_INVERT,
    CMD_SYSTEM_PING
} SystemCmd_t;

/**
 * @brief 狀態機：逐字元解析系統指令
 * @param c 傳入單一字元
 * @return SystemCmd_t 解析完成的指令 (若尚未湊齊換行符號則回傳 CMD_NONE)
 */
SystemCmd_t Sentinel_ParseChar(char c);

#endif  // SENTINEL_CORE_H