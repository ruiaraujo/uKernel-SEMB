

#include <avr/io.h> 
#include <avr/interrupt.h>
#define F_CPU 14.7456E6 
#include <util/delay.h> 
#include "bit_tools.h" 
#include "printf_tools.h"
#include "scheduler.h"


/* assuming 16 bit ints... */
unsigned char rand() {
	static unsigned int seed;
	seed = 181 * seed + 359 ;
	return (seed >> 8);
}
volatile uint8_t count;
/* C = [12 .. 20] ms */
void task1(void) {
	bit_set(PORTC, 0);
	_delay_ms(20 - (rand()>>5));
	bit_clear(PORTC, 0);
}
/* C = [7 .. 7] ms */
void task2(void) {
	bit_set(PORTC, 1);
	_delay_ms(7);
	bit_clear(PORTC, 1);
}
/* C = [34 .. 50] ms */
void task3(void) {
	bit_set(PORTC, 2);
	_delay_ms(50 - (rand()>>4));
	bit_clear(PORTC, 2);
}
/* C = [36 .. 100] ms */
void task4(void) {
	bit_set(PORTC, 3);
	_delay_ms(100 - (rand()>>2));
	bit_clear(PORTC, 3);
}

ISR(TIMER0_OVF_vect) {
//This is the interrupt service routine for TIMER0 OVERFLOW Interrupt.
//CPU automatically call this when TIMER0 overflows.
//Increment our variable

Sched_Dispatch();

/*count++;
if(count==61) {
PORTC=~PORTC; //Invert the Value of PORTC
count=0;
}*/
}


int main (void) {
	
	TCCR0|=(1<<CS02)|(1<<CS00); // Prescaler = FCPU/1024
	TIMSK|=(1<<TOIE0); //Enable Overflow Interrupt Enable
	TCNT0=0; //Initialize Timer Counter
	count=0; //Initialize our variable
	DDRC|=0x0F; //Port C[3,2,1,0] as output
	PORTB = 0x03; //activate pull-ups on PB0 e PB1
	PORTD = 0x04; //activate pull-ups on PD2-INT0
	GICR|=(0x40); //Enable External Interrupt 0
	sei(); //Enable Global Interrupts
	
	Sched_Init();
	/* periodic task */
	Sched_AddT(&task1, 0, 4);
	/* one-shot task */
	Sched_AddT(&task2, 50, 10);
	Sched_AddT(&task3, 25, 5);
	Sched_AddT(&task4, 30, 15);
	while (1) {
		//Sched_Dispatch();
	}
}


