#include <stdint.h>

#define MAX_ENTRIES 128
#define MAX_PATH    128
#define FAT_ATTR_DIRECTORY 0x10
#define FAT_ATTR_VOLUME_ID 0x08

static char current_path[MAX_PATH] = "/";
static fat_ent_t entries[MAX_ENTRIES];
static int entry_count = 0;


int has_ext(const char *name, const char *ext)
{
    int i = 0;

    while (name[i]) i++;

    if (i < 4) return 0;

    return name[i-4] == '.' &&
           name[i-3] == ext[0] &&
           name[i-2] == ext[1] &&
           name[i-1] == ext[2];
}

int load_file(const char *path, uint32_t address)
{
    fat_file_t file;

    if (fat_open(&file, path) != FAT_OK)
        return -1;

    uint32_t size = file.size;

    int bytes = fat_read(&file, (void *)address, size);

    if (bytes != (int)size)
        return -2;

    return size;
}

void make_path(char *dst, const char *dir, const char *name)
{
    int i = 0, j = 0;

    while (dir[i]) dst[j++] = dir[i++];

    if (j > 1) dst[j++] = '/';

    i = 0;
    while (name[i]) dst[j++] = name[i++];

    dst[j] = '\0';
}

/* Copy string */
void fm_strcpy(char *dst, const char *src)
{
    while (*src) {
        *dst++ = *src++;
    }

    *dst = '\0';
}


/* Get string length */
int fm_strlen(const char *s)
{
    int len = 0;

    while (s[len])
        len++;

    return len;
}


/* Append a string */
int fm_append(char *dst, const char *src)
{
    int i = fm_strlen(dst);
    int j = 0;

    while (src[j] && i < MAX_PATH - 1) {
        dst[i++] = src[j++];
    }

    dst[i] = '\0';

    return src[j] == '\0';
}


/* Enter a directory */
int fm_enter_dir(const char *name)
{
    int len = fm_strlen(current_path);

    if (len + fm_strlen(name) + 2 >= MAX_PATH)
        return 0;

    if (len > 1)
        current_path[len++] = '/';

    current_path[len] = '\0';

    if (!fm_append(current_path, name))
        return 0;

    return 1;
}


/* Go to parent directory */
void fm_parent_dir(void)
{
    int len = fm_strlen(current_path);

    if (len <= 1)
        return;

    while (len > 1 && current_path[len - 1] != '/')
        len--;

    if (len > 1)
        len--;

    current_path[len] = '\0';

    if (len == 0)
        fm_strcpy(current_path, "/");
}


/* Draw directory listing */
void fm_refresh(void)
{
    fat_dir_t dir;

    entry_count = 0;

    fb_clear(COL_BLACK);

    draw_string8(4, 2, current_path,
                 COL_WHITE, COL_BLACK, 0, 0);

    if (fat_opendir(&dir, current_path) != FAT_OK) {
        draw_string8(4, 20, "DIR OPEN FAILED",
                     COL_RED, COL_BLACK, 0, 0);
        fb_flush();
        return;
    }

    while (entry_count < MAX_ENTRIES &&
           fat_dir_next(&dir, &entries[entry_count]) == 0) {

        if (entries[entry_count].attr & FAT_ATTR_VOLUME_ID)
            continue;

        draw_int8(4, 20 + entry_count * 14,
                 entry_count, COL_GREEN, COL_BLACK);

        draw_string8(30, 20 + entry_count * 14,
                     entries[entry_count].name,
                     COL_WHITE, COL_BLACK, 0, 0);

        entry_count++;
    }

    fb_flush();
}


/* File manager */
void file_mgr(void)
{
    int selected;
    int r = sd_init();
    if (r != 0) {
        fb_clear(COL_BLACK);
        draw_string12(4, 100, "SD INIT FAILED",
                      COL_RED, COL_BLACK, 0, 0);
        fb_flush();
        return;
    }
    r = fat_mount();
    if (r != FAT_OK){
        fb_clear(COL_BLACK);
        draw_string12(4, 100, "SD MOUNT FAILED",
                      COL_RED, COL_BLACK, 0, 0);
        fb_flush();
        return;
    }
    fm_refresh();
    fm_strcpy(current_path, "/");
    watchdog_feed();
    for (;;) {
	watchdog_feed();
        if (KP_RIGHT == 0) {
              break;
        }
        fm_refresh();

        draw_string8(4, 300, "NUMBER: SELECT",
                     COL_WHITE, COL_BLACK, 0, 0);

        fb_flush();

        selected = kp_read_int();

        if (selected < 0 || selected >= entry_count)
            continue;

        /* Selected a directory */
        if (entries[selected].attr & FAT_ATTR_DIRECTORY) {

            if (!fm_enter_dir(entries[selected].name))
                continue;

        } else {

            /* Selected a file */
            fb_clear(COL_BLACK);

            draw_string12(4, 100, "SELECTED FILE:",
                          COL_WHITE, COL_BLACK, 0, 0);

            draw_string12(4, 130, entries[selected].name,
                          COL_WHITE, COL_BLACK, 0, 0);

            draw_string8(4, 160, "SIZE:",
                         COL_WHITE, COL_BLACK, 0, 0);

            draw_int(50, 160, entries[selected].size,
                     COL_WHITE, COL_BLACK);

            fb_flush();

            /* Wait for exit */
            while (KP_LEFT != 0){
		watchdog_feed();
		if(KP_RIGHT ==0){
			if(has_ext(entries[selected].name,"BIN")){
				char path[128];
				make_path(path, current_path, entries[selected].name);
				int size= load_file(path,0x00000000);
				if(size>0){
					fb_clear(COL_BLACK);
					draw_string12(4,100,"Loaded file!",COL_WHITE,COL_BLACK,0,0);
					fb_flush();
				}
		       }//0x002DA400-chip8 rom address
			else if(has_ext(entries[selected].name,"CH8")){
				char path[128];
                                make_path(path, current_path, entries[selected].name);
                                int size= load_file(path,0x002DA400);
                                if(size>0){
                                        fb_clear(COL_BLACK);
                                        draw_string12(4,100,"Loaded file!",COL_WHITE,COL_BLACK,0,0);
                                        fb_flush();
                                }
			}
		}
	    }
        }
    }
}
