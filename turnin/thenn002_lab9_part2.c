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
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

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

unsigned char toggle;

enum T_States{T_Start, T_Wait, T_Off, T_On} T_state;
int Toggle_Tick(int state) {
	switch(state) {
		case T_Start:
			state = T_Off;
			toggle = 0x00;
			break;
		case T_Off:
			if((~PINA & 0x01) == 0x01) {
				state = T_Wait;
				toggle = 0x01;
			}
			else { state = T_Off; }
			break;
		case T_On:
                        if((~PINA & 0x01) == 0x01) {
                                state = T_Wait;
                                toggle = 0x00;
                        }
			else { state = T_On; }
                        break;
		case T_Wait:
			if(((~PINA & 0x01) == 0x00) && (toggle == 0x01)) { state = T_On; }
			else if(((~PINA & 0x01) == 0x00) && (toggle == 0x00)) { state = T_Off; }
			else { state = T_Wait; }
			break;
		default:
			state = T_Start;
			break;
	}
	/*switch(state) {
                case T_Start:
                case T_Off:
			set_PWM(0);
                        break;
                case T_On:
			set_PWM(440);
                        break;
                case T_Wait:
                        if(toggle == 0x01) { set_PWM(440); }
                        else { set_PWM(0); }
                default:
                        break;
        }*/
	return state;
}

unsigned char cnt;
double notes[] = {261.63, 293.66, 329.63, 349.23, 392, 440, 493.88, 523.25};
double tone;

enum S_States{S_Start, S_Wait, S_Up, S_Down} S_state;
int Scale_Tick(int state) {
	switch(state) {
		case S_Start:
			state = S_Wait;
			cnt = 0;
			break;
		case S_Wait:
			if((~PINA & 0x06) == 0x02) {
				if(cnt > 0) { cnt--; }
				state = S_Down;
			}
			else if((~PINA & 0x06) == 0x04) {
				if(cnt < 7) { cnt++; }
                                state = S_Up;
			}
			else { state = S_Wait; }
			break;
		case S_Down:
			if((~PINA & 0x06) == 0x00) {
                                state = S_Wait;
                        }
                        else if((~PINA & 0x06) == 0x04) {
                                if(cnt < 7) { cnt++; }
                                state = S_Up;
                        }
                        else { state = S_Down; }
                        break;
		case S_Up:
			if((~PINA & 0x06) == 0x00) {
                                state = S_Wait;
                        }
                        else if((~PINA & 0x06) == 0x02) {
                                if(cnt > 0) { cnt--; }
                                state = S_Down;
                        }
                        else { state = S_Up; }
                        break;
		default:
			state = S_Start;
			break;
	}
	switch(state) {
                case S_Start:
                case S_Wait:
		case S_Down:
		case S_Up:
			if(toggle == 0x01) {
                        	tone = notes[cnt];
				set_PWM(tone);
			}
			else {
				set_PWM(0);
			}
                        break;
                default:
			set_PWM(0);
                        break;
        }
	return state;
}

int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	PWM_on();
	T_state = T_Start;
	S_state = S_Start;
    	while (1) {
		T_state = Toggle_Tick(T_state);
		S_state = Scale_Tick(S_state);
    	}
    	return 1;
}
