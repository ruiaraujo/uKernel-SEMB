

#include <avr/io.h> 
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h> 
#include "bit_tools.h" 
#include "scheduler.h"
#include <stdlib.h>


/* C = [12 .. 20] ms */
void task1(void * init) {
	while (1){
		bit_set(PORTC, 0);
		_delay_ms(100);
		bit_clear(PORTC, 0);
		_delay_ms(100);
	}
}
/* C = [7 .. 7] ms */
void task2(void *  init) {
	while (1){
		bit_set(PORTC, 1);
		_delay_ms(100);
		bit_clear(PORTC, 1);
		_delay_ms(100);
		sleep_ticks(10);
	}
}
/* C = [34 .. 50] ms */
void task3(void * init) {
	while (1){
		bit_set(PORTC, 2);
		_delay_ms(250/* - (rand()>>4)*/);
		bit_clear(PORTC, 2);
		_delay_ms(250);
		sleep_ticks(50);
	}
}

/* C = [36 .. 100] ms */
void task4(void * init) {
	while (1){
		bit_set(PORTC, 3);
		_delay_ms(300);
		bit_clear(PORTC, 3);
		_delay_ms(300);
		sleep_ticks(50);
	}
}


int main (void) {
	
	TCCR0|=(uint8_t)(1<<CS02)|(1<<CS00); // Prescaler = FCPU/1024
	TIMSK|=(uint8_t)(1<<TOIE0); //Enable Overflow Interrupt Enable
	TCNT0=(uint8_t)241; //Initialize Timer Counter
	DDRC|=(uint8_t)0x0F; //Port C[3,2,1,0] as output
	PORTB = (uint8_t)0x03; //activate pull-ups on PB0 e PB1
	PORTD = (uint8_t)0x04; //activate pull-ups on PD2-INT0
	GICR|=(uint8_t)(0x40); //Enable External Interrupt 0
    cli();
	/* periodic task */
	add_task(&task1,NULL, 0, 4,70);
	/* one-shot task */
	add_task(&task2,NULL, 50, 10,70);
	add_task(&task3,NULL, 25, 5,70);
	add_task(&task4,NULL, 30, 15,70);
	rtos_init(70);
	return 0;
}


