//********************************************************************
//*                    EEE2046F C example                            *
//*==================================================================*
//* WRITTEN BY:    	Lindelani Mbatha
//* DATE CREATED:      20/06/2019
//* MODIFIED:         20/06/2019                                               *
//*==================================================================*
//* PROGRAMMED IN: Eclipse Neon 3 Service Release 1 (4.4.1)          *
//* DEV. BOARD:    UCT STM32 Development Board                       *
//* TARGET:	   STMicroelectronics STM32F051C6                        *
//*==================================================================*
//* DESCRIPTION:  Multi-Purpose Dog like robot named stalker.                                                   *
//*                                                                  *
//********************************************************************
// INCLUDE FILES
//====================================================================
#include "lcd_stm32f0.h"
#include "stm32f0xx.h"
#include <stdint.h>
#include <stdio.h>
//====================================================================
// SYMBOLIC CONSTANTS
//====================================================================
#define TRUE 1
#define FALSE 0
//====================================================================
// GLOBAL VARIABLES
//====================================================================
typedef uint8_t flag_t; // New variable type (8 bit unsigned integer)
flag_t startFlag = FALSE;
flag_t upFlag = FALSE;
flag_t downFlag = FALSE;
flag_t channelSelNo = FALSE;
uint8_t count = 0;
uint8_t readPot7 =0;
uint8_t readPot7_pre =0;
uint8_t readPot6 = 0;
uint8_t readPot6_pre = 0;
char lineOne[30];
char lineTwo[30];
//====================================================================
// FUNCTION DECLARATIONS
//====================================================================
void init_GPIO(void);
void display(void);
void checkPB(void);
void TIM14init(void);
void initADC(void);
void wheels();
void TIM14_IRQHandler(void);
void pin4(void);
void pin7(void);
//====================================================================
// MAIN FUNCTION
//====================================================================
void main (void)
{
	init_GPIO();
	init_LCD();							// Initialise lcd
	display();
	TIM14init();
	initADC();
	for(;;)
	{
		pin4();
		pin7();
		wheels();
		//checkPB();
		for (volatile int i=0;i<=255;i++)
			for (volatile int j=0;j <=2244;j++);
	}									// Loop forever
}										// End of main

