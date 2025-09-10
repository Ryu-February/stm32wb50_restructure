/*
 * i2c.c
 *
 *  Created on: Sep 9, 2025
 *      Author: RCY
 */

#include "i2c.h"



void i2c_init(void)
{
    // 1. I2C1 클럭 Enable
    RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN;

    // 2. GPIOB 클럭 Enable
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

    // 3. PB8 (SCL), PB9 (SDA) Alternate Function 설정
    GPIOB->MODER &= ~((3 << (8 * 2)) | (3 << (9 * 2)));      // MODER clear
    GPIOB->MODER |= ((2 << (8 * 2)) | (2 << (9 * 2)));       // AF mode

    GPIOB->OTYPER |= (1 << 8) | (1 << 9);                    // Open-drain
    GPIOB->OSPEEDR |= (3 << (8 * 2)) | (3 << (9 * 2));       // High speed

    GPIOB->PUPDR &= ~((3 << (8 * 2)) | (3 << (9 * 2)));
    GPIOB->PUPDR |= ((1 << (8 * 2)) | (1 << (9 * 2)));       // Pull-up

    GPIOB->AFR[1] &= ~((0xF << ((8 - 8) * 4)) | (0xF << ((9 - 8) * 4)));
    GPIOB->AFR[1] |= ((4 << ((8 - 8) * 4)) | (4 << ((9 - 8) * 4)));  // AF4 for I2C1

    // 4. I2C1 Disable before config
    I2C1->CR1 &= ~I2C_CR1_PE;

    // 5. TIMINGR: Standard mode 100kHz @ 64MHz
    I2C1->TIMINGR = 0x106133FF;

    // 6. Enable I2C
    I2C1->CR1 |= I2C_CR1_PE;
}

void i2c_write(uint8_t slave_addr, uint8_t reg_addr, uint8_t data)
{
    I2C1->CR2 = (slave_addr << 1) << I2C_CR2_SADD_Pos;
    I2C1->CR2 |= (2 << I2C_CR2_NBYTES_Pos);   // 레지스터 + 데이터 = 2바이트
    I2C1->CR2 &= ~I2C_CR2_RD_WRN;             // Write
    I2C1->CR2 |= I2C_CR2_START | I2C_CR2_AUTOEND;

    while (!(I2C1->ISR & I2C_ISR_TXIS));
    I2C1->TXDR = reg_addr;

    while (!(I2C1->ISR & I2C_ISR_TXIS));
    I2C1->TXDR = data;

    while (!(I2C1->ISR & I2C_ISR_STOPF));
    I2C1->ICR = I2C_ICR_STOPCF;
}

uint8_t i2c_read(uint8_t slave_addr, uint8_t reg_addr)
{
    uint8_t data;

    // Write phase: 레지스터 주소 보내기
    I2C1->CR2 = (slave_addr << 1) << I2C_CR2_SADD_Pos;
    I2C1->CR2 |= (1 << I2C_CR2_NBYTES_Pos);
    I2C1->CR2 &= ~I2C_CR2_RD_WRN;            // Write
    I2C1->CR2 |= I2C_CR2_START;

    while (!(I2C1->ISR & I2C_ISR_TXIS));
    I2C1->TXDR = reg_addr;

    while (!(I2C1->ISR & I2C_ISR_TC));

    // Read phase: 데이터 1바이트 읽기
    I2C1->CR2 = (slave_addr << 1) << I2C_CR2_SADD_Pos;
    I2C1->CR2 |= (1 << I2C_CR2_NBYTES_Pos);
    I2C1->CR2 |= I2C_CR2_RD_WRN;            // Read
    I2C1->CR2 |= I2C_CR2_START | I2C_CR2_AUTOEND;

    while (!(I2C1->ISR & I2C_ISR_RXNE));
    data = I2C1->RXDR;

    while (!(I2C1->ISR & I2C_ISR_STOPF));
    I2C1->ICR = I2C_ICR_STOPCF;

    return data;
}
