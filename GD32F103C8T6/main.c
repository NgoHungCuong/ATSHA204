/**
  ******************************************************************************
  * @file    SysTick/Systick_demo/main.c
  * @author  MCU SD
  * @version V1.0
  * @date    26-Dec-2014
  * @brief   The main function file.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "gd32f10x.h"
#include <stdio.h>
#include "systick.h"
#include "soft_i2c.h"
#include "sha204.h"
#include <string.h>

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Configure the GPIO ports.
  * @param  None
  * @retval None
  */
  
  
#define LED_OFF GPIO_SetBits(GPIOC, GPIO_PIN_13)
#define LED_ON GPIO_ResetBits(GPIOC, GPIO_PIN_13)


void LED_config(void)
{    
    GPIO_InitPara GPIO_InitStructure;

    RCC_APB2PeriphClock_Enable(RCC_APB2PERIPH_GPIOC,ENABLE);

    GPIO_InitStructure.GPIO_Pin   = GPIO_PIN_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_SPEED_50MHZ;
    GPIO_InitStructure.GPIO_Mode  = GPIO_MODE_OUT_PP;

    GPIO_Init(GPIOC,&GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOC, GPIO_PIN_13);
	//khoi tao nut bam
	//co 2 nut bam
	//nut bam noi vao PA0 va PA1
	RCC_APB2PeriphClock_Enable(RCC_APB2PERIPH_GPIOA,ENABLE);

    GPIO_InitStructure.GPIO_Pin   = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_SPEED_50MHZ;
    GPIO_InitStructure.GPIO_Mode  = GPIO_MODE_IN_FLOATING;

    GPIO_Init(GPIOA,&GPIO_InitStructure);
}


/**
  * @brief  Toggle the LED every 500ms.
  * @param  None
  * @retval None
  */

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */

void Delay(void) {
	uint32_t i;
	
	for(i=0; i<120; ++i) {
		
	}
}

void LongDelay(uint32_t u32Delay) {
	uint32_t i;
	for(i=0; i<u32Delay; ++i) {
		
	}
}

