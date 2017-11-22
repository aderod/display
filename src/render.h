#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <ft2build.h>
#include FT_FREETYPE_H



#define WIDTH   64
#define HEIGHT  16
#define LEDCNT 1024

//int (*A)[4]
void normalize_buffer();

void draw_bitmap( FT_Bitmap*  bitmap, FT_Int x, FT_Int y);
void send_buffer_spi(); //sending frame to videodriver ~100 KHZ ~48 fps 
void clear_buffer();
void show_image();
void render_text(char * txt); //txt render to image, image normalize to framebuffer and send to panel
