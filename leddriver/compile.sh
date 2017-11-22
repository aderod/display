avr-gcc -mmcu=atmega328 -I. -gdwarf-2 -DF_CPU=8000000UL -O1 -o bin/main.o main.c p10_lib.c
avr-objcopy -O ihex bin/main.o bin/main.hex

