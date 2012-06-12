/*
 * tempo de troca quando uma tarefa é bloqueada 
 * 
 * usando sleep_ticks(), 
 * que por sua vez invoca yield()
 * e que depois invoca switch_task());
 *
 * O primeiro tempo é guardado quando a tarefa de maior prioridade começa a executar (task2), 
 * bloqueando-se a si própria de seguida, dando lugar à tarefa de menor prioridade de executar,
 * que regista o segundo tempo, mal começa a executar.
 * O tempo de troca de tarefas usando este método é registado.
 */

#include <avr/io.h> 
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h> 
#include "bit_tools.h" 
#include "scheduler.h"
#include <stdlib.h>
#include <sys/time.h>


timeval time;
double tini=0.0, tfin2=0.0, dif=0.0;

/* C = [12 .. 20] ms */
void task1(void * init) {
	
	gettimeofday(&time, NULL);
	tfin = time.tv_sec*1000000.0 + (time.tv_usec);
	dif = tfin - tini;	
	
	while (1){
		bit_set(PORTC, 0);
		_delay_ms(100);
		bit_clear(PORTC, 0);
		_delay_ms(100);
	}
}
/* C = [7 .. 7] ms */
void task2(void *  init) {
	
	gettimeofday(&time, NULL);
	tini = (time.tv_sec*1000000.0 + time.tv_usec);
	sleep_ticks(10);
	
	while (1){
		bit_set(PORTC, 1);
		_delay_ms(100);
		bit_clear(PORTC, 1);
		_delay_ms(100);
		sleep_ticks(10);
	}
}

int main (void) {
	
	TCCR0|=(uint8_t)(1<<CS02)|(1<<CS00); // Prescaler = FCPU/1024
	TIMSK|=(uint8_t)(1<<TOIE0); //Enable Overflow Interrupt Enable
	TCNT0=(uint8_t)241; //Initialize Timer Counter
	DDRC|=(uint8_t)0x3F; //Port C[3,2,1,0] as output
	PORTB = (uint8_t)0x03; //activate pull-ups on PB0 e PB1
	PORTD = (uint8_t)0x04; //activate pull-ups on PD2-INT0
	GICR|=(uint8_t)(0x40); //Enable External Interrupt 0
    cli();
	/* periodic task */
	
	add_task(&task1,NULL,NULL, 0, 1,50);
	add_task(&task2,NULL,NULL, 0, 2,50);
	rtos_init(50);
	return 0;
}
