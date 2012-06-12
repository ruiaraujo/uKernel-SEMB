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


/**
 * We take advantage of the distribution of the state codes.
 */
#define TASK_CAN_RUN(x) (((x) & 0x0F)!=0)


/**
 * If we don't want to use dynamic memory we have a static array of tasks
 * and a static buffer where we will hold the number of stacks needed.
 */
#if !USE_DYNAMIC_MEMORY
	uint8_t __stack_available[TOTAL_BYTES_STACK];
	static uint16_t __current_position_stack = 0;

	uint8_t * get_stack(uint16_t lenght ){
		if ( __current_position_stack + lenght >= TOTAL_BYTES_STACK )
			return NULL;
		__current_position_stack += lenght;
		return &__stack_available[__current_position_stack-lenght];
	}

	task_t __all_taks[NUMBER_OF_TASKS];
	static uint16_t __current_position_tasks = 0;
#endif

struct kernel kernel = { .system_stack = NULL, .current_task = NULL, .blocked_tasks = NULL,
						.spleeping_tasks = NULL, .stopped_tasks = NULL, .ready_tasks = NULL,.switch_active = 0};

static uint16_t tick_counter = 0;

void task_starter(void *) __attribute__ ((naked));

void task_stopper(void) __attribute__ ((naked));

// The all-important task switch function
void switch_task() __attribute__ ((naked));
void reduce_delays(void);

void add_task_to_running(task_t * task, task_t * first){
	task_t * current_task = first;
	task_t * previous_task = first;
	task->state = TASK_READY;
	while ( current_task != NULL ){
		if ( current_task->priority < task->priority ){
			task->next_task = current_task;
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


void add_task_to_blocked(task_t * task, task_t * first){
	task_t * current_task = first;
	task_t * previous_task = first;
	while ( current_task != NULL ){
		if ( current_task->delay < task->delay ){
			task->next_task = current_task;
			previous_task->next_task = task;
			while ( current_task != NULL ){
				current_task->delay -= task->delay;
				current_task = current_task->next_task;
			}
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


void reduce_delays(void){
	task_t * task = kernel.spleeping_tasks;
	if ( task != NULL )
	{
		/* We only need to test TASK_DELAYED and TASK_BLOCKED */
			task->delay--;
			/* Moving all finished tasks to the ready queue*/
			while ( task->delay == 0 )
			{
				if ( task->state ==  TASK_DELAYED )
					task->state = TASK_STARTING;
				else
					task->state = TASK_READY;
				add_task_to_running(task,kernel.ready_tasks);
				task = task->next_task;
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
}


//This is the interrupt service routine for TIMER0 OVERFLOW Interrupt.
//CPU automatically call this when TIMER0 overflows.
ISR(TIMER0_OVF_vect, increase_tick_counter();reduce_delays(););

void increase_tick_counter(void){
	uint8_t interrupts = GET_INTERRUPTS;
	tick_counter++; //multi byte variable. disabling interrupts while accessing
	RESTORE_INTERRUPTS(interrupts);
}

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
	SP = (uint16_t)kernel.system_stack;
	sei();
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
	kernel.current_task->func = NULL;
	kernel.current_task->state = TASK_STOPPED;
	kernel.current_task->next_task = kernel.stopped_tasks;
	kernel.stopped_tasks = kernel.current_task;
	yield();
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
	task_t * new_task = NULL,* task = kernel.stopped_tasks;
	uint16_t min_stack_len = 0xFFFF;
	uint8_t interrupt;
	while ( task != NULL )
	{
		/* Finding an task already created but stopped and which stack we can use*/
		if ( task->stack_len >= stack_len && task->stack_len <= min_stack_len )
		{
			new_task = task;
			min_stack_len = task->stack_len; 
			break;
		}
		task = task->next_task;
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
	new_task->delay = delay;
	new_task->init_data = init_data;
	//placing arguments in the stack for further use
 	*((new_task->stack)--) = ((uint16_t)init_data) & 0xFF;
	*((new_task->stack)--) = (((uint16_t)init_data) >> 8) & 0xFF;
	new_task->func = f;
	new_task->finisher = finisher;
	interrupt = GET_INTERRUPTS;
	if ( delay == 0 ){
		new_task->state = TASK_STARTING;
		add_task_to_running(new_task, kernel.ready_tasks);
	}
	else{
		new_task->state = TASK_DELAYED;
		add_task_to_blocked(new_task, kernel.spleeping_tasks);
	}
	RESTORE_INTERRUPTS(interrupt);
	return 0;
}

void sleep_ticks(uint16_t ticks){
	uint8_t interrupts = GET_INTERRUPTS;
	kernel.current_task->delay = ticks;
	kernel.current_task->state = TASK_BLOCKED;
	add_task_to_blocked(kernel.current_task, kernel.spleeping_tasks);
	save_cpu_context();
	kernel.current_task->stack = (uint8_t*)SP;
	kernel.current_task = NULL;
	RESTORE_INTERRUPTS(interrupts);
	__asm__ volatile ("rjmp switch_task\n" ::);
}


void yield() { /*  __attribute__ ((naked)) */
	uint8_t interrupt = GET_INTERRUPTS;
	if ( kernel.current_task->period == 0 )
		add_task_to_running(kernel.current_task,kernel.ready_tasks);
	else {
		kernel.current_task->state = TASK_BLOCKED;
		kernel.current_task->delay = kernel.current_task->period;
		add_task_to_blocked(kernel.current_task, kernel.spleeping_tasks);
	}
	save_cpu_context();
	kernel.current_task->stack = (uint8_t*)SP;
	kernel.current_task = NULL;
	RESTORE_INTERRUPTS(interrupt);
	__asm__ volatile ("rjmp switch_task\n" ::);
}

void switch_task()  /*__attribute__ ((naked))*/{
	register uint8_t interrupt  asm ("r18");
	interrupt = GET_INTERRUPTS;
	kernel.switch_active = 1;
	SP = (uint16_t)kernel.system_stack;
	sei();
	__asm__ volatile ("nop\n nop\n" ::);
	cli();
	if ( kernel.current_task != NULL )
	{
		/* If current != NULL here means it was not moved */
		add_task_to_running(kernel.current_task,kernel.ready_tasks);
	}
	kernel.current_task = kernel.ready_tasks;
	kernel.ready_tasks = kernel.ready_tasks->next_task;
	SP = (uint16_t)kernel.current_task->stack;
	if ( kernel.current_task->state == TASK_STARTING )
	{
		kernel.current_task->state = TASK_RUNNING;
		__asm__ volatile ("pop r25\npop r24\n" ::);
	}
	else if ( kernel.current_task->state != TASK_STOPPING ) // when stopping we don't need to context
	{
		kernel.current_task->state = TASK_RUNNING;
		restore_cpu_context(); // The task has previously saved its context onto its stack
	}
	kernel.switch_active = 0;
	RESTORE_INTERRUPTS(interrupt);
	__asm__ volatile ("ret\n" ::);
}


