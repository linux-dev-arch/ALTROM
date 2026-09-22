#include <stdint.h>
#include "defines.h"
#include "init_hw.c"
#include "helpers.c"
#include "apps.c"

void entry_main(void)
{	init();

	for(;;){
		kp_blink(100000);
		kp_toggle();
		psram_test();
	}

}
