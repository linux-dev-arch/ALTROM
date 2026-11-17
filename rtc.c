
#include "rtc.h"

/*
 * ALTROM native MT6261 RTC driver.
 *
 * Register offsets and programming sequence are based on
 * the MTK RTC implementation used by the DZ09 project.
 *
 * No DZ09 SDK dependencies.
 */

/* =========================================================
   HARDWARE CONFIGURATION
   ========================================================= */

#ifndef ALTROM_RTC_BASE
#define ALTROM_RTC_BASE 0xA0710000
#endif

#define RTC_REG(offset) \
    (*(volatile uint16_t *)(ALTROM_RTC_BASE + (offset)))

/* =========================================================
   RTC REGISTER MAP
   ========================================================= */

#define RTC_BBPU       RTC_REG(0x0000)
#define RTC_IRQ_STA    RTC_REG(0x0004)
#define RTC_IRQ_EN     RTC_REG(0x0008)
#define RTC_CII_EN     RTC_REG(0x000C)
#define RTC_AL_MASK    RTC_REG(0x0010)

#define RTC_TC_SEC     RTC_REG(0x0014)
#define RTC_TC_MIN     RTC_REG(0x0018)
#define RTC_TC_HOU     RTC_REG(0x001C)
#define RTC_TC_DOM     RTC_REG(0x0020)
#define RTC_TC_DOW     RTC_REG(0x0024)
#define RTC_TC_MTH     RTC_REG(0x0028)
#define RTC_TC_YEA     RTC_REG(0x002C)

#define RTC_AL_SEC     RTC_REG(0x0030)
#define RTC_AL_MIN     RTC_REG(0x0034)
#define RTC_AL_HOU     RTC_REG(0x0038)
#define RTC_AL_DOM     RTC_REG(0x003C)
#define RTC_AL_DOW     RTC_REG(0x0040)
#define RTC_AL_MTH     RTC_REG(0x0044)
#define RTC_AL_YEA     RTC_REG(0x0048)

#define RTC_OSC32CON   RTC_REG(0x004C)
#define RTC_POWERKEY1  RTC_REG(0x0050)
#define RTC_POWERKEY2  RTC_REG(0x0054)
#define RTC_PROT       RTC_REG(0x0068)
#define RTC_WRTGR      RTC_REG(0x0074)
#define RTC_CON        RTC_REG(0x0078)

/* =========================================================
   REGISTER BITS
   ========================================================= */

#define KEY_BBPU       0x4300

#define PWREN          (1U << 0)
#define BBPU           (1U << 2)
#define AUTO           (1U << 3)
#define CBUSY          (1U << 6)

#define ALSTA          (1U << 0)
#define TCSTA          (1U << 1)

#define AL_EN          (1U << 0)
#define TC_EN          (1U << 1)

#define SEC_MSK        (1U << 0)
#define MIN_MSK        (1U << 1)
#define HOU_MSK        (1U << 2)
#define DOM_MSK        (1U << 3)
#define DOW_MSK        (1U << 4)
#define MTH_MSK        (1U << 5)
#define YEA_MSK        (1U << 6)

#define SECCII         (1U << 0)
#define MINCII         (1U << 1)
#define HOUCII         (1U << 2)
#define DOMCII         (1U << 3)
#define DOWCII         (1U << 4)
#define MTHCII         (1U << 5)
#define YEACII         (1U << 6)
#define SECCII1_2      (1U << 7)
#define SECCII1_4      (1U << 8)
#define SECCII1_8      (1U << 9)

#define OSC32_RSV      0x0400
#define OSC32KEY1      0x1A57
#define OSC32KEY2      0x2B68

#define XOSCCALI(v)    (((v) & 0x1F) << 0)
#define EMB_MODE(v)    (((v) & 0x07) << 6)

#define POWERKEY1      0xA357
#define POWERKEY2      0x67D2

#define PROT1          0x586A
#define PROT2          0x9136

#define WRTGR          (1U << 0)

#define VBAT_LPSTA_RAW (1U << 0)
#define LPEN(v)        (((v) & 0x03) << 1)
#define LPRST          (1U << 3)
#define LPSTA_RAW      (1U << 15)

