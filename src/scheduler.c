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
#include "scheduler_private.h"
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdlib.h>

/**
 * If we don't want to use dynamic memory we have a static array of tasks
 * and a static buffer where we will hold the number of stacks needed.
 */
#if !USE_DYNAMIC_MEMORY
	task_t __all_taks[NUMBER_OF_TASKS];
	static uint16_t __current_position_tasks = 0;
	uint8_t __stack_available[TOTAL_BYTES_STACK];
	static uint16_t __current_position_stack = 0;

	uint8_t * get_stack(uint16_t lenght ){
		if ( __current_position_stack + lenght > TOTAL_BYTES_STACK )
			return NULL;
		__current_position_stack += lenght;
		return &__stack_available[__current_position_stack-lenght];
	}

#endif

struct kernel kernel = { .system_stack = NULL, .current_task = NULL,.spleeping_tasks = NULL, .stopped_tasks = NULL,
						.ready_tasks = NULL,.switch_active = 0};

static uint16_t tick_counter = 0;

void add_task_to_priority_list(task_t * task, task_t ** first){
	task_t * current_task = *first;
	task_t * previous_task = *first;
	if ( *first == NULL ) {
		*first = task;
		task->next_task = NULL;
		return;
	}
	while ( current_task != NULL ){
		if ( current_task->priority < task->priority ){
			task->next_task = current_task;
			/* The first position insertion is different*/
			if ( previous_task == current_task )
				*first = task;
			else
				previous_task->next_task = task;
			return;
		}
		previous_task = current_task;
		current_task = current_task->next_task;
	}
	//last one
	previous_task->next_task = task;
	task->next_task = NULL;
}


static void add_task_to_blocked(task_t * task, task_t ** first){
	task_t * current_task = *first;
	task_t * previous_task = *first;
	if ( *first == NULL ) {
		*first = task;
		task->next_task = NULL;
		return;
	}
	while ( current_task != NULL ){
		if ( task->delay < current_task->delay ){
			task->next_task = current_task;
			/* The first position insertion is different*/
			if ( previous_task == current_task )
				*first = task;
			else
				previous_task->next_task = task;
			current_task->delay -= task->delay;
			return;
		}
		previous_task = current_task;
		task->delay -= current_task->delay;
		current_task = current_task->next_task;
	}
	//last one
	previous_task->next_task = task;
	task->next_task = NULL;
}


uint8_t reduce_delays(void){
	task_t * task = kernel.spleeping_tasks;
	task_t * tmp = NULL;
	uint8_t need_switch = 0;
	tick_counter++;
	if ( task != NULL )
	{
		/* We only need to test TASK_DELAYED and TASK_BLOCKED */
			task->delay--;
			/* Moving all finished tasks to the ready queue*/
			while ( task->delay == 0 )
			{
				need_switch = 1;
				if ( task->state ==  TASK_DELAYED )
					task->state = TASK_STARTING;
				else
					task->state = TASK_READY;
				tmp = task->next_task;
				add_task_to_priority_list(task,&kernel.ready_tasks);
				task->ticks_after_activation = tick_counter;
				task = tmp;
				if ( task == NULL  )
					break;
			}
			kernel.spleeping_tasks = task;

	}	
	#if TEST_STACK_OVERFLOW
		if ( kernel.current_task->stack < kernel.current_task->bottom_stack )
		{
			cli();
			ACTION_IN_STACK_OVERFLOW;
			while (1);
		}
	#endif
	return need_switch;
}


//This is the interrupt service routine for TIMER0 OVERFLOW Interrupt.
//CPU automatically call this when TIMER0 overflows.
ISR(TIMER0_OVF_vect, reduce_delays());


uint16_t get_tick_counter(void){
	return tick_counter;
}


#undef rtos_init 
/* rtos_init can be a macro in case that USE_DEFAULT_IDLE == 1
 * so we undef it here.
 */
void rtos_init(void (*idle)(void*),uint16_t stack_len,uint16_t system ){
    set_sleep_mode(SLEEP_MODE_IDLE);
#if USE_DYNAMIC_MEMORY
	uint8_t interrupt = GET_INTERRUPTS;
	cli();
    kernel.system_stack = ( uint8_t * )malloc( sizeof(uint8_t)*system ) + system - 1;
	RESTORE_INTERRUPTS(interrupt);
#else
	kernel.system_stack = get_stack( sizeof(uint8_t)*system ) + system - 1;
#endif
	if ( kernel.system_stack == NULL  ){
		/**
		 * If we don't have enough stack for the system,
		 * we try to warn the user
		 */
		#if TEST_STACK_OVERFLOW
				cli();
				ACTION_IN_STACK_OVERFLOW;
				while (1);
		#endif
		return;
	}
	add_task(idle,NULL, NULL,0,0,0,stack_len);
	__asm__ volatile ("rjmp switch_task\n" ::);
};


