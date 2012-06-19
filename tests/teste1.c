/*
 * Copyright (C) 2012 Rui AraÃºjo, Ricardo Lopes and Pedro Silva
 *
 * This file is part of uKernel-SEMB (see https://github.com/ruiaraujo/uKernel-SEMB)
 *
 * uKernel-SEMB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * uKernel-SEMB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with uKernel-SEMB.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
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

#include "selected_test.h"
#if _TEST_1
#include <avr/io.h> 
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h> 
#include "bit_tools.h" 
#include "scheduler.h"
#include <stdlib.h>


uint16_t tfin=0xFF;

uint16_t TIM16_ReadTCNT1( void )
{
	uint8_t sreg;
	uint16_t value;

	/* Save Global Interrupt Flag */
	sreg = GET_INTERRUPTS;
	/* Disable interrupts */
	cli();
	/* Read TCNT1 into i */
	value = TCNT1;
	/* Restore Global Interrupt Flag */
	RESTORE_INTERRUPTS(sreg);
	return value;
}

void TIM16_WriteTCNT1( uint16_t value )
{
	uint8_t sreg;
	/* Save Global Interrupt Flag */
	sreg = GET_INTERRUPTS;;
	/* Disable interrupts */
	cli();
	/* Set TCNT1 to i */
	TCNT1 = value;
	/* Restore Global Interrupt Flag */
	RESTORE_INTERRUPTS(sreg);
}

/* C = [12 .. 20] ms 
 tfin holds the execution time of 2 * yield();
*/
void task1(void * init) {
	while (1){		
		TCNT1 = 0;
		yield();
		tfin = TCNT1 >> 1;
	}
}
/* C = [7 .. 7] ms */
void task2(void *  init) {
	while (1){
		yield();
	}
}

int main (void) {
	
	TCCR0|=(uint8_t)(1<<CS02)|(1<<CS00); // Prescaler = FCPU/1024
	TIMSK|=(uint8_t)(1<<TOIE0); //Enable Overflow Interrupt Enable
	TCNT0=(uint8_t)241; //Initialize Timer Counter 0
	DDRC|=(uint8_t)0x3F; //Port C[3,2,1,0] as output
	PORTB = (uint8_t)0x03; //activate pull-ups on PB0 e PB1
	PORTD = (uint8_t)0x04; //activate pull-ups on PD2-INT0
	GICR|=(uint8_t)(0x40); //Enable External Interrupt 0
    DDRD &= ~(1 << DDD5);     // Clear the PD5 pin	// PD5 is now an input
	PORTD |= (1 << PORTD5);   // turn On the Pull-up
	TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10);  // No prescaling
	cli();
	/* periodic task */
	
	add_task(&task1,NULL,NULL,0,0, 1,50);
	add_task(&task2,NULL,NULL,0,0, 1,50);
	rtos_init(50);
	return 0;
}

#endif