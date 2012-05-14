/*
 * Copyright (C) 2012 Rui Araújo, Ricardo Lopes and Pedro Silva
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
#include "scheduler.h"
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdlib.h>

static uint16_t tick_counter = 0;

void task_starter(void) __attribute__ ((naked));

// The all-important task switch function
void switch_task() __attribute__ ((naked));


void increase_tick_counter(void){
	sei();
	tick_counter++; //multi byte variable. disabling interrupts while accessing
	cli();
}

uint16_t get_tick_counter(void){
	return tick_counter;
}


task_t * current_task;
task_t * first_task;
task_t * idle_task;

void rtos_init(void (*idle)(void),uint16_t stack_len){
/* - Configura interrupção
* que periodicamente
* corre Sched_Schedule().
* P periodo da
* interrupção define a
* resolusão do relógio.
* (Hardware specific!)
*/
    set_sleep_mode(SLEEP_MODE_IDLE);
	add_task(idle,0,0,stack_len);
	__asm__ volatile ("rjmp switch_task\n" ::);
};

void task_starter(void){
	current_task->func();
}


int add_task(void (*f)(void),uint16_t delay, uint8_t priority, uint16_t stack_len ){
	task_t * new_task,* temp;
	new_task = ( task_t * ) malloc(sizeof(task_t));
	if ( new_task == NULL )
		return NO_MEMORY;
	new_task->bottom_stack = ( uint8_t * )malloc( sizeof(uint8_t)*stack_len );
	if (  new_task->stack == NULL)
	{
		free(new_task);
		return NO_MEMORY;
	}
	// This can be used to detect stack overflows.
	new_task->stack = new_task->bottom_stack + sizeof(uint8_t)*stack_len-1;

	// To jump start the task.
	// we use another function with no arguments.
 	*((new_task->stack)--) = ((uint16_t)task_starter) & 0xFF;
	*((new_task->stack)--) = (((uint16_t)task_starter) >> 8) & 0xFF;
	new_task->priority = priority;
	new_task->delay = delay;
	new_task->func = f;
	new_task->state = TASK_STARTING;
	if ( first_task == NULL )
	{
		first_task = new_task;
		return 0;
	}
	temp = first_task;
	while ( temp->next_task != NULL )
		temp = temp->next_task;
	temp->next_task = new_task;
	return 0;
}

void sleep(uint16_t ticks){
	current_task->delay = ticks;
	current_task->state = TASK_BLOCKED;
	yield();
}


void yield() { /*  __attribute__ ((naked)) */
	save_cpu_context();
	current_task->stack = (uint8_t*)SP;
	__asm__ volatile ("rjmp switch_task\n" ::);
}

void switch_task()  /*__attribute__ ((naked))*/{
	register uint8_t max_priority asm ("r18");
	register uint8_t state asm ("r19");
	register task_t * selected_task asm ("r20");
	register task_t * c_task asm ("r22");
	max_priority = 0;
	c_task = first_task;
	if ( current_task != NULL )
		current_task->state = TASK_READY;
	while ( c_task != NULL ){
		state = c_task->state & 0x03;
		if ( c_task->priority > max_priority && state != 0 )
		{
			selected_task = c_task;
			max_priority = c_task->priority;
		}
		c_task = c_task->next_task;
	}
	if ( selected_task == NULL )
		current_task = idle_task;
	else
		current_task = selected_task;
	SP = (uint16_t)current_task->stack;
	state = current_task->state;
	if ( state != TASK_STARTING )
		restore_cpu_context(); // The task has previously saved its context onto its stack
	selected_task->state = TASK_RUNNING;
	__asm__ volatile ("ret\n" ::);
}


