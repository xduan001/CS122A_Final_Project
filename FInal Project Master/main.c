#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/portpins.h>
#include <avr/pgmspace.h>

//FreeRTOS include files
#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"

#include "keypad.h"
#include "usart_ATmega1284.h"


void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}


unsigned char signal;
unsigned int T;

////blue tooth
enum BTMState {BTM_init, BTM_send, BTM_trans} BTM_state;

unsigned char f_send;

void BTM_Init(){
	//DDRA = 0xFF; PORTA = 0x00;

	
	initUSART(0);
	BTM_state = BTM_init;
}

void BTM_Tick(){
	static unsigned char temp;
	T = ADC;
	//Actions
	switch(BTM_state){
		case BTM_init:
		temp = 0;
		USART_Flush(0);
		
		break;
		
		default:
		break;
	}
	//Transitions
	switch(BTM_state){
		case BTM_init:
		BTM_state = BTM_send;
		break;
		case BTM_send:
		//PORTA = 0;
		if(T<=170)
		{
			
			f_send = 0;
			//temp = (~PIND);
			if (USART_IsSendReady(0))
			{
				USART_Send(signal,0);
				//PORTC= signal;
				BTM_state=BTM_trans;
			}
		}
		else
		{
			BTM_state = BTM_send;
			//PORTC=0;
		}
			
		break;
		
		case BTM_trans:
		if (USART_HasTransmitted(0))
		{
			//PORTA = 0x01;
			BTM_state=BTM_send;
		}
		break;
	}
}

void BTM_Task()
{
	BTM_Init();
	for(;;)
	{
		BTM_Tick();
		vTaskDelay(50);
	}
}

void BTMPulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(BTM_Task, (signed portCHAR *)"BTM_Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}

////keypad

enum keyState {key_init, key_send} key_state;

unsigned char mem1;
unsigned char mem2;
unsigned char keys;
unsigned char press;

void key_Init(){
	DDRB = 0xF0; PORTB = 0x0F;
	
	key_state = key_init;
}

void key_Tick(){
	static unsigned char botm;
	static unsigned char oldbotm;
	
	//Actions
	switch(key_state){
		case key_init:
		botm;
		oldbotm=0;
		press =1;
		mem1=0;
		mem2=0;
		keys=0;
		
		break;
		
		case key_send:
		keys=PINB;
		//botm = GetKeypadKey();
		botm = press;
		if (oldbotm != botm)
		{
			oldbotm = botm;
			if(!GetBit(PINB,0))//auto
			{
				press=0x01;
				botm=1;
				signal = 0x01;
				PORTC=0x01;
			}
			else if(!GetBit(PINB,1))//manual
			{
				press=0x02;
				botm=2;
				signal = 0x02;
				PORTC=0x02;
			}
			
			else if(!GetBit(PINB,2))//sleep
			{
				press=0x03;
				botm=3;
				signal = 0x03;
				PORTC=0x03;
			}
		}
		else{
			PORTC=0;
		}
		

		if(keys != 0xFF && press == 0x01)
		{
			if( mem1 == 0x00)
			{
				mem1= 0x01;
				signal = 0x01;
			}
			else if (mem1 == 0x01)
			{
				mem1= 0x00;
				signal = 0x00;
			}
		}
		
		if (keys != 0xFF && press == 0x02)
		{
			if(mem2 == 0x00)
			{
				mem2= 0x01;
				signal = 0x02;
			}
			else if (mem2 == 0x01)
			{
				mem2= 0x00;
				signal = 0x00;
			}
		}
		
		if (keys != 0xFF && press == 0x03)
		{
			signal=0x03;
		}
		
		break;
		
		default:
		break;
	}
	//Transitions
	switch(key_state){
		case key_init:
		key_state = key_send;
		break;
		
		default:
		key_state = key_init;
		break;
	}
}

void key_Task()
{
	key_Init();
	for(;;)
	{
		key_Tick();
		vTaskDelay(50);
	}
}

void keyPulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(key_Task, (signed portCHAR *)"key_Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}


int main(void)
{
	DDRC = 0xFF; PORTC = 0x00;
	
	ADC_init();
	//Start Tasks
	keyPulse(1);
	BTMPulse(2);
	//RunSchedular
	vTaskStartScheduler();
	
	return 0;
}