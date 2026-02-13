#include "sentinel_core.h"

#include <string.h>  // ✨ 新增：為了支援 strcmp

// ==========================================
// 模組 A：系統電壓監控 (原本的設定)
// ==========================================
SentinelStatus Sentinel_CheckVoltage(float voltage)
{
    // 簡單的邊界判斷
    if (voltage < 3.0f)
    {
        return STATUS_LOW_BATTERY;
    }

    return STATUS_OK;
}

void sentinel_init(void)
{
    int x = 0;
    if (x)
    {
    }
}

// ==========================================
// 模組 B：指令解析狀態機 (新增)
// ==========================================
// 將 Buffer 定義為 static，確保狀態在多次呼叫間得以保留，且不被外部檔案直接存取
static char cmd_buffer[16];
static uint8_t cmd_idx = 0;

SystemCmd_t Sentinel_ParseChar(char c)
{
    // 遇到換行符號 (\n 或 \r)，代表指令輸入完畢，開始解析
    if (c == '\n' || c == '\r')
    {
        cmd_buffer[cmd_idx] = '\0';  // 補上 C 語言字串結尾
        SystemCmd_t result = CMD_NONE;

        // 核心邏輯：字串比對
        if (strcmp(cmd_buffer, "INV") == 0)
        {
            result = CMD_OLED_INVERT;
        }
        else if (strcmp(cmd_buffer, "NORM") == 0)
        {
            result = CMD_OLED_NORMAL;
        }
        else if (strcmp(cmd_buffer, "PING") == 0)
        {
            result = CMD_SYSTEM_PING;
        }

        // 解析完畢後重置 index，準備接收下一道指令
        cmd_idx = 0;
        return result;
    }

    // 如果還不是換行符號，就把字元存起來 (並防止 Buffer Overflow)
    if (cmd_idx < sizeof(cmd_buffer) - 1)
    {
        cmd_buffer[cmd_idx++] = c;
    }

    // 指令還沒湊齊，回傳 CMD_NONE 告訴主程式繼續等
    return CMD_NONE;
}