/* Maximum number of iterations when waiting for RTC. */
#define RTC_WAIT_LIMIT 1000000U

/* =========================================================
   INTERNAL HELPERS
   ========================================================= */

static bool RTC_WaitCBUSY(void)
{
    uint32_t timeout = RTC_WAIT_LIMIT;

    while (RTC_BBPU & CBUSY)
    {
        if (--timeout == 0)
            return false;
    }

    return true;
}

static bool RTC_UpdateContext(void)
{
    RTC_WRTGR = WRTGR;

    return RTC_WaitCBUSY();
}

static bool RTC_Unprotect(void)
{
    RTC_PROT = PROT1;

    if (!RTC_UpdateContext())
        return false;

    RTC_PROT = PROT2;

    return RTC_UpdateContext();
}

static bool RTC_SetPowerkey(void)
{
    RTC_POWERKEY1 = POWERKEY1;
    RTC_POWERKEY2 = POWERKEY2;

    return RTC_UpdateContext();
}

static bool RTC_SetOSC32CON(uint16_t data)
{
    RTC_OSC32CON = OSC32KEY1;

    if (!RTC_WaitCBUSY())
        return false;

    RTC_OSC32CON = OSC32KEY2;

    if (!RTC_WaitCBUSY())
        return false;

    RTC_OSC32CON = data;

    return RTC_WaitCBUSY();
}

/* =========================================================
   DATE HELPERS
   ========================================================= */

static const uint8_t RTC_DaysInMonth[12] =
{
    31, 28, 31, 30, 31, 30,
    31, 31, 30, 31, 30, 31
};

static bool RTC_IsLeapYear(uint16_t year)
{
    return ((year % 4U) == 0U &&
            ((year % 100U) != 0U || (year % 400U) == 0U));
}

static uint8_t RTC_GetDaysInMonth(uint16_t year, uint8_t month)
{
    uint8_t days;

    if (month < 1 || month > 12)
        return 0;

    days = RTC_DaysInMonth[month - 1];

    if (month == 2 && RTC_IsLeapYear(year))
        days++;

    return days;
}

static uint8_t RTC_DayOfWeek(const RTC_DATE *date)
{
    uint32_t year;
    uint32_t month;
    uint32_t day;
    uint32_t a;
    uint32_t dow;

    year = date->Year;
    month = date->Month;
    day = date->Day;

    a = (14U - month) / 12U;

    year -= a;
    month += 12U * a - 2U;

    dow = (day + year + year / 4U -
           year / 100U + year / 400U +
           (31U * month) / 12U) % 7U;

    return (uint8_t)(dow == 0 ? 7 : dow);
}

static bool RTC_ValidateDateTime(const RTC_DATETIME *dt)
{
    if (dt == NULL)
        return false;

    if (dt->Date.Year < RTC_INITIAL_YEAR ||
        dt->Date.Year > RTC_INITIAL_YEAR + 127)
        return false;

    if (dt->Date.Month < 1 || dt->Date.Month > 12)
        return false;

    if (dt->Date.Day < 1 ||
        dt->Date.Day > RTC_GetDaysInMonth(
            dt->Date.Year, dt->Date.Month))
        return false;

    if (dt->Time.Hour > 23 ||
        dt->Time.Min > 59 ||
        dt->Time.Sec > 59)
        return false;

    return true;
}

/* =========================================================
   RTC INITIALIZATION
   ========================================================= */

