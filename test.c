/* MAIN.C file
 * 
 * Copyright (c) 2002-2005 STMicroelectronics
 */
#include "stm8l15x.h"
#include "stm8l15x_conf.h"
#include "rf4432.h"
#include <stdio.h>
#include "rtd_tab.h"
//适应不同编译器

#define PUTCHAR_PROTOTYPE char putchar (char c)
#define GETCHAR_PROTOTYPE char getchar (void)

void uart_config(void)
{
		USART_DeInit(USART1);
	
	 /* Enable USART clock */
  CLK_PeripheralClockConfig(CLK_Peripheral_USART1, ENABLE);
/////////////////PC5 ,PC6////////////////////////////
  /* Configure USART Tx as alternate function push-pull  (software pull up)*/
  GPIO_ExternalPullUpConfig(GPIOC, GPIO_Pin_5, ENABLE);
  /* Configure USART Rx as alternate function push-pull  (software pull up)*/
  GPIO_ExternalPullUpConfig(GPIOC, GPIO_Pin_6, ENABLE);

  /* USART configuration */
	USART_Init(USART1,
						(uint32_t)115200,
						USART_WordLength_8b,
						USART_StopBits_1,
						USART_Parity_No, 
						(USART_Mode_TypeDef)(USART_Mode_Tx | USART_Mode_Rx)
						);
}

void print_mv(uint32_t ad)
{
	ad%=1000000;
	putchar(ad/100000+0x30);
	ad%=100000;
	putchar(ad/10000+0x30);
	ad%=10000;
	putchar(ad/1000+0x30);
	ad%=1000;
	putchar(ad/100+0x30);
	ad%=100;
	putchar(ad/10+0x30);
	ad%=10;
	putchar(ad+0x30);
}


void RTD_ADC_init(void)
{
	CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, ENABLE);
	// 16Mhz
	ADC_Init(ADC1, ADC_ConversionMode_Single , ADC_Resolution_12Bit,ADC_Prescaler_1);
	//ADC使能
	ADC_Cmd(ADC1,ENABLE);
	//ADC_ChannelCmd(ADC1, ADC_Channel_17,ENABLE);
	ADC_SamplingTimeConfig(ADC1,ADC_Group_SlowChannels,ADC_SamplingTime_384Cycles);
}

main()
{
	double ad_result=0.0;
	uint8_t ans=0;
	uint8_t i=0;
	
	CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
	uart_config();
	spi_config();
	SI4432_init();
	GPIO_Init(GPIOB,GPIO_Pin_3,GPIO_Mode_In_PU_No_IT);
//	spi_config();
	//设置运放电源引脚上拉输出
	GPIO_ExternalPullUpConfig(GPIOD, GPIO_Pin_0, ENABLE);
//运放供电	
	GPIO_SetBits(GPIOD, GPIO_Pin_0);   
	RTD_ADC_init();
	rx_data();
	RF_CS_CLR;
		spi_byte(0x7f);
			
			for(i=0;i < 3;i++) 
			{ 
				ans = spi_byte(0x00);//read the FIFO Access register 
				//payload = spi_rw(ans,0x00);
				putchar(ans);
			} 
			RF_CS_SET;
	while (1)
	{
		/*
		tx_data();
		//printf("tx_data \r\n");
		for(i=0;i<50;i++)
		{
			ans=spi_rw(i,0x00);
			putchar(ans);		
		}
		*/
		
		ans = spi_rw(0x03,0x00);//read the Received Packet Length register 
		putchar(ans);
		ans = spi_rw(0x04,0x00);//read the Received Packet Length register 
		putchar(ans);
		//rx_data();
		//ans=GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3);
		if(ans&0x02 )
		{
			//disable the receiver chain 
			//spi_rw(0x07|0x80, 0x01);//write 0x01 to the Operating Function Control 1 register 
		//Read the length of the received payload 
			ans = spi_rw(0x4B,0x00);//read the Received Packet Length register 
			putchar(ans);
			RF_CS_CLR;
			spi_byte(0x7f);
			
			for(i=0;i < 10;i++) 
			{ 
				ans = spi_byte(0x00);//read the FIFO Access register 
				//payload = spi_rw(ans,0x00);
				putchar(ans);
			} 
			RF_CS_SET;
			rx_data();
		}
		
		/*
		ans= getchar();
		switch (ans)
		{
		case 'a':
		{
			ADC_ChannelCmd(ADC1, ADC_Channel_17,ENABLE);
			ADC_SoftwareStartConv(ADC1);
			printf("power senser : ");
			ad_result=(double)ADC_GetConversionValue(ADC1);
			ADC_ChannelCmd(ADC1, ADC_Channel_17,DISABLE);
			ad_result*=4.04443;
			print_mv(ad_result);
			printf("mV\r\n");
			break;
		}
		case 'b' :
		{
			ADC_ChannelCmd(ADC1, ADC_Channel_18,ENABLE);
			ADC_SoftwareStartConv(ADC1);
			printf("RTD senser : ");
			ad_result=(double)ADC_GetConversionValue(ADC1);
			ADC_ChannelCmd(ADC1, ADC_Channel_18,DISABLE);
			//电压
			//电流1.0mA
			//放大前电压
			ad_result*=110.0634;  //
			//ad_result*=12728;  //80566/633
			//ad_result/=1000; //100倍
			//ad_result=(uint32_t)(CalculateTemperature(ad_result)*100);
			print_mv(ad_result);
			printf("C\r\n");
			break;
		}
		default :
		{
			printf("input a or b \r\n");
			break;
		}
	}*/
	}
}
//重定向到标准输入输出函数
PUTCHAR_PROTOTYPE
{
  /* Write a character to the USART */
  USART_SendData8(USART1, c);
  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);

  return (c);
}
/**
  * @brief Retargets the C library scanf function to the USART.
  * @param[in] None
  * @retval char Character to Read
  * @par Required preconditions:
  * - None
  */
GETCHAR_PROTOTYPE
{
  int c = 0;
  /* Loop until the Read data register flag is SET */
  while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
    c = USART_ReceiveData8(USART1);
    return (c);
  }

#ifdef  USE_FULL_ASSERT
  /**
    * @brief  Reports the name of the source file and the source line number
    *   where the assert_param error has occurred.
    * @param  file: pointer to the source file name
    * @param  line: assert_param error line source number
    * @retval None
    */
  void assert_failed(uint8_t* file, uint32_t line)
  {
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {}
  }
#endif
