/*
 * stopwatch.c
 *
 *  Created on: Sep 11, 2023
 *      Author: bassel
 */
#include<stdio.h>
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
//define the six pins of the multiplexed 7_segment
#define SEGMENT1 PA0
#define SEGMENT2 PA1
#define SEGMENT3 PA2
#define SEGMENT4 PA3
#define SEGMENT5 PA4
#define SEGMENT6 PA5
//prototypes of the used functions
void INT0_init(void); //Initialization of external interrupt 0
void INT1_init(void); //Initialization of external interrupt 1
void INT2_init(void); //Initialization of external interrupt 2
void TIMER1_init(void); //Initialization of Timer 1
void init_value(void); //Display zeroes on all the 7_segments
void start_stopwatch(void); //Start to display the time on the 7_segment
void last_time(void); //display the last number on another 7_segment when the user press reset button
//define global variables that used to display the numbers
unsigned char sec = 0, min = 0, hour = 0, unit = 0, ten = 0 ;
unsigned char last_sec = 0 , last_min = 0;
unsigned char flag = 0;
int main(void){
	SREG |= (1<<7); //Set pin 7 to 1 in register SREG to activate I-bit which activate the global interrupt
	INT0_init(); //call the function that activate external interrupt 0
	INT1_init(); //call the function that activate external interrupt 1
	INT2_init(); //call the function that activate external interrupt 2
	DDRB &= ~(1<<0); //activate Pin 0 in portB as an input pin
	DDRB |= 0xF0; //activate the last four pins in port B as output pins
	DDRC |= 0x0F; //activate the first four pins in port C as output pins
	DDRD |= 0xF0; //activate the last four pins in port D as output pins
	DDRA |= 0x3F; //activate the first six pins in port A as output pins
	PORTB &= 0x0F; //Turn off the last four pins in Port B
	PORTC &= 0xF0; //Turn off the first four pins in Port C
	PORTD &= 0x0F; //Turn off the last four pins in Port D
	PORTA &= 0xC0; //Turn off the first six pins in Port A
	while(1){
		//Call the function that display initial value of stop watch
		//The multiplexed seven segment should display zeroes till the user press start button
		init_value();
		//Call the function that display initial value of last lap segment
		//The multiplexed seven segment should display zeroes till the user press reset button
		last_time();
		// if condition to wait the user to press start button
		if(PINB & (1<<0)){
			//delay 30 milliseconds to avoid debouncing
			_delay_ms(30);
			if(PINB & (1<<0)){
				//After the user press the start button the timer will start to count 1 second
				TIMER1_init();
				//infinite loop to display the time on stop watch
				while(1){
					if(flag == 1){
						flag = 0;
						break;
					}
					//call the function that display the time on the stop watch multiplexed 7_segment
					start_stopwatch();
					//call the function that display the time of the last lap
					last_time();
				}
			}
		}
	}
}
//function to activate external interrupt 0
void INT0_init(void){
	DDRD &= ~(1<<2); //activate pin 2 in port D as input pin
	PORTD |= (1<<2); //activate the internal pull up in pin 2 in port D
	GICR |= (1<<INT0); //activate external interrupt 0
	MCUCR |= (1<<ISC01); //activate interrupt 0 to detect falling edge
}
//function to activate external interrupt 1
void INT1_init(void){
	DDRD &= ~(1<<3); //activate pin  in port D as input pin
	GICR |= (1<<INT1); //activate external interrupt 1
	MCUCR |= (1<<ISC10) | (1<<ISC11); //activate interrupt 1 to detect rising edge
}
//function to activate external interrupt 2
void INT2_init(void){
	DDRB &= ~(1<<2); //activate pin 2 in port B as input pin
	PORTB |= (1<<2); //activate the internal pull up in pin 2 in port B
	GICR |= (1<<INT2); //activate external interrupt 0
	MCUCSR &= ~(1<<ISC2); //activate interrupt 2 to detect falling edge
}
// function to activate Timer 1
void TIMER1_init(void){
	TCCR1A = (1<<FOC1A); //activate timer1A in timer 1
	TCCR1B = (1<<WGM12) | (1<<CS12); //activate timer1A in compare A with prescalar 256
	//the timer will count from zero to 3906
	//After the timer ends its count it will give an interrupt and call ISR
	TCNT1 = 0; //set the start value to zero
	OCR1A = 3906; //set the compared value to 3906
	TIMSK |= (1<<OCIE1A); //activate timer1A in compare mode
}
//function ISR of interrupt 0 that automatically called when the external interrupt 0 happens
ISR(INT0_vect){
	//when the reset button is pushed the value of last time 7_segment will change
	//Also the stop watch 7_segment will be set to zeroes till the user press start button again
	last_sec = sec; //the value of last value of seconds will be assigned to last_sec to be displayed on the another 7_segment
	last_min = min; //the value of last value of minutes will be assigned to last_min to be displayed on the another 7_segment
	if(flag == 0){
		flag = 1;
		sec = 0; //set the value of seconds to zero
		min = 0; //set the value of minutes to zero
		hour = 0; //set the value of hours to zero
		TCCR1B &= ~(1<<CS12); //stop the clock of timer1A to stop counting the time
		TCNT1 = 0;
	}
}
//function ISR of interrupt 1 that automatically called when the external interrupt 1 happens
ISR(INT1_vect){
	//when the pause button is pressed , the time should stop counting
	//the clock of the timer will be deactivated
	TCCR1B &= ~(1<<CS12);
}
//function ISR of interrupt 2 that automatically called when the external interrupt 2 happens
ISR(INT2_vect){
	//when the resume button is pressed , the time should resume counting
	//the clock of the timer will be activated again
	TCCR1B |= (1<<CS12);
}
//function ISR of timer1A that automatically called when the timer1A counts 1 second
ISR(TIMER1_COMPA_vect){
	sec++; //value of seconds will increment each second
	if(sec==60){
		//when the value of seconds reaches to 60 , it will be set to zero
		//the value of minutes will increment
		sec = 0;
		min++;
		if(min == 60){
			//when the value of minutes reaches to 60 , it will be set to zero
			//the value of hours will increment
			min = 0;
			hour ++;
			if(hour == 99){
				//when the hours reaches to 99 , it should not increment
				hour = 99;
			}
		}
	}
	//when the hours reaches to 99 and minutes reaches to 60 and seconds reaches to 60
	//the clock should stop counting
	if(hour == 99 && sec == 60 && min == 60){
		TCCR1B &= ~(1<<CS12);
	}
}

