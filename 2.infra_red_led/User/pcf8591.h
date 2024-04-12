#ifndef HARDWARE_PCF8591_H
#define HARDWARE_PCF8591_H

#include "I2C.h"

void PCF8591_dac(unsigned char addr, unsigned char dat);
unsigned char PCF8591_adc(unsigned char addr, unsigned char channel);

#endif
