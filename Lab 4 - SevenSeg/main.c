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

char values[16];
char sevenSegVals[2] = { 0, 0 };
char currSevenSeg = 0;
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
	SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOC;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOD;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA;

	GPIO_PORTC_DIR_R = 0xF0;
	GPIO_PORTC_DEN_R = 0xF0;
	GPIO_PORTC_DATA_R = 0x00;

	GPIO_PORTD_DIR_R = 0x0F;
	GPIO_PORTD_DEN_R = 0x0F;
	GPIO_PORTD_DATA_R = 0x00;

	GPIO_PORTA_DIR_R = 0x0C;
	GPIO_PORTA_DEN_R = 0x0C;
	GPIO_PORTA_DATA_R = 0x0C;
}
void PrintNumber(char num) {
	GPIO_PORTC_DATA_R = ~(values[num] & 0xF0);
	GPIO_PORTD_DATA_R = ~(values[num] & 0x0F);

}

//Change to port and pin selection.
void IntMaskEnable()
{
	HWREG(0x40004410) = 0x03;
}


//Init the interrupt framework.
void InitInterruptModule()
{
	IntMasterEnable();
	IntEnable(INT_GPIOA);
	HWREG(0x40004408)= 0x00;
	HWREG(0x4000440C) = 0x00;
	IntMaskEnable();
}

void InitTimerModule()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet());
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

void TimerInterruptIncrease() {
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	sevenSegVals[1]++;
	sevenSegVals[0] += sevenSegVals[1]/16;
	sevenSegVals[0] %= 16;
	sevenSegVals[1] %= 16;
}
void TimerInterruptSevenSegmentDisplay() {
	TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	PrintNumber(sevenSegVals[currSevenSeg]);
	GPIO_PORTA_DATA_R ^= 0x0C;
	currSevenSeg ^= 1;
}
int main(void) {

	init();
	SevenSegInit();
	FillValuesArray();
	IntMasterEnable();

	GPIO_PORTA_DATA_R = 0x04;
	sevenSegVals[0] = 0;
	sevenSegVals[1] = 0;
	currSevenSeg = 0;
	InitTimerModule();
	HWREG(NVIC_SYS_CTRL_R) |= NVIC_SYS_CTRL_SLEEPEXIT;
}


