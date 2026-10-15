/*
 * LSM6DSL.c
 *
 *  Created on: Feb 7, 2025
 *      Author: Angel Jimenez
 */

#include "stm32l4xx_hal.h"
#include "LSM6DSL.h"

extern I2C_HandleTypeDef hi2c2;

void LSM6DSL_Init(){

	uint8_t buffer[1];
	buffer[0] = ODR_XL_416Hz;
	HAL_I2C_Mem_Write(&hi2c2, LSM6DSL_ADDR, REG_CTRL1_XL, I2C_MEMADD_SIZE_8BIT, buffer, 1, 1000);

}

uint8_t LSM6DSL_DataReady(){

	uint8_t buffer[1];
    HAL_I2C_Mem_Read(&hi2c2, LSM6DSL_ADDR, REG_STATUS, I2C_MEMADD_SIZE_8BIT, buffer, 1, 1000);
    return ((buffer[0] & 0x01) != 0);

}

void LSM6DSL_ReadAccel(float accel[]){

	uint8_t buffer[6];

	HAL_I2C_Mem_Read(&hi2c2, LSM6DSL_ADDR, REG_OUTX_L_XL, I2C_MEMADD_SIZE_8BIT, buffer, 6, 1000);

	for(uint8_t i = 0; i < 3; i++){
		accel[i] = ((int16_t)(buffer[2*i+1]<<8) | buffer[2*i])*0.061f;
	}

}
