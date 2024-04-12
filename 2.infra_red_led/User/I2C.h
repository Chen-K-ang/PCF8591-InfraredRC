#ifndef BUS_I2C_H
#define BUS_I2C_H

#include <reg52.h>
#include <intrins.h>

#define I2C_DELAY_4US {_nop_();_nop_();_nop_();_nop_();}

sbit I2C_SCL = P1^4;
sbit I2C_SDA = P1^5;

void I2C_start(void);
bit I2C_write_byte(unsigned char dat);
bit I2C_recv_ack(void);
unsigned char I2C_read_byte(void);
void send_ack(bit ack);
void I2C_stop(void);

#endif