/*Trick to start tasks.
*/
void task_starter(void * data)/*__attribute__ ((naked))*/{
	sei();
	do
	{
		kernel.current_task->func(data);
	} while (kernel.current_task->priority == 0); // Idle tasks cannot quit
	task_stopper();
}


void task_stopper(void)/*__attribute__ ((naked))*/{
	sei();
	kernel.current_task->state = TASK_STOPPING;
	if (  kernel.current_task->finisher != NULL) 
		kernel.current_task->finisher();
	#if USE_MUTEX
	while( kernel.current_task->holding_mutex != NULL )
		mutex_unlock(kernel.current_task->holding_mutex);
	#endif
	kernel.current_task->func = NULL;
	kernel.current_task->next_task = kernel.stopped_tasks;
	kernel.stopped_tasks = kernel.current_task;
	cli();
	kernel.current_task->state = TASK_STOPPED;
	kernel.current_task = NULL;
	__asm__ volatile ("rjmp switch_task\n" ::);
}

task_t * get_task(void (*f)(void*))
{
	task_t * task = kernel.ready_tasks;
	if (  kernel.current_task->func == f )
		return  kernel.current_task;
	while ( task != NULL )
	{
		if ( task->func == f )
		{
			return task;
		}
		task = task->next_task;
	}
	task = kernel.spleeping_tasks;
	while ( task != NULL )
	{
		if ( task->func == f )
		{
			return task;
		}
		task = task->next_task;
	}
	task = kernel.stopped_tasks;
	while ( task != NULL )
	{
		if ( task->func == f )
		{
			return task;
		}
		task = task->next_task;
	}
	return NULL;
}

int stop_task(task_t * task){
	if ( task == NULL )
		return NULL_TASK;
	if ( task == kernel.current_task )
		task_stopper(); // it won't return.
	if ( task->priority > kernel.current_task->priority || task->priority  == 0	)
		return NO_PERMISSIONS;
	task->state = TASK_STOPPING;
 	*((task->stack)--) = ((uint16_t)task_stopper) & 0xFF;
	*((task->stack)--) = (((uint16_t)task_stopper) >> 8) & 0xFF;
	return OK;
}

int add_task(void (*f)(void*),void (*finisher)(void), void * init_data,uint16_t period,uint16_t delay, uint8_t priority, uint16_t stack_len ){
	task_t * new_task = NULL,* current, *previous;
	uint16_t min_stack_len = 0xFFFF;
	uint8_t interrupt;
	current = previous = kernel.stopped_tasks;
	while ( current != NULL )
	{
		/* Finding an task already created but stopped and which stack we can use*/
		if ( current->stack_len >= stack_len && current->stack_len <= min_stack_len )
		{
			if ( previous == current )
				kernel.stopped_tasks = current->next_task;
			else
				previous->next_task = current->next_task;
			new_task = current;
			min_stack_len = current->stack_len; 
			break;
		}
		previous = current;
		current = current->next_task;
	}	
	if ( new_task == NULL )
	{
		interrupt = GET_INTERRUPTS;
		cli();
#if USE_DYNAMIC_MEMORY
		new_task = ( task_t * ) malloc(sizeof(task_t));
		if ( new_task == NULL )
		{
			RESTORE_INTERRUPTS(interrupt);
			return NO_MEMORY;
		}
		new_task->bottom_stack = ( uint8_t * )malloc( sizeof(uint8_t)*stack_len );
		if (  new_task->bottom_stack == NULL)
		{
			free(new_task);
			RESTORE_INTERRUPTS(interrupt);
			return NO_MEMORY;
		}
#else
		if ( __current_position_tasks >=  NUMBER_OF_TASKS )
		{
			RESTORE_INTERRUPTS(interrupt);
			return NO_MEMORY;
		}
		new_task = &__all_taks[__current_position_tasks++];
		new_task->bottom_stack = get_stack( sizeof(uint8_t)*stack_len );
		if (  new_task->bottom_stack == NULL)
		{
			RESTORE_INTERRUPTS(interrupt);
			return NO_MEMORY;
		}
#endif
		RESTORE_INTERRUPTS(interrupt);
		new_task->stack_len = stack_len;
	}

	// This can be used to detect stack overflows.
	new_task->stack = new_task->bottom_stack + sizeof(uint8_t)*new_task->stack_len-1;
	// To jump start the task.
	// we use another function with no arguments.
 	*((new_task->stack)--) = ((uint16_t)task_starter) & 0xFF;
	*((new_task->stack)--) = (((uint16_t)task_starter) >> 8) & 0xFF;
	new_task->priority = priority;
	new_task->period = period;
	new_task->delay = delay;
	new_task->init_data = init_data;
	#if USE_MUTEX
	new_task->holding_mutex = NULL;
	#endif
	//placing arguments in the stack for further use
 	*((new_task->stack)--) = ((uint16_t)init_data) & 0xFF;
	*((new_task->stack)--) = (((uint16_t)init_data) >> 8) & 0xFF;
	new_task->func = f;
	new_task->finisher = finisher;
	interrupt = GET_INTERRUPTS;
	if ( delay == 0 ){
		new_task->state = TASK_STARTING;
		add_task_to_priority_list(new_task, &kernel.ready_tasks);
	}
	else{
		new_task->state = TASK_DELAYED;
		add_task_to_blocked(new_task, &kernel.spleeping_tasks);
	}
	RESTORE_INTERRUPTS(interrupt);
	return 0;
}

