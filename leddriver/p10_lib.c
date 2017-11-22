#include "p10_lib.h"
#include <util/delay.h>

/*
8 строк
D не работает!!! мляяя
64 бита регистр сдвиговый
2 регистра сдвиговых


*/


#define A   PC2
#define B   PC3
#define C   PC4
#define D   PC5

#define R1  PC1
#define R2  PC0


#define EO_S  PORTD |= (1<<PD5) 
#define CLK_S PORTD |= (1<<PD6)
#define STR_S PORTD |= (1<<PD7)

#define EO_C  PORTD &=~ (1<<PD5)
#define CLK_C PORTD &=~ (1<<PD6)
#define STR_C PORTD &=~ (1<<PD7)


#define D_DDR (DDRD |= (1 << PD5)|(1 << PD6)|(1 << PD7))
#define C_DDR (DDRC |= (1 << A)|(1 << B)|(1 << C)|(1 << D)|(1 << R1)|(1 << R2))





void p10_init()
{
 EO_S;
 CLK_C;
 STR_C;
 D_DDR;
 C_DDR;
 DDRD=0xFF;
 DDRC=0xFF;
}

void p10_shut()
{
	EO_S;
}


void p10_send(uint32_t * data, uint8_t row)
{
	uint8_t j,mask;
	uint32_t b1,b2,b3,b4;
	b1=data[row*2];
	b2=data[row*2+1];

	b3=data[row*2+16];
	b4=data[row*2+1+16];
	mask=row<<3;

	for (j=0;j<32;j++)
	{
			
			CLK_C;
			PORTC=((b1&1)*2)|(b2&1);
			b1=(b1>>1);
			CLK_S;
			b2=(b2>>1);

	}

	for (j=0;j<32;j++)
	{
			CLK_C;
			PORTC=((b3&1)*2)|(b4&1);
			b3=(b3>>1);
			CLK_S;
			b4=(b4>>1);

	}	



	asm("nop");
	STR_C;
	asm("nop");
	STR_S;
	asm("nop");
	STR_C;
	PORTC=mask;
	EO_C;
}