//====================================================================
// FUNCTION DEFINITIONS
//====================================================================
void pin7(void){
	ADC1->CHSELR |= ADC_CHSELR_CHSEL7;
	ADC1->CR |= ADC_CR_ADEN;//activate ADC to be ready
	while ((ADC1->ISR & ADC_ISR_ADRDY)==0);// wait for ADC to be ready

	ADC1->CR |= ADC_CR_ADSTART;// Start conversion

	//int checker = 0;
	while ((ADC1->ISR & ADC_ISR_EOC)==0);//wait for conversion to finish
	readPot7  = ADC1->DR;
	display();
	ADC1->CHSELR &= ~ADC_CHSELR_CHSEL7;
}
void pin4(){
	ADC1->CHSELR |= ADC_CHSELR_CHSEL4;
	ADC1->CR |= ADC_CR_ADEN;//activate ADC to be ready
	while ((ADC1->ISR & ADC_ISR_ADRDY)==0);// wait for ADC to be ready

	ADC1->CR |= ADC_CR_ADSTART;// Start conversion

	//int checker = 0;
	while ((ADC1->ISR & ADC_ISR_EOC)==0);//wait for conversion to finish
	readPot6  = ADC1->DR;
	display();
	ADC1->CHSELR &= ~ADC_CHSELR_CHSEL4;

}
void init_GPIO(void) //Initialize GPIO
{
	// initialize GPIOB
	RCC  ->AHBENR |= RCC_AHBENR_GPIOBEN;
	GPIOB->MODER  |= 0x00005555;
	GPIOB->ODR    |= 0b0000000000000000;
	//initialize GPIOA
	RCC  ->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER  &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | GPIO_MODER_MODER2);
	GPIOA->MODER  |= (GPIO_MODER_MODER7 |GPIO_MODER_MODER4);
	GPIOA->PUPDR  &= ~(GPIO_PUPDR_PUPDR0 | GPIO_PUPDR_PUPDR1 | GPIO_PUPDR_PUPDR2);
	GPIOA->PUPDR  |= (GPIO_PUPDR_PUPDR0_0 | GPIO_PUPDR_PUPDR1_0 | GPIO_PUPDR_PUPDR2_0);
}
//display
void display(void){
	lcd_command(CLEAR); // Clears the lcd screen
	lcd_command(CURSOR_HOME); // ensure the cursor is in line one

	if (startFlag == FALSE && upFlag == FALSE && downFlag == FALSE){
		sprintf(lineOne,"%d", readPot6);
		sprintf(lineTwo,"%d", readPot7);
		lcd_putstring(lineOne);
		lcd_command(LINE_TWO); // Moves the cursor to line two
		lcd_putstring(lineTwo);
		//GPIOB->ODR &= 0b1111111100000000;
	}

}
//initADC
void initADC(void){
	RCC->APB2ENR |= RCC_APB2ENR_ADCEN;
	ADC1->CR &= ~(ADC_CR_ADSTART);

	ADC1->CFGR1 |= ADC_CFGR1_RES_1; // CONFIGURE TO 8 BITS resolution
	ADC1->CFGR1 &= ~(ADC_CFGR1_RES_0);
	ADC1->CFGR1 &= ~(ADC_CFGR1_ALIGN);//right align
	ADC1->CFGR1 &= ~(ADC_CFGR1_CONT);//single shot mode

	//ADC1->CHSELR |= ADC_CHSELR_CHSEL6 | ADC_CHSELR_CHSEL7;//select channel 6 & 7 to sample data

	//ADC1->CR |= ADC_CR_ADEN;//activate ADC to be ready
	//while ((ADC1->ISR & ADC_ISR_ADRDY)==0);// wait for ADC to be ready

	//ADC1->CR |= ADC_CR_ADSTART;// Start conversion
	//while ((ADC1->ISR & ADC_ISR_EOC)==0);//wait for conversion to finish

}
void TIM14init(void){
	RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
	TIM14->PSC = 367;
	TIM14->CNT = 65396;
	TIM14->DIER |= TIM_DIER_UIE;
	TIM14->CR1 = TIM_CR1_CEN;
	NVIC_EnableIRQ(TIM14_IRQn);

}

void wheels(){
	//left is readPot7 and  right is readPot6
	//GPIO 0 Left GPIO 1 Right
	if (count == 0){
		readPot6_pre = readPot6;
		readPot7_pre = readPot7;
		count++;
	}
	else if(count == 1){

		int dif6 = readPot6 - readPot6_pre;
		int dif7 = readPot7 - readPot7_pre;
		int dif  = readPot7 - readPot6;
		if(readPot6 < 190 && readPot7 < 190){
			GPIOB -> ODR &= ~(GPIO_ODR_1 | GPIO_ODR_0);
		}
		else if(dif > 5){ //&& dif7 > -5){
			//turn right
			GPIOB -> ODR &= ~(GPIO_ODR_1 | GPIO_ODR_0);
			GPIOB -> ODR |= GPIO_ODR_1;
		}else if(dif < -5){  //&& dif7 < -5){
			//turn left
			GPIOB -> ODR &= ~(GPIO_ODR_1 | GPIO_ODR_0);
			GPIOB -> ODR |= GPIO_ODR_0;
		}
		else if (dif < 5){
			//mover Forward
			GPIOB -> ODR &= ~(GPIO_ODR_1 | GPIO_ODR_0);
			GPIOB -> ODR |= GPIO_ODR_0 | GPIO_ODR_1;
		}
		count = 0;
	}


}
void TIM14_IRQHandler(void){
	TIM14->SR &= ~TIM_SR_UIF;
}
//check pressed button
void checkPB(void){
	while ((GPIOA->IDR & 0x0001) == 0){
		startFlag = TRUE;
		upFlag = FALSE;
		downFlag = FALSE;
		count = 0;
	}
	while ((GPIOA->IDR & 0x0002) == 0){
		startFlag = TRUE;
		upFlag = TRUE;
		downFlag = FALSE;
	}
	while ((GPIOA->IDR & 0x0004) == 0){
		startFlag = TRUE;
		upFlag = FALSE;
		downFlag = TRUE;
	}
}
//********************************************************************
// END OF PROGRAM
//********************************************************************