void sleep_ticks(uint16_t ticks){
	kernel.current_task->delay = ticks;
	kernel.current_task->state = TASK_BLOCKED;
	save_cpu_context();
	kernel.current_task->stack = (uint8_t*)SP;
	add_task_to_blocked(kernel.current_task, &kernel.spleeping_tasks);
	kernel.current_task = NULL;
	__asm__ volatile ("rjmp switch_task\n" ::);
}

void yield_fast(){ /*  __attribute__ ((naked)) */
	save_cpu_context();
	kernel.current_task->stack = (uint8_t*)SP;
	kernel.current_task->state = TASK_READY;
	add_task_to_priority_list(kernel.current_task,&kernel.ready_tasks);
	kernel.current_task = NULL;
	__asm__ volatile ("rjmp switch_task\n" ::);
}

void yield() { /*  __attribute__ ((naked)) */
	uint16_t time;
	save_cpu_context();
	kernel.current_task->stack = (uint8_t*)SP;
	if ( kernel.current_task->period == 0 ){
		kernel.current_task->state = TASK_READY;
		add_task_to_priority_list(kernel.current_task,&kernel.ready_tasks);
	}
	else {
		kernel.current_task->state = TASK_BLOCKED;
		if ( tick_counter > kernel.current_task->ticks_after_activation )
			time = tick_counter - kernel.current_task->ticks_after_activation;
		else // tick counter has overflown, math is different
			time = ( 0xFFFF - kernel.current_task->ticks_after_activation )  + tick_counter;
		if ( time < kernel.current_task->period ){
			kernel.current_task->delay = kernel.current_task->period - time;
			add_task_to_blocked(kernel.current_task, &kernel.spleeping_tasks);
		}
		else{
			kernel.current_task->state = TASK_READY;
			add_task_to_priority_list(kernel.current_task,&kernel.ready_tasks);
		}
		kernel.current_task->ticks_after_activation = 0; // Cleaning this field to get it set up again 
	}
	kernel.current_task = NULL;
	__asm__ volatile ("rjmp switch_task\n" ::);
}

void switch_task()  /*__attribute__ ((naked))*/{
	kernel.switch_active = 1;
	SP = (uint16_t)kernel.system_stack;
	sei();
	__asm__ volatile ("nop\n nop\n" ::);
	cli();
	if ( kernel.current_task != NULL )
	{
		kernel.current_task->state = TASK_READY;
		/* If current != NULL here means it was not moved */
		add_task_to_priority_list(kernel.current_task,&kernel.ready_tasks);
	}
	kernel.current_task = kernel.ready_tasks;
	kernel.ready_tasks = kernel.ready_tasks->next_task;
	SP = (uint16_t)kernel.current_task->stack;
	
	
	/* Although this is only used on periodic tasks 
	   there is no harm setting it for non periodic and 
	   it will only happen once per non periodic task after tick_couuter != 0.
	*/
	if ( kernel.current_task->ticks_after_activation == 0 )
		kernel.current_task->ticks_after_activation = tick_counter;
	if ( kernel.current_task->state == TASK_STARTING )
	{
		kernel.current_task->state = TASK_RUNNING;
		__asm__ volatile ("pop r25\npop r24\n" ::);
	}
	else
	{
		kernel.current_task->state = TASK_RUNNING;
		restore_cpu_context(); // The task has previously saved its context onto its stack
	}
	kernel.switch_active = 0;
	__asm__ volatile ("ret\n" ::);
}


