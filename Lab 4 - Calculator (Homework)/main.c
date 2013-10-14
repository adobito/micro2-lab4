#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/tm4c1230h6pm.h"
#include "driverlib/sysctl.h"
#include <math.h>
#include "driverlib/cpu.h"
//#include "inc/hw_ints.h"
#include "driverlib/timer.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

char values[17];
char buttons[4][3];
char portBStates[4] = { 1,2,4,8 };
char portBCurrState = 0;
char lastKeysPressed[3] = { 0 , 0 , 0 };
char currState = 0;
char lastPressedKey = 0;
char currSevenSeg = 0;
char keypadScanCurr = 0;
char output[2] = { 0 , 0 };

void delay(int delay) {
	SysCtlDelay(delay);
}

void FillValuesArray() {
	values[0] = 0xD7;
	values[1] = 0x84;
	values[2] = 0xE3;
	values[3] = 0xE6;
	values[4] = 0xB4;
	values[5] = 0x76;
	values[6] = 0x77;
	values[7] = 0xC4;
	values[8] = 0xF7;
	values[9] = 0xF4;
	values[0xA] = 0xF5;
	values[0xB] = 0xFF;
	values[0xC] = 0x53;
	values[0xD] = 0xDF;
	values[0xE] = 0x73;
	values[0xF] = 0x71;
	values[0x10] = 0x20;
}

void FillKeypadButtonsArray() {

	buttons[0][0] = 1;
	buttons[0][1] = 2;
	buttons[0][2] = 3;
	buttons[1][0] = 4;
	buttons[1][1] = 5;
	buttons[1][2] = 6;
	buttons[2][0] = 7;
	buttons[2][1] = 8;
	buttons[2][2] = 9;
	buttons[3][0] = 0xA;;
	buttons[3][1] = 0;
	buttons[3][2] = 0xB;
}
void init() {

	SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOA;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOC;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOD;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;

	GPIO_PORTA_DIR_R = 0x00;
	GPIO_PORTB_DIR_R = 0x00;
	GPIO_PORTC_DIR_R = 0x00;
	GPIO_PORTD_DIR_R = 0x00;
	GPIO_PORTE_DIR_R = 0x00;
	GPIO_PORTF_DIR_R = 0x00;

	GPIO_PORTA_DEN_R = 0x00;
	GPIO_PORTB_DEN_R = 0x00;
	GPIO_PORTC_DEN_R = 0x00;
	GPIO_PORTD_DEN_R = 0x00;
	GPIO_PORTE_DEN_R = 0x00;
	GPIO_PORTF_DEN_R = 0x00;

	GPIO_PORTA_DATA_R = 0x00;
	GPIO_PORTB_DATA_R = 0x00;
	GPIO_PORTC_DATA_R = 0x00;
	GPIO_PORTD_DATA_R = 0x00;
	GPIO_PORTE_DATA_R = 0x00;
	GPIO_PORTF_DATA_R = 0x00;

	SYSCTL_RCGC2_R = 0x00;
}

void SevenSegInit() {
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOC;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOD;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA;

	GPIO_PORTC_DIR_R |= 0xF0;
	GPIO_PORTC_DEN_R |= 0xF0;
	GPIO_PORTC_DATA_R &= 0x0F;

	GPIO_PORTD_DIR_R |= 0x0F;
	GPIO_PORTD_DEN_R |= 0x0F;
	GPIO_PORTD_DATA_R &= 0xF0;

	GPIO_PORTA_DIR_R |= 0x0C;
	GPIO_PORTA_DEN_R |= 0x0C;
	GPIO_PORTA_DATA_R |= 0x0C;
}
void KeypadInterruptEnable() {
	IntEnable(INT_GPIOE);
	HWREG(0x40024408) = 0x00;
	HWREG(0x4002440C) = 0x0E;
	HWREG(0x40024410) |= 0x0E;

}
void ExtraButtonsInterruptEnable() {
	IntEnable(INT_GPIOA);

	HWREG(0x40004408) = 0x00; 	//disable dual edge interrupts
	HWREG(0x4000440C) = 0x60; 	//enable rising edge interrupts
	HWREG(0x40004410) |= 0x60;	//mask disable
}

