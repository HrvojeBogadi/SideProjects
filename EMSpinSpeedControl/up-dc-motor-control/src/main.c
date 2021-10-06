/* DC motor control based on STM32F103
 *
 * 
 * H. Bogadi, 2020.
 * D. Ivezic, 2020.
 *
 */


/* Includes */
#include <stddef.h>
#include <stdlib.h>
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "uart.h"
#include "stm32f103_misc.h"
#include "stm32f10x_util.h"
#include "stm32f10x_ina219.h"

#define NGAPS 41
#define Ts 0.01
#define NUMEL 4

/* global variables */
volatile char receivedChar;
volatile uint16_t currentPWM;
extern const uint16_t PWM_period;
volatile int noMs = 0;
float voltage, current;
char *buff;
volatile int flagMeasure = 0;

volatile uint16_t pulse_ticks = 0;
volatile unsigned long start_time = 0;
volatile unsigned long end_time = 0;

volatile uint32_t gapCounter = 0;//Number of gaps on EM
volatile float speed = 0;	//Revolutions per INTERVAL
volatile float speedPerSec = 0; //Revolutions per SECOND


/* Recursive equation variables */
volatile float xt = 0;
volatile float xt_1 = 0;
volatile float xt_2 = 0;
volatile float yt = 0;
volatile float yt_1 = 0;
volatile float yt_2 = 0;
volatile float PWMt = 0;
volatile float PWMt_1 = 0;
volatile float PWMt_2 = 0;

volatile float speedAvg[NUMEL];
volatile float avgSpeedPerTs;

/*GENERATE RANDOM SIGNAL*/
volatile uint16_t randPWM = 0;	//Determine value of current period of PRBS
volatile uint16_t randT = 0;	//Determine length of current period of PRBS
const int upperLimitRandT = 75;
const int lowerLimitRandT = 25;
volatile int timeCount = 0;			//Counts sample times for PRBS
volatile bool up = false;		//Check if PRBS should be upper or lower value

/* Regulator Variables */
volatile float r0, r1, r2;
volatile float s0, s1, s2;
volatile float t0, t1, t2;

void prbs(){

	//Set operating point at 7500PWM -- Set PRBS to +/- 2500PWM
	if(timeCount == randT){
		timeCount = 0;
		randT = (uint16_t)((rand() % (upperLimitRandT - lowerLimitRandT + 1)) + lowerLimitRandT);
		if(up){
			randPWM = 10000;
			up = false;
		}else{
			randPWM = 5000;
			up = true;
		}
		Set_PWM(randPWM);
	}
}

void step(){
	if(timeCount == 1){
		Set_PWM(5000);
	}
	if(timeCount >= 300){
		Set_PWM(9000);
	}
}

/* UART receive interrupt handler */
void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1,USART_IT_RXNE))
	{
		//echo character
		receivedChar = USART_GetChar();
		if(receivedChar == 'u')
		{
			currentPWM = Get_PWM();
			currentPWM += (uint16_t)500;
			if(currentPWM < PWM_period)
				Set_PWM(currentPWM);
		}
		else if(receivedChar == 'd')
		{

			currentPWM = Get_PWM();
			if(currentPWM > 7000u)
			{
				currentPWM -= (uint16_t)500;
				Set_PWM(currentPWM);
			}
		}
		else if(receivedChar == 's'){
			xt = 50;
		}
		else if(receivedChar == 'f'){
			xt = 80;
		}
		else if(receivedChar == 'j'){
			xt = 65;
		}

		USART_PutChar(receivedChar);
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}


/* TIM2 input capture interrupt */
void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2, TIM_IT_CC4))
	{
		end_time = TIM2->CCR4;
		pulse_ticks = end_time - start_time;
        start_time = end_time;
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC4);
		gapCounter++;
	}

}


/* TIMER4 every Ts second interrupt --> sending data to PC */
void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4, TIM_IT_Update))
	{
		USART_PutChar('p');
		USART_PutChar('m');

		//USART_SendUInt_32((uint32_t)(gapCounter));
		USART_SendUInt_32((uint32_t)(speedPerSec * 100.00));		//PRINT SPEED IN REVOLUTIONS PER SECOND
		USART_SendUInt_32((uint32_t)xt * 100.00);			//Multiply by 100 to show float values 
		//USART_SendUInt_32((uint32_t)randPWM);

		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);


		timeCount++;

		//Determine speed in Ts intervals as revolutions per interval
		//Speed is multiplied by 100 to get float values written out on serial plotter

		speed = ((float)gapCounter / (float)NGAPS);	//Number of revolutions per interval

		for(int i = 0; i < NUMEL - 1; i++){
			speedAvg[i] = speedAvg[i+1];
		}
		speedAvg[NUMEL - 1] = speed;

		avgSpeedPerTs = 0;

		for(int i = 0; i < NUMEL; i++){
			avgSpeedPerTs += speedAvg[i];
		}

		avgSpeedPerTs = avgSpeedPerTs / NUMEL;

		speedPerSec = (float)avgSpeedPerTs / (float)Ts;	//Number of revolutions per second
		yt = speedPerSec;
		gapCounter = 0;

		//step();	//helper function for determining Ts
		//prbs();	//helper function for process identification

		/**** IMPLEMENT REGULATOR ****/


		PWMt =  t2 * xt + t1 * xt_1 + t0 * xt_2 - (s2 * yt + s1 * yt_1 + s0 * yt_2 + r1 * PWMt_1 + r0 * PWMt_2);


		Set_PWM(PWMt);

		xt_2 = xt_1;
		xt_1 = xt;
		yt_2 = yt_1;
		yt_1 = yt;
		PWMt_2 = PWMt_1;
		PWMt_1 = PWMt;

	}
}


/* systick timer for periodic tasks */
void SysTick_Handler(void)
{
	noMs++;
}


/* main program - init peripherals and loop */
int main(void)
{
	NVIC_SetPriorityGrouping(0u);
	Systick_init();
	Output_setup();
	USART1_PC_Init();
	Timer_setup();
	Button_init();

	r0 = 0.3679;
	r1 = -1.3679;
	r2 = 0.2735;
	s0 = 76.5663;
	s1 = -251.1088;
	s2 = 189.6203;
	t0 = 14.6715;
	t1 = -59.0896;
	t2 = 59.4960;


	//randT = (uint16_t)((rand() % (upperLimitRandT - lowerLimitRandT + 1)) + lowerLimitRandT);

	while (1)
	{
		// read push button - stop motor
		if(!GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14))
		{
			Set_PWM(0u);
			Delay_ms(50);
		}
		// read push button - turn on motor
		if(!GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15))
		{
			Set_PWM(5000u);
			Delay_ms(50);
		}


	}
}

