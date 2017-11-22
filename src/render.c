
#include "gpio.h"
#include "render.h"

int normalize_array[] ={
0,	1,	2,	3,	64,	65,	66,	67,
32,	33,	34,	35,	96,	97,	98,	99,
16,	17,	18,	19,	80,	81,	82,	83,
48,	49,	50,	51,	112,	113,	114,	115,
8,	9,	10,	11,	72,	73,	74,	75,
40,	41,	42,	43,	104,	105,	106,	107,
24,	25,	26,	27,	88,	89,	90,	91,
56,	57,	58,	59,	120,	121,	122,	123,
4,	5,	6,	7,	68,	69,	70,	71,
36,	37,	38,	39,	100,	101,	102,	103,
20,	21,	22,	23,	84,	85,	86,	87,
52,	53,	54,	55,	116,	117,	118,	119,
12,	13,	14,	15,	76,	77,	78,	79,
44,	45,	46,	47,	108,	109,	110,	111,
28,	29,	30,	31,	92,	93,	94,	95,
60,	61,	62,	63,	124,	125,	126,	127
};

static uint8_t framebuffer[LEDCNT/8];
static uint8_t databuffer[LEDCNT/4]; //данные храняться в строковом виде 00..FF поэтому и в два раза больше буфер
static uint8_t image[HEIGHT][WIDTH];

// returns the number of utf8 code points in the buffer at s
size_t utf8len(char *s)
{
    size_t len = 0;
    for (; *s; ++s) if ((*s & 0xC0) != 0x80) ++len;
    return len;
}

// returns a pointer to the beginning of the pos'th utf8 codepoint
// in the buffer at s
char *utf8index(char *s, size_t pos)
{    
    ++pos;
    for (; *s; ++s) {
        if ((*s & 0xC0) != 0x80) --pos;
        if (pos == 0) return s;
    }
    return NULL;
}

// converts codepoint indexes start and end to byte offsets in the buffer at s
void utf8slice(char *s, ssize_t *start, ssize_t *end)
{
    char *p = utf8index(s, *start);
    *start = p ? p - s : -1;
    p = utf8index(s, *end);
    *end = p ? p - s : -1;
}


unsigned long utf8chr(unsigned long chr)
{
   static unsigned long tmp;
   if (chr<0x80) return chr;

		tmp=0;
		tmp=chr & 0x3fUL;
		tmp=tmp |((chr>>2) & 0x07c0UL);
		return tmp;

}


void clear_buffer()
{
	int i;
    	for (i=0;i<LEDCNT/4;i++)
	{	
		databuffer[i]='0';
    	}

}

void normalize_buffer()
{
	int i;
	uint8_t buf[4];
	uint8_t tmp;
	for (i=0;i<128;i++)
	{	
		buf[0]=databuffer[i*2];
		buf[1]=databuffer[i*2+1];
		//memcpy(buf,databuffer[i*2],2);
		buf[2]='\0';
		tmp=strtol(buf,NULL,16);
		framebuffer[normalize_array[i]]=~tmp;
	}
}



void draw_bitmap( FT_Bitmap*  bitmap,
             FT_Int      x,
             FT_Int      y)
{
  FT_Int  i, j, p, q;
  FT_Int  x_max = x + bitmap->width;
  FT_Int  y_max = y + bitmap->rows;

  for ( i = x, p = 0; i < x_max; i++, p++ )
  {
    for ( j = y, q = 0; j < y_max; j++, q++ )
    {
      if ( i < 0      || j < 0       ||
           i >= WIDTH || j >= HEIGHT )
        continue;
      image[j][i] |= bitmap->buffer[q * bitmap->width + p];
    }
  }
}

