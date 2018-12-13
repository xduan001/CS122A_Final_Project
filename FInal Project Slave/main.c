
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
#include "bit.h"

#include "usart_ATmega1284.h"


void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}

int numPhases = 0;
int phasecounter =0;
unsigned int pressure =0;
int open = 0;
unsigned char direction = 0;
enum motorStates{A, AB, B, BC, C, CD, D, DA} state;
enum ButtonStates{wait, read} btnState;

unsigned char temp;
unsigned char signal;
unsigned char mode;

enum BTS_State {BTS_INIT, BTS_REC, BTS_Funct} BTS_state;

	
void Motor_Tick()
{
	switch(state)
	{
		case A:
		if(!direction && numPhases > 0)
		{
			state++;
			numPhases--;
		}
		else if(direction && numPhases > 0)
		{
			state = DA;
			numPhases--;
		}
		else
		state = state;
		break;
		case AB:
		if(!direction && numPhases > 0)
		{
			state++;
			numPhases--;
		}
		else if(direction && numPhases > 0)
		{
			state--;
			numPhases--;
		}
		else
		state = state;
		break;
		case B:
		if(!direction && numPhases > 0)
		{
			state++;
			numPhases--;
		}
		else if(direction && numPhases > 0)
		{
			state--;
			numPhases--;
		}
		else
		state = state;
		break;
		case BC:
		if(!direction && numPhases > 0)
		{
			state++;
			numPhases--;
		}
		else if(direction && numPhases > 0)
		{
			state--;
			numPhases--;
		}
		else
		state = state;
		break;
		case C:
		if(!direction && numPhases > 0)
		{
			state++;
			numPhases--;
		}
		else if(direction && numPhases > 0)
		{
			state--;
			numPhases--;
		}
		else
		state = state;
		break;
		case CD:
		if(!direction && numPhases > 0)
		{
			state++;
			numPhases--;
		}
		else if(direction && numPhases > 0)
		{
			state--;
			numPhases--;
		}
		else
		state = state;
		break;
		case D:
		if(!direction && numPhases > 0)
		{
			state++;
			numPhases--;
		}
		else if(direction && numPhases > 0)
		{
			state--;
			numPhases--;
		}
		else
		state = state;
		break;
		case DA:
		if(!direction && numPhases > 0)
		{
			state=A;
			numPhases--;
		}
		else if(direction && numPhases > 0)
		{
			state--;
			numPhases--;
		}
		else
		state = state;
		break;
	}
	switch(state)
	{
		case A:
		PORTB = 0x01;
		//PORTC = 0x01;
		break;
		case AB:
		PORTB = 0x03;
		//PORTC = 0x03;
		break;
		case B:
		PORTB = 0x02;
		//PORTC = 0x02;
		break;
		case BC:
		PORTB = 0x06;
		//PORTC = 0x06;
		break;
		case C:
		PORTB = 0x04;
		//PORTC = 0x04;
		break;
		case CD:
		PORTB = 0x0C;
		//PORTC = 0x0C;
		break;
		case D:
		PORTB = 0x08;
		//PORTC = 0x08;
		break;
		case DA:
		PORTB = 0x09;
		//PORTC = 0x09;
		break;
		default:
		break;
	}
}
void Button_Tick()
{	
	pressure = ADC;
	switch(btnState)
	{
		case wait:
		if(mode == 1)
		{
			if(pressure > 200)//pressed
			{
				//PORTC= signal;
				PORTD = 0xFF;
				if(numPhases <= 0)
					numPhases = 500;
				direction = 0;//clockwise
				open = 1;
				btnState = read;
			}
			else if(pressure <200 && open == 1)
			{
				PORTD = 0;
				if(numPhases <= 0)
					numPhases = 500;
				direction = 1;
				open = 0;
				btnState = read;
			}
			else
				btnState=wait;
		}
		else
			btnState = wait;
		break;
		
		case read:
		if(numPhases > 0 && mode==1)
			btnState = read;
		else
			btnState = wait;
		
		default:
			btnState = wait;
		break;
	}
}

void BTS_Init()
{
	BTS_state = BTS_INIT;
	initUSART(0);
}

void BTS_Tick()
{
	switch(BTS_state)
	{
		case BTS_INIT:
		
		signal = 0x00;
		USART_Flush(0);
		
		break;
		
		default:

		break;
	}
	
	
	switch(BTS_state)
	{
		case BTS_INIT:
		
		BTS_state = BTS_REC;
		
		break;

		case BTS_REC:
		
		if (USART_HasReceived(0))
		{
			signal = USART_Receive(0);
			PORTC=signal;
			BTS_state = BTS_Funct;
		}
		else
		{
			PORTC=0;
		}
		
		break;

		case BTS_Funct:
		switch(signal)
		{
			case 0x01:
			mode=1;//auto
			//PORTC=0x01;
			BTS_state = BTS_REC;
			break;
			
			case 0x02:
			mode=2;// manual
			//PORTC=0x02;
			BTS_state = BTS_REC;
			break;
			
			case 0x03:
			mode=3;//sleep
			//PORTC=0x03;
			BTS_state = BTS_REC;
			break;
			
			default:
			mode=0x01;
			BTS_state = BTS_REC;
			break;
		}
		
		default:
		BTS_state=BTS_INIT;
		break;
	}
}

void Button_Task()
{
	btnState = wait;
	for(;;)
	{
		Button_Tick();
		vTaskDelay(20);
	}
}

void Motor_Task()
{
	state = A;
	for(;;)
	{
		Motor_Tick();
		vTaskDelay(3);
	}
}

void BTS_Task()
{
	BTS_Init();
	for(;;)
	{
		BTS_Tick();
		vTaskDelay(50);
	}
}

void StartShiftPulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(Motor_Task, (signed portCHAR *) "Motor_Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL);
	xTaskCreate(Button_Task, (signed portCHAR *) "Button_Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL);
	xTaskCreate(BTS_Task, (signed portCHAR *)"BTS_Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}

int main(void)
{
	//DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	ADC_init();
	pressure = ADC;
		
	
	StartShiftPulse(1);
	vTaskStartScheduler();
	
	

	
	while (1) {}
	
	return 0;
}