void init_value(void){
	PORTA &= 0xC0;
	PORTA |= (1<<SEGMENT1);
	PORTC = (PORTC & 0xF0) | (0 & 0x0F);
	_delay_ms(1);
	PORTA &= ~(1<<SEGMENT1);
	PORTA |= (1<<SEGMENT2);
	PORTC = (PORTC & 0xF0) | (0 & 0x0F);
	_delay_ms(1);
	PORTA &= ~(1<<SEGMENT2);
	PORTA |= (1<<SEGMENT3);
	PORTC = (PORTC & 0xF0) | (0 & 0x0F);
	_delay_ms(1);
	PORTA &= ~(1<<SEGMENT3);
	PORTA |= (1<<SEGMENT4);
	PORTC = (PORTC & 0xF0) | (0 & 0x0F);
	_delay_ms(1);
	PORTA &= ~(1<<SEGMENT4);
	PORTA |= (1<<SEGMENT5);
	PORTC = (PORTC & 0xF0) | (0 & 0x0F);
	_delay_ms(1);
	PORTA &= ~(1<<SEGMENT5);
	PORTA |= (1<<SEGMENT6);
	PORTC = (PORTC & 0xF0) | (0 & 0x0F);
	_delay_ms(1);
}
void start_stopwatch(void){
	unit = sec % 10;
	ten = sec / 10 ;
	PORTA &= 0xC0;
	PORTA |= (1<<SEGMENT1);
	PORTC = (PORTC & 0xF0) | (unit & 0x0F);
	_delay_ms(1);
	PORTA &= ~(1<<SEGMENT1);
	PORTA |= (1<<SEGMENT2);
	PORTC = (PORTC & 0xF0) | (ten & 0x0F);
	unit = min % 10;
	ten = min / 10 ;
	_delay_ms(1);
	PORTA &= ~(1<<SEGMENT2);
	PORTA |= (1<<SEGMENT3);
	PORTC = (PORTC & 0xF0) | (unit & 0x0F);
	_delay_ms(1);
	PORTA &= ~(1<<SEGMENT3);
	PORTA |= (1<<SEGMENT4);
	PORTC = (PORTC & 0xF0) | (ten & 0x0F);
	unit = hour % 10;
	ten = hour / 10 ;
	_delay_ms(1);
	PORTA &= ~(1<<SEGMENT4);
	PORTA |= (1<<SEGMENT5);
	PORTC = (PORTC & 0xF0) | (unit & 0x0F);
	_delay_ms(1);
	PORTA &= ~(1<<SEGMENT5);
	PORTA |= (1<<SEGMENT6);
	PORTC = (PORTC & 0xF0) | (ten & 0x0F);
	_delay_ms(1);


}
void last_time(void){
	unit = last_sec % 10;
	ten = last_sec / 10 ;
	PORTD &= 0x0F;
	PORTD |= (1<<4);
	PORTB = (PORTB & 0x0F) | (unit << 4);
	_delay_ms(1);
	PORTD &= 0x0F;
	PORTD |= (1<<5);
	PORTB = (PORTB & 0x0F) | (ten << 4);
	_delay_ms(1);
	unit = last_min % 10;
	ten = last_min / 10 ;
	PORTD &= 0x0F;
	PORTD |= (1<<6);
	PORTB = (PORTB & 0x0F) | (unit << 4);
	_delay_ms(1);
	PORTD &= 0x0F;
	PORTD |= (1<<7);
	PORTB = (PORTB & 0x0F) | (ten << 4);
	_delay_ms(1);
}
