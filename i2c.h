#ifndef I2C_H_
#define I2C_H_

extern void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
void i2c_transmit_ack(unsigned char ack);
unsigned char i2c_receive_ack(void);
extern unsigned char i2c_transmit_frame(unsigned char slave_addr, unsigned char *p_buff, unsigned char num);
extern unsigned char i2c_receive_frame(unsigned char slave_addr, unsigned char *p_buff, unsigned char num);
extern void i2c_transmit_char(unsigned char data);
extern unsigned char i2c_receive_char(unsigned char ack);

#endif /* I2C_H_ */


