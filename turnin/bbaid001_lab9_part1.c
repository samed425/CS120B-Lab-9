/*	Author: bbaid001
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn(){
    TCCR1B = 0x0B;
    OCR1A = 125;
    TIMSK1 = 0x02;
    TCNT1 = 0;
    _avr_timer_cntcurr = _avr_timer_M;
    SREG |= 0x80;
}

void TimerOff(){
    TCCR1B = 0x00;
}

void TimerISR(){
    TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect){
    _avr_timer_cntcurr--;
    if(_avr_timer_cntcurr == 0) {
        TimerISR();
        _avr_timer_cntcurr = _avr_timer_M;
    }
}

void TimerSet(unsigned long M){
    _avr_timer_M = M;
    _avr_timer_cntcurr = _avr_timer_M;
}

enum ThreeStates { INIT, s0, s1, s2 } ThreeState;
enum BlinkingStates { on, off } BlinkingState;

unsigned char threeLEDs;
unsigned char blinkingLED;
unsigned char temp;

void ThreeLEDsSM() {
    switch (ThreeState) {
        case INIT :
	    ThreeState = s0;
	    break;

	case s0 :
	    ThreeState = s1;
	    break;

	case s1 :
	    ThreeState = s2;
	    break;
	
	case s2:
	    ThreeState = s0;
	    break;	
    }

    switch (ThreeState) {
	case INIT :
	    break;

	case s0 :
	    threeLEDs = 0x01;
	    break;

	case s1 :
	    threeLEDs = 0x02;
	    break;
	
	case s2 :
	    threeLEDs = 0x04;
	    break;

	default :
	    break;
    }
}

void BlinkingLEDSM () {
    switch (BlinkingState) {
	case on :
	    BlinkingState = off;
	    break;

	case off :
	    BlinkingState = on;
	    break;

	default :
	    break;
    }
    switch (BlinkingState) {
	case on :
	    blinkingLED = 0x08;
	    break;
	case off :
	    blinkingLED = 0x00;
	    break;
	default :
	    break;
    }
}

void CombineLEDsSM () {
    temp = threeLEDs | blinkingLED;
}

int main(void) {
    DDRB = 0xFF; PORTB = 0x00;
    threeLEDs = 0x00;
    blinkingLED = 0x00;
    ThreeState = INIT;
    BlinkingState = on;
    TimerSet(1000);
    TimerOn();   


    while (1) {
	ThreeLEDsSM();
	BlinkingLEDSM();
	CombineLEDsSM();
	PORTB = temp;
	while (!TimerFlag);
	TimerFlag = 0;
    }
    return 1;
}