bool RTC_Initialize(void)
{
    RTC_DATETIME initial;

    /*
     * Configure 32 kHz oscillator.
     * Calibration value matches the original driver.
     */
    if (!RTC_SetOSC32CON(
            OSC32_RSV | EMB_MODE(0) | XOSCCALI(0x0F)))
        return false;

    if (!RTC_Unprotect())
        return false;

    if (!RTC_IsValidState())
    {
        if (!RTC_SetPowerkey())
            return false;

        RTC_BBPU = KEY_BBPU;

        if (!RTC_UpdateContext())
            return false;

        RTC_CON = LPEN(1) | LPSTA_RAW;

        if (!RTC_UpdateContext())
            return false;

        RTC_CON = LPEN(1) | LPRST;

        if (!RTC_UpdateContext())
            return false;

        RTC_CON = LPEN(1);

        if (!RTC_UpdateContext())
            return false;

        /* Disable periodic and normal interrupts. */
        RTC_CII_EN = 0;
        RTC_IRQ_EN = 0;

        RTC_AL_MASK = YEA_MSK | MTH_MSK | DOM_MSK |
                      DOW_MSK | HOU_MSK | MIN_MSK |
                      SEC_MSK;

        if (!RTC_UpdateContext())
            return false;

        /* Read interrupt status to clear pending flags. */
        (void)RTC_IRQ_STA;

        /* Set initial date to 1 January 2000. */
        initial.Time.Sec = 0;
        initial.Time.Min = 0;
        initial.Time.Hour = 0;

        initial.Date.Day = 1;
        initial.Date.Month = 1;
        initial.Date.Year = RTC_INITIAL_YEAR;
        initial.Date.DoW = RTC_DayOfWeek(&initial.Date);

        if (!RTC_SetDateTime(&initial))
            return false;

        if (!RTC_SetAlarmDateTime(&initial, false))
            return false;
    }

    return true;
}

/* =========================================================
   RTC STATE
   ========================================================= */

bool RTC_IsValidState(void)
{
    return ((RTC_CON & VBAT_LPSTA_RAW) ||
            (RTC_POWERKEY1 != POWERKEY1) ||
            (RTC_POWERKEY2 != POWERKEY2))
           ? false : true;
}

bool RTC_IsAlarmActive(void)
{
    uint16_t flags;

    if (!RTC_IsValidState())
        return false;

    flags = RTC_IRQ_EN & RTC_IRQ_STA;

    return ((RTC_BBPU & PWREN) && (flags & ALSTA))
           ? true : false;
}

/* =========================================================
   GET DATE AND TIME
   ========================================================= */

RTC_DATETIME RTC_GetDateTime(void)
{
    RTC_DATETIME dt;
    uint16_t sec_before;
    uint16_t sec_after;
    uint8_t attempts = 3;

    do
    {
        sec_before = RTC_TC_SEC;

        dt.Time.Min  = (uint8_t)RTC_TC_MIN;
        dt.Time.Hour = (uint8_t)RTC_TC_HOU;

        dt.Date.Day   = (uint8_t)RTC_TC_DOM;
        dt.Date.DoW   = (uint8_t)RTC_TC_DOW;
        dt.Date.Month = (uint8_t)RTC_TC_MTH;
        dt.Date.Year  = (uint16_t)(RTC_TC_YEA +
                                   RTC_INITIAL_YEAR);

        sec_after = RTC_TC_SEC;

        attempts--;

    } while (sec_before != sec_after && attempts != 0);

    dt.Time.Sec = (uint8_t)sec_after;

    return dt;
}

/* =========================================================
   SET DATE AND TIME
   ========================================================= */

bool RTC_SetDateTime(const RTC_DATETIME *dt)
{
    if (!RTC_ValidateDateTime(dt))
        return false;

    if (!RTC_Unprotect())
        return false;

    RTC_TC_SEC = dt->Time.Sec;
    RTC_TC_MIN = dt->Time.Min;
    RTC_TC_HOU = dt->Time.Hour;

    RTC_TC_YEA = dt->Date.Year - RTC_INITIAL_YEAR;
    RTC_TC_MTH = dt->Date.Month;
    RTC_TC_DOW = RTC_DayOfWeek(&dt->Date);
    RTC_TC_DOM = dt->Date.Day;

    return RTC_UpdateContext();
}

/* =========================================================
   GET ALARM DATE AND TIME
   ========================================================= */

