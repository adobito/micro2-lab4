#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/tm4c1230h6pm.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/cpu.h"


int count = 0x00; //keeps track of string pos
int diff = 0x01; //used to know if string pos has changed
char words[16][20];
int mod = 16;
int isPressed = 0;

void delay(int delay) {
	int i;
	for(i = 0; i < delay; i++) {}
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
void lcdCommandWrite(int value) {
	GPIO_PORTA_DATA_R &=0xEB;

	GPIO_PORTD_DEN_R = 0x0F;
	GPIO_PORTC_DEN_R = 0xF0;

	GPIO_PORTC_DATA_R = value & 0xF0;
	GPIO_PORTD_DATA_R = value & 0x0F;

	GPIO_PORTA_DEN_R = 0xD4;
	GPIO_PORTA_DATA_R |= 0x04;
	delay(10000);
	GPIO_PORTA_DATA_R &= 0xFB;
	GPIO_PORTA_DATA_R = 0x00;
	GPIO_PORTC_DATA_R = 0x00;
	GPIO_PORTD_DATA_R = 0x00;
}

void lcdCharacterWrite(char character) {
	GPIO_PORTA_DATA_R &= 0xFB;
	GPIO_PORTA_DATA_R |= 0x10;

	GPIO_PORTC_DATA_R = character & 0xF0;
	GPIO_PORTD_DATA_R = character & 0x0F;
	GPIO_PORTA_DATA_R |= 0x04;
	delay(10000);
	GPIO_PORTA_DATA_R &= 0xFB;

}
void lcdInit() {
	SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOC;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOD;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA;

	GPIO_PORTC_DIR_R = 0xF0;
	GPIO_PORTD_DIR_R = 0x0F;
	GPIO_PORTA_DIR_R = 0x14;

	GPIO_PORTC_DEN_R = 0xF0;
	GPIO_PORTD_DEN_R = 0x0F;
	GPIO_PORTA_DEN_R = 0xD4;

	GPIO_PORTC_DR8R_R = 0xF0;
	GPIO_PORTD_DR8R_R = 0x0F;
	GPIO_PORTA_DR8R_R = 0x14;


	lcdCommandWrite(0x38);
	delay(1000);
	lcdCommandWrite(0x0F);
	delay(1000);
	lcdCommandWrite(0x06);
	delay(1000);
	lcdCommandWrite(0x01);

}
void lcdStringWrite(char string[]) {
	int i;
	for(i = 0; string[i] != ';'; i++) {
		lcdCharacterWrite(string[i]);
	}
}
void clearAllGPIOAInterrupts() {
	HWREG(0x4000441C) = 0xFF;
}
void interruptGPIOAMaskDisabler() {
	HWREG(0x40004410) = 0xFF;
}
void interruptInGPIOA() {
	if(!isPressed) {
		if(GPIO_PORTA_DATA_R & 0x80) { //if up button is pressed
			isPressed = 1; //set to button pressed
			count = (count + mod + 1) % mod; //increase counter
		}
		else if(GPIO_PORTA_DATA_R & 0x40) { //if down button is pressed
			isPressed = 1; //set to button pressed
			count = (count + mod - 1) % mod; //decrease counter
		}
	}
	else if( (GPIO_PORTA_DATA_R & 0xC0) == 0 ) { //if no buttons are pressed
		delay(10000);
		isPressed = 0; //set pressed to false
	}

	if(count != diff) { //if position has changed from button press
		diff = count; //reset
		lcdCommandWrite(0x01); //send clear command
		lcdStringWrite(words[diff]); //write first string
		lcdCommandWrite(0xA8); //set cursor to second line
		lcdStringWrite(words[(diff + 1 ) % mod]); //write second string

		delay(300000);
	}
	clearAllGPIOAInterrupts();
}


int main(void) {
	init();
	lcdInit();
	strcpy(words[0],"Aloha!;");
	strcpy(words[1],"Hello!;");
	strcpy(words[2],"Hallo!;");
	strcpy(words[3],"Salut!;");
	strcpy(words[4],"Shalom!;");
	strcpy(words[5],"Ciao!;");
	strcpy(words[6],"Oi!;");
	strcpy(words[7],"Privet!;");
	strcpy(words[8],"Hola!;");
	strcpy(words[9],"Hej!;");
	strcpy(words[10],"Xin Chao!;");
	strcpy(words[11],"Ni Hao!;");
	strcpy(words[12],"Haai!;");
	strcpy(words[13],"Allo!;");
	strcpy(words[14],"Hai!;");
	strcpy(words[15],"Selam!;");

	IntMasterEnable();
	IntEnable(INT_GPIOA);
	interruptGPIOAMaskDisabler();
	HWREG(NVIC_SYS_CTRL_R) |= NVIC_SYS_CTRL_SLEEPDEEP;
	HWREG(NVIC_SYS_CTRL_R) |= NVIC_SYS_CTRL_SLEEPEXIT;

	CPUwfi();
	//SysCtlDeepSleep();

}
