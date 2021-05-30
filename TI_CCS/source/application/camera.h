/*
 * camera.h
 *
 *  Created on: Sep 4, 2020
 *      Author: yubo
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include <stdio.h>
#include <string.h>



#define HM01B0ADDR 0x24 /* camera I2C address */
#define framesize 10000 // 40016

extern uint8_t image_ready;

/* init i2c */
void i2c_init_camera(void);

uint8_t read_camera(uint16_t regaddr);

void write_camera(uint16_t regaddr, uint8_t data);

void write_verify_camera(uint16_t regaddr, uint8_t data);

void config_camera(void);

void config_dma(void);

void config_TimerA0(void);

void reset_camera(void);

uint8_t capture_image(void);

#endif /* CAMERA_H_ */