RTC_DATETIME RTC_GetAlarmDateTime(void)
{
    RTC_DATETIME dt;

    dt.Time.Sec  = (uint8_t)RTC_AL_SEC;
    dt.Time.Min  = (uint8_t)RTC_AL_MIN;
    dt.Time.Hour = (uint8_t)RTC_AL_HOU;

    dt.Date.Day   = (uint8_t)RTC_AL_DOM;
    dt.Date.DoW   = (uint8_t)RTC_AL_DOW;
    dt.Date.Month = (uint8_t)RTC_AL_MTH;
    dt.Date.Year  = (uint16_t)(RTC_AL_YEA +
                               RTC_INITIAL_YEAR);

    return dt;
}

/* =========================================================
   SET ALARM DATE AND TIME
   ========================================================= */

bool RTC_SetAlarmDateTime(const RTC_DATETIME *dt,
                          bool use_interrupt)
{
    uint16_t irq_enable;

    if (!RTC_ValidateDateTime(dt))
        return false;

    if (!RTC_Unprotect())
        return false;

    RTC_AL_HOU = dt->Time.Hour;
    RTC_AL_MIN = dt->Time.Min;
    RTC_AL_SEC = dt->Time.Sec;

    RTC_AL_YEA = dt->Date.Year - RTC_INITIAL_YEAR;
    RTC_AL_MTH = dt->Date.Month;
    RTC_AL_DOW = RTC_DayOfWeek(&dt->Date);
    RTC_AL_DOM = dt->Date.Day;

    irq_enable = RTC_IRQ_EN;

    if (use_interrupt)
        irq_enable |= AL_EN;
    else
        irq_enable &= (uint16_t)~AL_EN;

    RTC_IRQ_EN = irq_enable;

    RTC_AL_MASK =
        (uint16_t)(~(YEA_MSK | MTH_MSK | DOM_MSK |
                     HOU_MSK | MIN_MSK | SEC_MSK) & 0x7F);

    return RTC_UpdateContext();
}

/* =========================================================
   BATTERY BACKUP POWER CONTROL
   ========================================================= */

bool RTC_LockBBPower(void)
{
    if (!RTC_Unprotect())
        return false;

    RTC_BBPU = KEY_BBPU | AUTO |
               ((RTC_BBPU | BBPU) & 0xFF);

    if (!RTC_UpdateContext())
        return false;

    return (RTC_BBPU & BBPU) != 0;
}

bool RTC_UnlockBBPower(void)
{
    if (!RTC_Unprotect())
        return false;

    RTC_BBPU = KEY_BBPU |
               ((RTC_BBPU & ~BBPU) & 0xFF);

    if (!RTC_UpdateContext())
        return false;

    return (RTC_BBPU & BBPU) == 0;
}

/* =========================================================
   RTC POWER-UP CONTROL
   ========================================================= */

bool RTC_EnablePowerUp(void)
{
    if (!RTC_Unprotect())
        return false;

    RTC_BBPU = KEY_BBPU |
               ((RTC_BBPU | PWREN) & 0xFF);

    if (!RTC_UpdateContext())
        return false;

    return (RTC_BBPU & PWREN) != 0;
}

bool RTC_DisablePowerUp(void)
{
    if (!RTC_Unprotect())
        return false;

    RTC_BBPU = KEY_BBPU |
               ((RTC_BBPU & ~PWREN) & 0xFF);

    if (!RTC_UpdateContext())
        return false;

    return (RTC_BBPU & PWREN) == 0;
}

/* =========================================================
   PERIODIC INTERRUPTS
   ========================================================= */

void RTC_SetCIIEnable(uint16_t intervals, bool enable)
{
    const uint16_t valid_intervals =
        SECCII1_8 | SECCII1_4 | SECCII1_2 |
        YEACII | MTHCII | DOWCII | DOMCII |
        HOUCII | MINCII | SECCII;

    if (!RTC_Unprotect())
        return;

    RTC_CII_EN = intervals & valid_intervals;

    if (enable)
        RTC_IRQ_EN |= TC_EN;
    else
        RTC_IRQ_EN &= (uint16_t)~TC_EN;

    RTC_UpdateContext();
}

/* =========================================================
   SLEEP / RESUME
   ========================================================= */

void RTC_Sleep(void)
{
    /* Reserved for future power-management integration. */
}

void RTC_Resume(void)
{
    /* Reserved for future power-management integration. */
}
