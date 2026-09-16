//KEYPAD STUFF

#define keypad_backlight (*(volatile uint32_t *) (0xA0700000 + 0x0C80))
#define KP_STATUS (*(volatile uint32_t *) ( 0xA00D0000))

#define KP_MEM1 (*(volatile uint32_t *) (0xa00d0004))
#define KP_MEM2 (*(volatile uint32_t *) (0xa00d0008))
#define KP_MEM3 (*(volatile uint32_t *) (0xa00d000C))
#define KP_MEM4 (*(volatile uint32_t *) (0xa00d0010))
#define KP_MEM5 (*(volatile uint32_t *) (0xa00d0014))

#define KP_STAR (KP_MEM1&1)
#define KP_ZERO ((KP_MEM1>>12)&0x1)
#define KP_HASH ((KP_MEM3 >> 10)&0x1)
#define KP_TAKE ((KP_MEM3 >> 4)&0x1)
#define KP_RIGHT ((KP_MEM3 >> 1)&0x1)
#define KP_LEFT ((KP_MEM2 >> 14)&0x1)
#define KP_OK ((KP_MEM4 >> 9)&0x1)
#define KP_CENTER_UP ((KP_MEM3 >> 13)&0x1)
#define KP_CENTER_DOWN ((KP_MEM4)&0x1)
#define KP_CENTER_LEFT ((KP_MEM4 >> 3)&0x1)
#define KP_CENTER_RIGHT ((KP_MEM4 >> 6)&0x1)
#define KP_1 ((KP_MEM1 >> 15)&0x1)
#define KP_2 ((KP_MEM2 >> 2)&0x1)
#define KP_3 ((KP_MEM2 >> 5)&0x1)
#define KP_4 ((KP_MEM2 >> 8)&0x1)
#define KP_5 ((KP_MEM2 >> 11)&0x1)
#define KP_6 ((KP_MEM3 >> 7)&0x1)
#define KP_7 ((KP_MEM1 >> 9)&0x1)
#define KP_8 ((KP_MEM1 >> 3)&0x1)
#define KP_9 ((KP_MEM1 >> 6)&0x1)
//END OF KEYPAD STUFF
//wdt timer
#define WDT (*(volatile uint32_t *) (0xa003001c))
