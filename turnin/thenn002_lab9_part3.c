/*	Author: lab
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

void TimerOn() {
        TCCR1B = 0x0B;
        OCR1A = 125;
        TIMSK1 = 0x02;
        TCNT1 = 0;
        _avr_timer_cntcurr = _avr_timer_M;
        SREG |= 0x80;
}
void TimerOff() {
        TCCR1B = 0x00;
}
void TimerISR() {
        TimerFlag = 1;
}
ISR(TIMER1_COMPA_vect) {
        _avr_timer_cntcurr--;
        if(_avr_timer_cntcurr == 0) {
                TimerISR();
                _avr_timer_cntcurr = _avr_timer_M;
        }
}
void TimerSet(unsigned long M) {
        _avr_timer_M = M;
        _avr_timer_cntcurr = _avr_timer_M;
}
void set_PWM(double frequency) {
	static double current_frequency;
	if(frequency != current_frequency) {
		if(!frequency) { TCCR3B &= 0x08; }
		else { TCCR3B |= 0x03; }

		if(frequency < 0.954) { OCR3A = 0xFFFF; }
		else if (frequency > 31250) { OCR3A = 0x0000; }
		else { OCR3A = (short)(8000000 / (128*frequency))-1; }
		TCNT3 = 0;
		current_frequency = frequency;
	}
}
void PWM_on() {
	TCCR3A = (1 << COM3A0);
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	set_PWM(0);
}
void PMW_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

unsigned char cnt, i;
double notes[31] = {329.63, 0, 329.63, 0, 349.23, 0, 392.00, 0, 329.63, 0, 392.00, 0, 440.00, 0, 493.88, 0, 329.63, 0, 329.63, 0, 349.23, 0, 392.00, 0, 349.23, 0, 392.00, 0, 440.00, 0, 493.88};
unsigned char timing[31] = {2, 2, 2, 2, 2, 2, 2, 2, 4, 1, 1, 1, 1, 1, 8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 1, 1, 1, 1, 1, 8};
double tone;
unsigned char time;

enum States{Start, Stop, Go, Wait} state;
void Tick() {
	switch(state) {
		case Start:
			state = Stop;
			break;
		case Stop:
			if((~PINA & 0x01) == 0x01) { i=cnt=0; state = Go; }
			else { state = Stop; }
			break;
		case Go:
			if(cnt <= 30) { state = Go; }
			else { state = Wait; }
			break;
		case Wait:
			if((~PINA & 0x01) == 0x01) { state = Wait; }
			else { state = Stop; }
			break;
		default:
			state = Start;
			break;
	}
	switch(state) {
		case Start:
		case Wait:
		case Stop:
			set_PWM(0);
			break;
		case Go:
			i++;
			tone = notes[cnt];
			time = timing[cnt];
			if(i <= time) { set_PWM(tone);}
			else { cnt++; i=0; }
			break;
		default:
			break;
        }
}

int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	TimerSet(50);
	TimerOn();
	PWM_on();
	state = Start;
    	while (1) {
		Tick();
		while(!TimerFlag);
		TimerFlag = 0;
    	}
    	return 1;
}
