#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include "p10_lib.h"

volatile static uint32_t buffer[2][32];
uint8_t * databuffer;
uint32_t * framebuffer;
volatile uint8_t bufferindex;
volatile uint8_t buffernum;

void spi_init_slave (void)
{
 uint8_t tmp;
 DDRB=(1<<PINB4);               //MISO as OUTPUT
 SPCR=(1<<SPE)|(1<<SPIE)|(1<<CPHA)|(1<<CPOL)|(1<<SPI2X);       //Enable SPI && interrupt enable bit
 tmp=SPDR;
 tmp=SPSR;
  
}

ISR(SPI_STC_vect)
{
  databuffer[bufferindex]=SPDR;
  bufferindex++;
  if (bufferindex>127)
  {
      bufferindex=0;
      framebuffer=(uint32_t *) &buffer[buffernum]; 
      buffernum=buffernum^1;
      databuffer=(uint8_t *) &buffer[buffernum]; 
  }
}




int main()
 { 
   int i;
   bufferindex=0; 
   databuffer=(uint8_t *) &buffer[0];
    
   for (i=0;i<128;i++)
    {
      databuffer[i]=0xFF;
    }
    
   databuffer=(uint8_t *) &buffer[1]; 
   for (i=0;i<128;i++)
    {
       databuffer[i]=0xEE;
    }
    buffernum=0;
    databuffer=(uint8_t *) &buffer[0];
    framebuffer=(uint32_t *) &buffer[1]; 
   
    spi_init_slave();                             //Initialize slave SPI
    p10_init();
    sei(); 
    
    
   // Write your code here
   while (1)
   {
      for (i=0;i<8;i++)
      {
      p10_shut();
	_delay_us(85);
      p10_send(framebuffer,i); 
	_delay_us(10);
      }

   }
      
   return 0;
 }
