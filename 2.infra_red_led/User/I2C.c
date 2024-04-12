#include "I2C.h"

void I2C_start(void)
{
	I2C_SCL = 1;   //bus free
	I2C_SDA = 1;
	I2C_DELAY_4US; //keep 4.7us
	I2C_SDA = 0;
	I2C_DELAY_4US;
	I2C_SCL = 0;
}

bit I2C_write_byte(unsigned char dat)
{
	unsigned char temp;
	bit ack;
	for (temp = 0x80; temp != 0; temp >>= 1) {
		if ((temp & dat) == 0) //if current bit is 0
			I2C_SDA = 0;
		else
			I2C_SDA = 1;
		
		//prepare for sending the next bit
		I2C_DELAY_4US;
		I2C_SCL = 1;
		I2C_DELAY_4US;
		I2C_SCL = 0;
	}
	ack = I2C_recv_ack();
	return (ack);
}

bit I2C_recv_ack(void)
{
	bit ack;
	
	I2C_SDA = 1;   //release bit, ready for ack
	I2C_DELAY_4US; //keep SCL for 4.7us as 0 bit
	I2C_SCL = 1;
	ack = I2C_SDA;
	I2C_DELAY_4US; //keep SCL as 1 bit for 4us at least
	I2C_SCL = 0;
	return (ack);
}

unsigned char I2C_read_byte(void)
{
	unsigned char dat = 0;
	unsigned char temp;
	I2C_SDA = 1;
	for (temp = 0x80; temp != 0; temp >>= 1) {
		I2C_DELAY_4US;
		I2C_SCL = 1;
		if (I2C_SDA == 1)
			dat |= temp;
		else
			dat &= ~temp;
		I2C_DELAY_4US;
		I2C_SCL = 0;
	}
	
	return dat;
}

void send_ack(bit ack)
{
	I2C_SDA = ack;
	I2C_DELAY_4US;
	I2C_SCL = 1;
	I2C_DELAY_4US;
	I2C_SCL = 0;
}

void I2C_stop(void)
{
	I2C_SCL = 0;
	I2C_SDA = 0;
	I2C_DELAY_4US;
	I2C_SDA = 1;
	I2C_DELAY_4US;
	I2C_SCL = 1;
	I2C_DELAY_4US;
}

