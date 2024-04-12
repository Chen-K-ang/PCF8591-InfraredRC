#include "pcf8591.h"

void PCF8591_dac(unsigned char addr, unsigned char dat)
{
	I2C_start();
	I2C_write_byte(addr); //not read ack
	I2C_write_byte(0x40);
	I2C_write_byte(dat);
	I2C_stop();
}

unsigned char PCF8591_adc(unsigned char addr, unsigned char channel)
{
	unsigned char ad_result = 0;
	I2C_start();
	if (I2C_write_byte(addr << 1)) { //not read ack
		I2C_stop();
		return 0;
	}
	I2C_write_byte(0x40);
	I2C_write_byte(channel);
	
	I2C_start(); //restart
	if (I2C_write_byte((addr << 1) | 0x01)) {
		I2C_stop();
		return 0;
	}
	I2C_read_byte(); //release the last date
	send_ack(0);     //send to pcf8591 a ack signal
	ad_result = I2C_read_byte();
	send_ack(1);
	I2C_stop();
	
	return (ad_result); 
}