void send_buffer_spi() //sending frame to videodriver ~100 KHZ ~48 fps 
{
 int j,i;
 uint8_t tmp;
 int cs,data,clk;	

 gpioExport(cs_pin);
 gpioDirection(cs_pin,1);

 gpioExport(clk_pin);
 gpioDirection(clk_pin,1);

 gpioExport(data_pin);
 gpioDirection(data_pin,1);

 cs=gpioOpen(cs_pin);
 clk=gpioOpen(clk_pin);
 data=gpioOpen(data_pin);
 

 gpioSet(cs,'0');
 usleep(20);
 for (i=0;i<LEDCNT/8;i++)
 {
	tmp=framebuffer[i];
	for (j=0;j<8;j++)
	{	
		gpioSet(clk,'0');
		if ((tmp&1)==1)
		{
			gpioSet(data,'1');
		}else{
			gpioSet(data,'0');
		}
		tmp=tmp>>1;
		usleep(20);
		gpioSet(clk,'1');
		usleep(20);
	}
 }
 gpioSet(cs,'1');

    gpioClose(cs);
    gpioClose(clk);
    gpioClose(data);

    gpioUnExport(cs_pin);
    gpioUnExport(clk_pin);
    gpioUnExport(data_pin);

}



void show_image()
{
	int i,j,t;
	uint8_t byte;
	for ( i = 0; i < HEIGHT; i++ )
	  {
		    for ( j = 0; j < WIDTH/8; j++ )
			{
				byte=0;
				for (t=0; t<8 ;t++)
				{
					byte=byte<<1;
					if (image[i][j*8+t]==0)
					{
						byte|=1;
						printf("1");
						
					}else{
						printf("0");
					}
					

				}
				framebuffer[normalize_array[i*WIDTH/8+j]]=byte;
			}
			printf("\n");
  	}
	send_buffer_spi();	
}



void render_text(char * txt)
{
	FT_Library    library;
  	FT_Face       face;
	FT_GlyphSlot  slot;
	FT_Matrix     matrix;                 /* transformation matrix */
	FT_Vector     pen;                    /* untransformed origin  */
	FT_Error      error;    
	double        angle;
	int           target_height;
	int           n, num_chars;
	int		i,j;
	

	for ( i = 0; i < HEIGHT; i++ )
	  {
		    for ( j = 0; j < WIDTH; j++ )
			{
				image[i][j]=0;
			}
	}

	num_chars     = utf8len( txt );
	target_height = HEIGHT;
	error = FT_Init_FreeType( &library );              /* initialize library */
	 /* error handling omitted */
	error = FT_New_Face( library, "/usr/share/PTM55F.ttf", 0, &face );/* create face object */
	/* error handling omitted */
	//FT_Set_Pixel_Sizes(&face, 1, 0);

	error = FT_Set_Char_Size(
          face,    /* handle to face object           */
          9*64,       /* char_width in 1/64th of points  */
          13*64,   /* char_height in 1/64th of points */
          110,     /* horizontal device resolution    */
          110 );   /* vertical device resolution      */
	/* error handling omitted */

	slot = face->glyph;



	/* the pen position in 26.6 cartesian space coordinates; */
	/* start at (300,200) relative to the upper left corner  */
	pen.x = 0;
	pen.y = 1<<6;

	for ( n = 0; n < num_chars; n++ )
	{
  	        FT_Set_Transform( face, 0, &pen );
		/* load glyph image into the slot (erase previous one) */
		ssize_t start = n;
		ssize_t end = n+1;
		utf8slice(txt, &start, &end);

		unsigned long chr=0;
		    if (end>0)
			{
			for (int i=start;i<end;i++)
			{
		
				chr=chr<<8;
				chr+=txt[i]&0xFF;
			}
			}else{
			for (int i=start;i<strlen(txt);i++)
			{
		
				chr=chr<<8;
				chr+=txt[i]&0xFF;
			}
	
	
			}

		unsigned long c = FT_Get_Char_Index(face,utf8chr(chr));
		/* load glyph image into the slot (erase previous one) */
		FT_Load_Glyph(face,c,FT_LOAD_RENDER);


		/* now, draw to our target surface (convert position) */
		draw_bitmap( &slot->bitmap,slot->bitmap_left,target_height - slot->bitmap_top);

		/* increment pen position */
		pen.x += slot->advance.x+64;
		//pen.y += slot->advance.y;
	}

  show_image();

  FT_Done_Face    ( face );
  FT_Done_FreeType( library );

}