int main(void)
{
    uint8_t i;
	uint8_t u8Tmp[64];
	uint8_t szBuff[64];
	uint32_t dwTimeOut;
	uint8_t szRanOut[100];
	uint8_t szNumIn[100];
	uint8_t szTempKey[100];
	uint8_t szMac[100];
	uint8_t szInPut[100];
	uint8_t szOutPut[100];
	char szSlot0[100]="Ngo Hung Cuong Data Zone 0\0\0\0\0\0\0\0";
	
	LED_config();
	
	KT_I2C_Init();
	//danh thuc neu chua duoc danh thuc
	KT_I2C_Start();
	
	if(KT_I2C_Write(0xC9)) {
		Wake();
		KT_I2C_Stop();
	} else {
		KT_I2C_Stop();
	}
	//doc thu 4 byte
	KT_I2C_Start();
	
	if(KT_I2C_Write(0xC9)) {
		LED_ON;
		while(1);
	}
	
	//while(1);
	
	for(i=0; i<3; ++i) {
		u8Tmp[i]=KT_I2C_Read(0);
	}
	u8Tmp[3]=KT_I2C_Read(1);
	KT_I2C_Stop();
	
	if(u8Tmp[0]!=0x04) {
		LED_ON;
		while(1);
	}
	
	//while(1);
	szBuff[0]=9;
	szBuff[1]=0xC8;
	szBuff[2]=0x03;
	szBuff[3]=0x07;//so luong byte
	szBuff[4]=0x02;//bat dau lenh
	szBuff[5]=0x80;// 32 byte vung config
	szBuff[6]=0x00;
	szBuff[7]=0x00;
	
	sha204c_calculate_crc(szBuff[3]-2, &szBuff[3], &szBuff[8]);
	
	KT_I2C_Start();
	for(i=1; i<10; ++i) {
		if(KT_I2C_Write(szBuff[i])) {
			LED_ON;
			while(1);
		}
	}
	//thu doc du lieu vung config
	KT_I2C_Stop();
	
	//hoi vong den khi doc duoc du lieu thi thoi
	dwTimeOut=1;
	for(i=0; i<4; ++i) {
		LongDelay(10000);
		KT_I2C_Start();
		if(!KT_I2C_Write(0xC9)) {
			dwTimeOut=0;
			break;
		}
		KT_I2C_Stop();
	}
	
	if(dwTimeOut) {
		LED_ON;
		while(1);
	}
	
	//doc tiep 0x23 byte
	//byte cuoi cung Ack 1
	for(i=0; i<0x22; ++i) {
		szBuff[i]=KT_I2C_Read(0);
	}
	szBuff[0x22]=KT_I2C_Read(1);
	
	if(szBuff[1]!=0x01||szBuff[2]!=0x23||szBuff[13]!=0xee) {
		LED_ON;
		while(1);
		//khong thay SHA204
	}
	KT_I2C_Stop();
	//=====================================================
	for(i=0; i<20; ++i) {
		szNumIn[i]=0x00;//rand()%256;
		//co the thay bang 20 byte ngau nhien neu muon an toan cao hon
	}
	
	//gui lenh NONCE
	szBuff[1]=0xC8;//SHA_ADDR-1;
	szBuff[2]=0x03;
	szBuff[3]=0x1b;//so luong byte
	szBuff[4]=0x16;//bat dau lenh NONCE
	szBuff[5]=0x00;// mode 0
	szBuff[6]=0x00; //00 00
	szBuff[7]=0x00;
	
	for(i=0; i<20; ++i) {
		szBuff[8+i]=szNumIn[i];
	}
	
	sha204c_calculate_crc(szBuff[3]-2, &szBuff[3], &szBuff[28]);
	
	KT_I2C_Start();
	for(i=1; i<30; ++i) {
		if(KT_I2C_Write(szBuff[i])) {
			LED_ON;
			while(1);
		}
	}
	
	KT_I2C_Stop();
	//doc ra 32 byte RanOut
	dwTimeOut=1;
	for(i=0; i<200; ++i) {
		LongDelay(10000);
		KT_I2C_Start();
		if(!KT_I2C_Write(0xC9)) {
			dwTimeOut=0;
			break;
		}
		KT_I2C_Stop();
	}
	
	if(dwTimeOut) {
		LED_ON;
		while(1);
	}
	
	//doc tiep 0x23 byte
	//byte cuoi cung Ack 1
	for(i=0; i<0x22; ++i) {
		szBuff[i]=KT_I2C_Read(0);
	}
	szBuff[0x22]=KT_I2C_Read(1);
	
	for(i=0; i<0x20; ++i) {
		szRanOut[i]=szBuff[1+i];;
	}
	
	KT_I2C_Stop();
	
	//gui lenh tinh MAC la SHA256
	KT_I2C_Start();
	szBuff[1]=0xc8;//SHA_ADDR-1;
	szBuff[2]=0x03;
	szBuff[3]=0x07;//so luong byte
	szBuff[4]=0x08;//bat dau lenh MAC
	szBuff[5]=0x01;// mode 1
	szBuff[6]=0x00; //SLOT LOW
	szBuff[7]=0x00; //SLOT HIGH
	sha204c_calculate_crc(szBuff[3]-2, &szBuff[3], &szBuff[8]);
	//Write(szBuff);
	for(i=1; i<10; ++i) {
		if(KT_I2C_Write(szBuff[i])) {
			LED_ON;
			while(1);
		}
	}
	KT_I2C_Stop();
	
	dwTimeOut=1;
	for(i=0; i<200; ++i) {
		LongDelay(10000);
		KT_I2C_Start();
		if(!KT_I2C_Write(0xC9)) {
			dwTimeOut=0;
			break;
		}
		KT_I2C_Stop();
	}
	
	if(dwTimeOut) {
		LED_ON;
		while(1);
	}
	
	for(i=0; i<0x22; ++i) {
		szBuff[i]=KT_I2C_Read(0);
	}
	szBuff[0x22]=KT_I2C_Read(1);
	
	for(i=0; i<0x20; ++i) {
		szMac[i]=szBuff[1+i];
	}
	
	KT_I2C_Stop();
	
	//bat dau tinh toan va so sanh xem mac co giong nhau hay khong
	for(i=0; i<32; ++i) {
		szInPut[i]=szRanOut[i];
	}
	
	for(i=0; i<20; ++i) {
		szInPut[32+i]=szNumIn[i];
	}
	szInPut[52]=0x16;
	szInPut[53]=0x00;
	szInPut[54]=0x00;
	
	sha204h_calculate_sha256(55, szInPut, szOutPut);
	memmove(szTempKey, szOutPut, 32);
	
	memmove(szInPut, szSlot0, 32);
	
	memmove(&szInPut[32], szTempKey, 32);
	szInPut[64]=0x08;
	szInPut[65]=0x01;
	szInPut[66]=0x00;//slot low
	szInPut[67]=0x00;//slot high
	for(i=0; i<11; ++i) {
		szInPut[68+i]=0;
	}
	szInPut[79]=0xee;
	szInPut[80]=0x00;
	szInPut[81]=0x00;
	szInPut[82]=0x00;
	szInPut[83]=0x00;
	szInPut[84]=0x01;
	szInPut[85]=0x23;
	szInPut[86]=0x00;
	szInPut[87]=0x00;
	sha204h_calculate_sha256(88, szInPut, szOutPut);
	
	if(memcmp(szOutPut, szMac, 32)) {
			while(1);
	} else {
		//OK check success
	}
	
    while (1)
    {
		LED_ON;
		LongDelay(10000000);
		LED_OFF;
		LongDelay(10000000);
    }
}




/******************* (C) COPYRIGHT 2014 GIGADEVICE *****END OF FILE****/    