void KeypadInit() {
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA;

	GPIO_PORTB_DIR_R |= 0x0F;
	GPIO_PORTB_DEN_R |= 0x0F;
	GPIO_PORTB_DATA_R = 0x01;

	GPIO_PORTE_DIR_R &= ~(0x0E);
	GPIO_PORTE_DEN_R |= 0x0E;
	GPIO_PORTE_DATA_R &= ~(0x0E);

	GPIO_PORTA_DIR_R &= ~(0x60);
	GPIO_PORTA_DEN_R |= 0x60;
	GPIO_PORTA_DATA_R &= ~(0x60);

	FillKeypadButtonsArray();
	KeypadInterruptEnable();
	ExtraButtonsInterruptEnable();
}
void PrintNumber(char num) {
	GPIO_PORTC_DATA_R = ~(values[num] & 0xF0);
	GPIO_PORTD_DATA_R = ~(values[num] & 0x0F);

}

void InitTimerModule()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet()/10);
	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	TimerEnable(TIMER0_BASE, TIMER_A);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
	TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
	TimerLoadSet(TIMER1_BASE, TIMER_A, SysCtlClockGet()/120);
	IntEnable(INT_TIMER1A);
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	TimerEnable(TIMER1_BASE, TIMER_A);
}

void TimerInterruptSevenSegmentDisplay() {
	TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	//PrintNumber(sevenSegVals[currSevenSeg]);
	PrintNumber(output[currSevenSeg]);
	GPIO_PORTA_DATA_R ^= 0x0C;
	currSevenSeg ^= 1;
}

int FindKeypadButtonPressed(char num) {
	switch (GPIO_PORTE_DATA_R & 0x0E) {
	case 0x02:
		return buttons[num][0];
	case 0x04:
		return buttons[num][1];
	case 0x08:
		return buttons[num][2];
	default:
		return -1;
	}
}

void TimerInterruptKeypadScan() {
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	portBCurrState++;
	portBCurrState %= 4;
	GPIO_PORTB_DATA_R = portBStates[portBCurrState];

}

void InterruptGPIOEPressed() {
	//portBCurrState += 3;
	//portBCurrState %= 4;
	//GPIO_PORTB_DATA_R = portBStates[portBCurrState];
	delay(1000);
	HWREG(0x4002441C) = 0xFF; //clear interrupt flag signal
	int keyPressed = -1;
	switch (GPIO_PORTB_DATA_R & 0x0F) {
	case 0x01:
		keyPressed = FindKeypadButtonPressed(0);
		break;
	case 0x02:
		keyPressed = FindKeypadButtonPressed(1);
		break;
	case 0x04:
		keyPressed = FindKeypadButtonPressed(2);
		break;
	case 0x08:
		keyPressed = FindKeypadButtonPressed(3);
		break;
	default:
		keyPressed = -1;
	}
	if((keyPressed + 1)) {
		if(currState < 3) {
			lastKeysPressed[currState] = keyPressed;
			currState++;
		}

	}

}
void InterruptExtraButtonPressed() {

	if(HWREG(0x40004414) & 0x40) {
		output[0] = 0;
		output[1] = 0;
		currState = 0;
	}
	else if(HWREG(0x40004414) & 0x20) {
		if(currState == 3) {
			if( lastKeysPressed[2] == 0x0A) {
				output[0] = (lastKeysPressed[0] + lastKeysPressed[1])/10;
				output[1] = (lastKeysPressed[0] + lastKeysPressed[1])%10;
			}
			else if( lastKeysPressed[2] == 0x0B) {
				if( lastKeysPressed[0] < lastKeysPressed[1]) {
					output[0] = 0x10;
					output[1] = lastKeysPressed[1] - lastKeysPressed[0];
				}
				else {
					output[0] = 0x00;
					output[1] = lastKeysPressed[0] - lastKeysPressed[1];
				}

			}
			currState = 0;
		}
	}
	HWREG(0x4000441C) = 0x60; //clear interrupt flag signal
}
int main(void) {

	init();
	SevenSegInit();
	KeypadInit();
	FillValuesArray();


	IntMasterEnable();
	//ExtraButtonsInterruptEnable();
	GPIO_PORTA_DATA_R = 0x04;
	lastKeysPressed[0] = 0;
	lastKeysPressed[1] = 0;
	currSevenSeg = 0;
	InitTimerModule();
	HWREG(NVIC_SYS_CTRL_R) |= NVIC_SYS_CTRL_SLEEPEXIT;
	while(1);




}


