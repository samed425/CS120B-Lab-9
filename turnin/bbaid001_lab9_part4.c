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
enum SoundStates { wait, play, pause } SoundState;
enum FrequencyStates { wait_freq, raise, lower, release } FrequencyState;

unsigned char threeLEDs;
unsigned char blinkingLED;
unsigned char sound;
unsigned char temp;
unsigned char button0;
unsigned char button1;
unsigned char button2;
unsigned long frequency;

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
	    BlinkingState = on;
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

void SoundSM () {
    switch (SoundState) {
	case wait :
	    if (button2 == 0x04) {
		SoundState = play;
		break;
	    }
	    SoundState = wait;
	    break;
	    
	case play :
	    if (button2 == 0x04) {
		SoundState = pause;
		break;
	    }
	    SoundState = wait;
	    break;

	case pause :
	    if (button2 == 0x04) {
		SoundState = play;
	        break;
	    }
	    SoundState = wait;
	    break;

	default :
	    SoundState = wait;
	    break;
	    
    }

    switch (SoundState) {
	case wait :
	    sound = 0x00;
	    break;

	case play :
	    sound = 0x10;
	    break;   

	case pause :
	    sound = 0x00;
	    break;

	default :
	    break;
    }
}

void FrequencySM () {
    switch (FrequencyState) {
	case wait_freq :
	    if (button0 == 0x01) {
		FrequencyState = raise;
	    }

	    else if (button1 == 0x02) {
		FrequencyState = lower;
	    }
	    else {
		FrequencyState = wait_freq;
	    }
 	    break;

	case raise :
	    FrequencyState = release;
	    break;

	case lower :
	    FrequencyState = release;
	    break;

	case release :
	    if (button0 == 0x00 && button1 == 0x00) {
		FrequencyState = wait_freq;
	    	break;
	    }
	    FrequencyState = release;
	    break;
    }	    

    switch (FrequencyState) {
	case wait_freq :
	    break;
	
	case raise :
	    if (frequency > 1) {
	    	frequency = frequency - 1;
	    }
	    break;

	case lower :
	    frequency = frequency + 1;
	    break;

	case release :
	    break;
    }		
}

void CombineLEDsSM () {
    temp = threeLEDs | blinkingLED | sound;
}

int main(void) {
    DDRB = 0xFF; PORTB = 0x00;
    DDRA = 0x00; PORTA = 0xFF;
    const unsigned long synchPeriod = 1;
    unsigned long TLSM_et = 0;
    unsigned long BLSM_et = 0;
    unsigned long SSM_et = 0;
    frequency = 2;
    threeLEDs = 0x00;
    blinkingLED = 0x00;
    sound = 0x00;
    ThreeState = INIT;
    BlinkingState = on;
    SoundState = wait;
    TimerSet(1);
    TimerOn();   


    while (1) {
	button0 = ~PINA & 0x01;
	button1 = ~PINA & 0x02;
	button2 = ~PINA & 0x04;
	if (TLSM_et >= 300) {
	    ThreeLEDsSM();
	    TLSM_et = 0;
	}
	if (BLSM_et >= 1000) {
	    BlinkingLEDSM();
	    BLSM_et = 0;
	}
	if (SSM_et >= frequency) {
	    SoundSM();
	    SSM_et = 0;
        }
	FrequencySM();	
	CombineLEDsSM();
	PORTB = temp;
	while (!TimerFlag);
	TimerFlag = 0;
	TLSM_et += synchPeriod;
	BLSM_et += synchPeriod;
	SSM_et += synchPeriod; 
   }
    return 1;
}
