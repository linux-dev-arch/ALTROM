#include <stdint.h>
#include "defines.h"
#include "watchdog.c"
//#include "divmod.c"
#include "init_hw.c"
#include "helpers.c"
#define SD_NO_TEST
#define FAT_NO_UI
#include "sd.c"
#include "fat.c"
//#include "battery.c"
#include "bat.c"
//#include "gpt.c"
#define ALTROM_ENTRY 0x70000000
int load_file(const char *path, uint32_t address)
{
    fat_file_t file;

    if (fat_open(&file, path) != FAT_OK)
        return -1;

    uint32_t size = file.size;

    if (size == 0)
        return -2;

    int bytes = fat_read(&file, (void *)address, size);

    if (bytes != (int)size)
        return -3;

    return (int)size;
}

void entry_main(void){
	init();
	sd_init();
	fat_mount();
	//battery_init();
	Force_Battery_Power_Hold();
	if(!(KP_5==0)){return;}
	int size = load_file("ALTROM.BIN", ALTROM_ENTRY);
	if (size > 0) {
    		void (*altrom_entry)(void) = (void (*)(void))ALTROM_ENTRY;
    		altrom_entry();
	}
}
