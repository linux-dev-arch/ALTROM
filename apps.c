#include "file_mgr.c"
void bounce(void){
	int x=120;
	int y=160;
	int old_x;
	int old_y;
	int speed_x=5;
	int speed_y=5;
	fb_clear(COL_BLACK);
	fb_rect(x,y,15,10,COL_GREEN);
	fb_flush();
	for(;;){
		if(KP_LEFT==0){
			sleep(10000);
			break;
		}
		old_x=x;
		old_y=y;
		x+=speed_x;
		y+=speed_y;
		if(x+15>240 || x <= 0){
			speed_x=-speed_x;
		}
		if(y+15>320 || y <= 0){
			speed_y=-speed_y;
		}
		fb_rect(old_x,old_y,15,15,COL_BLACK);
        	fb_rect(x,y,15,15,COL_GREEN);
        	fb_flush();
		watchdog_feed();
	}
}
/*
void calc(void){
	int32_t out =0;
	int32_t temp=0;
	int32_t in=0;
	fb_clear(COL_BLACK);
	fb_rect(10,30,220,40,COL_GREEN);
	draw_int(10,50,in,COL_BLACK,COL_GREEN);
	fb_flush();
	for(;;){
		if(KP_LEFT==0){break;}
		in=kp_read_int();
		draw_int(10,50,in,COL_BLACK,COL_GREEN);
		fb_flush();
		temp=in;
                in=kp_read_int();
		fb_clear(COL_BLACK);
                fb_rect(10,30,220,40,COL_GREEN);
		draw_int(10,50,in,COL_BLACK,COL_GREEN);
		fb_flush();
		for(;;){
			watchdog_feed();
			if(KP_CENTER_UP==0){
				//add
				out=temp+in;
				break;
			}
			if(KP_CENTER_DOWN==0){
				//sub
				out=temp-in;
				break;
			}
			if(KP_CENTER_LEFT==0){
                        	//mul
                        	out=temp*in;
				break;
                	}
			if(KP_CENTER_RIGHT==0){
                        	//div
                        	out=temp/in;
				break;
                	}
		}
		in=0;
		fb_clear(COL_BLACK);
        	fb_rect(10,30,220,40,COL_GREEN);
        	draw_int(150,40,out,COL_BLACK,COL_GREEN);
        	fb_flush();
		for(;;){
			if(KP_OK==0){break;}
			watchdog_feed();
		}
		watchdog_feed();
	}
}*/
void calc(void)
{
    int32_t a = 0;
    int32_t b = 0;
    int32_t result = 0;
    int op = 0;

    for (;;) {
        watchdog_feed();

        fb_clear(COL_BLACK);
        draw_string8(10, 10, "CALCULATOR",
                     COL_GREEN, COL_BLACK, 0, 0);
        draw_string8(10, 60, "ENTER FIRST NUMBER",
                     COL_WHITE, COL_BLACK, 0, 0);
        fb_flush();

        a = kp_read_int();
        delay_ms(250);

        fb_clear(COL_BLACK);
        draw_string8(10, 10, "SELECT OPERATOR",
                     COL_GREEN, COL_BLACK, 0, 0);
        draw_string8(10, 60, "UP    : ADD",
                     COL_WHITE, COL_BLACK, 0, 0);
        draw_string8(10, 85, "DOWN  : SUBTRACT",
                     COL_WHITE, COL_BLACK, 0, 0);
        draw_string8(10, 110, "LEFT  : MULTIPLY",
                     COL_WHITE, COL_BLACK, 0, 0);
        draw_string8(10, 135, "RIGHT : DIVIDE",
                     COL_WHITE, COL_BLACK, 0, 0);
        fb_flush();

        for (;;) {
            watchdog_feed();

            if (KP_CENTER_UP == 0) {
                op = 1;
                break;
            }
            if (KP_CENTER_DOWN == 0) {
                op = 2;
                break;
            }
            if (KP_CENTER_LEFT == 0) {
                op = 3;
                break;
            }
            if (KP_CENTER_RIGHT == 0) {
                op = 4;
                break;
            }
        }

        delay_ms(200);

        fb_clear(COL_BLACK);
        draw_string8(10, 10, "ENTER SECOND NUMBER",
                     COL_GREEN, COL_BLACK, 0, 0);
        draw_int(10, 60, a, COL_WHITE, COL_BLACK);
        fb_flush();

        b = kp_read_int();
        delay_ms(250);

        if (op == 1)
            result = a + b;
        else if (op == 2)
            result = a - b;
        else if (op == 3)
            result = a * b;
        else if (op == 4 && b != 0)
            result = a / b;

        delay_ms(500);

        fb_clear(COL_BLACK);
        draw_string8(10, 10, "RESULT",
                     COL_GREEN, COL_BLACK, 0, 0);

        draw_int(10, 50, a, COL_WHITE, COL_BLACK);

        if (op == 1)
            draw_string8(75, 50, "+", COL_GREEN, COL_BLACK, 0, 0);
        else if (op == 2)
            draw_string8(75, 50, "-", COL_GREEN, COL_BLACK, 0, 0);
        else if (op == 3)
            draw_string8(75, 50, "*", COL_GREEN, COL_BLACK, 0, 0);
        else
            draw_string8(75, 50, "/", COL_GREEN, COL_BLACK, 0, 0);

        draw_int(100, 50, b, COL_WHITE, COL_BLACK);

        if (op == 4 && b == 0) {
            draw_string8(10, 100, "DIVISION BY ZERO!",
                         COL_RED, COL_BLACK, 0, 0);
        } else {
            draw_string8(10, 100, "ANSWER:",
                         COL_GREEN, COL_BLACK, 0, 0);
            draw_int(100, 100, result, COL_WHITE, COL_BLACK);
        }

        draw_string8(10, 180, "OK: AGAIN",
                     COL_WHITE, COL_BLACK, 0, 0);
        draw_string8(10, 205, "LEFT: EXIT",
                     COL_WHITE, COL_BLACK, 0, 0);

        fb_flush();

        for (;;) {
            watchdog_feed();

            if (KP_OK == 0) {
                delay_ms(300);
                break;
            }

            if (KP_LEFT == 0)
                return;
        }
    }
}

void snake(void){
	int x=120;
	int y=160;
	int x_vel=10;
	int y_vel=0;
	int old_x=0;
	int old_y=0;
	int speed=10;
	fb_clear(COL_BLACK);
	for(;;){
		fb_clear(COL_BLACK);
		if(x > 240){
			x=0;
		}
		if(x < 0){
			x=239;
		}
		if(y > 320){
			y=0;
		}
		if(y < 0){
			y=319;
		}
		if(KP_CENTER_UP==0 && y_vel < 10){
			y_vel=-speed;
			x_vel=0;
		}
		if(KP_CENTER_DOWN==0 && y_vel > -10){
                        y_vel=speed;
			x_vel=0;
                }
		if(KP_CENTER_LEFT==0 && x_vel < 10){
                        x_vel=-speed;
			y_vel=0;
                }
		if(KP_CENTER_RIGHT==0 && x_vel > -10){
                        x_vel=speed;
			y_vel=0;
                }
		if(KP_LEFT==0){
			break;
		}
		x+=x_vel;
		y+=y_vel;
		fb_rect(old_x,old_y,15,15,COL_BLACK);
                fb_rect(x,y,15,15,COL_GREEN);
		fb_flush();
		watchdog_feed();
	}
}

