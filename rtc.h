
#ifndef ALTROM_RTC_H
#define ALTROM_RTC_H

#include <stdint.h>
#include <stdbool.h>

#define RTC_INITIAL_YEAR 2000

typedef struct {
    uint8_t Sec;
    uint8_t Min;
    uint8_t Hour;
} RTC_TIME;

typedef struct {
    uint8_t Day;
    uint8_t DoW;
    uint8_t Month;
    uint16_t Year;
} RTC_DATE;

typedef struct {
    RTC_TIME Time;
    RTC_DATE Date;
} RTC_DATETIME;

bool RTC_Initialize(void);

bool RTC_IsValidState(void);

RTC_DATETIME RTC_GetDateTime(void);
bool RTC_SetDateTime(const RTC_DATETIME *datetime);

RTC_DATETIME RTC_GetAlarmDateTime(void);
bool RTC_SetAlarmDateTime(const RTC_DATETIME *datetime,
                          bool use_interrupt);

bool RTC_IsAlarmActive(void);

bool RTC_LockBBPower(void);
bool RTC_UnlockBBPower(void);

bool RTC_EnablePowerUp(void);
bool RTC_DisablePowerUp(void);

void RTC_SetCIIEnable(uint16_t intervals, bool enable);

void RTC_Sleep(void);
void RTC_Resume(void);

#endif
