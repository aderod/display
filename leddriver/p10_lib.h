#pragma once
#include <avr/io.h>


void p10_init();
void p10_shut();
void p10_send(uint32_t * data, uint8_t row);
