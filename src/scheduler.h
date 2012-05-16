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
#include <stdint.h>
#include "rtos_config.h"

#include <avr/interrupt.h>
#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#define GET_INTERRUPTS (SREG & _BV(SREG_I))
#define RESTORE_INTERRUPTS(x) if(x) __asm__ volatile ("sei\n" ::);else __asm__ volatile ("cli\n" ::);

/*
 * It is important to keep these defines as they are because we take advantage of that
 * in the switch_task routine
 */
#define TASK_STARTING 0x01
#define TASK_RUNNING 0x02
#define TASK_READY 0x03
#define TASK_BLOCKED 0x10
#define TASK_DELAYED 0x20
#define TASK_STOPPED 0x30
#define NO_MEMORY 1


typedef struct task_t{
	/* period in ticks */
	uint8_t priority;

	/* ticks to execute*/
	uint16_t delay;

	/* function pointer */
	void (*func)(void *);

	/*  the init data passed to the task*/
	void * init_data;
	
	/* task state */
	uint8_t state;

	/* current stack pointer*/
	uint8_t * stack;

	/* Maximum top for the stack, will help to detect stack overflows */
	uint8_t * bottom_stack;

	/* Pointer to the next task*/
	struct task_t * next_task;
} task_t;

struct kernel{
	uint8_t * system_stack;
	task_t * current_task;
	task_t * first_task;
	uint8_t switch_active;
//	uint16_t millis_per_tick;
};

extern struct kernel kernel;

void increase_tick_counter(void);

uint16_t get_tick_counter(void);

int add_task(void (*f)(void *),void * init_data, uint16_t delay, uint8_t prority, uint16_t stack_len );

void yield(void) /*__attribute__ ((naked))*/;

/**
 * This function will allows a task to sleep for a number of ticks.
 */
void sleep_ticks(uint16_t ticks);

#define sleep_millis(millis) sleep_ticks(millis/*/kernel.millis_per_tick*/)

void rtos_init(void (*idle)(void *),uint16_t stack_len,uint16_t system);


/**
 * These are macros used by GCC when switching to ISR
 * Save the CPU context to the stack, and disable interrupts
 */
#define save_cpu_context() __asm__ volatile( \
		"push  r0\n in r0, 0x3f\n cli\n" \
		"push  r1\n push  r2\n push  r3\n push  r4\n push  r5\n push  r6\n push  r7\n" \
		"push  r8\n push  r9\n push r10\n push r11\n push r12\n push r13\n push r14\n push r15\n" \
		"push r16\n push r17\n push r18\n push r19\n push r20\n push r21\n push r22\n push r23\n" \
		"push r24\n push r25\n push r26\n push r27\n push r28\n push r29\n push r30\n push r31\n" \
		"push  r0\n" ::)

/**
 * Restore the CPU context from the stack, possibly re-enabling interrupts
 */
#define restore_cpu_context() __asm__ volatile ( \
		"pop r0\n" \
		"pop r31\n pop r30\n pop r29\n pop r28\n pop r27\n pop r26\n pop r25\n pop r24\n" \
		"pop r23\n pop r22\n pop r21\n pop r20\n pop r19\n pop r18\n pop r17\n pop r16\n" \
		"pop r15\n pop r14\n pop r13\n pop r12\n pop r11\n pop r10\n pop  r9\n pop  r8\n" \
		"pop  r7\n pop  r6\n pop  r5\n pop  r4\n pop  r3\n pop  r2\n pop  r1\n out 0x3f, r0\n pop  r0\n" ::)

/*
 * AVR GCC ISR macro would crash the system. Built a new one compatible 
 */
#undef ISR
#warning ISR redefined please check documentation
#define ISR(vector, function) void vector(void) __attribute__ ((signal,naked,__INTR_ATTRS));\
	void vector(void) {\
		save_cpu_context();\
		if ( !kernel.switch_active )\
		{	\
			*(((uint8_t*)SP)+1) |= _BV(SREG_I); \
			kernel.current_task->stack = (uint8_t*)SP; \
			SP = (uint16_t)kernel.system_stack;\
		}\
		function;\
		if ( !kernel.switch_active )\
		{\
			SP = (uint16_t)kernel.current_task->stack;\
			__asm__ volatile ("rjmp switch_task\n" ::);\
		}\
		restore_cpu_context();\
		__asm__ volatile ("reti\n" ::); \
	}

#endif
