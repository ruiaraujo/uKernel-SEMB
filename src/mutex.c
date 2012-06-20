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
#include "mutex.h"
#include "scheduler_private.h"
#include <stdlib.h>

#if USE_MUTEX


void block_task(mutex *m){
	save_cpu_context();
	kernel.current_task->stack = (uint8_t*)SP;
	kernel.current_task->state = TASK_BLOCKED;
	add_task_to_priority_list(kernel.current_task,&m->blocked_tasks );
	kernel.current_task = NULL;
	__asm__ volatile ("rjmp switch_task\n" ::);
}

uint8_t mutex_init(mutex* m){
	if ( m == NULL )
		 return ERROR;
	m->owner = NULL;
	m->blocked_tasks = NULL;
	m->next = NULL;
	return OK;
}


uint8_t mutex_lock(mutex* m){
	if ( kernel.current_task == NULL || m == NULL )
		 return 1;
	if ( m->owner == kernel.current_task )
		return LOCKED;
	while ( m->owner != NULL )
		block_task(m);
	m->owner = kernel.current_task;
	kernel.current_task->holding_mutex = m;
	return LOCKED;
}

uint8_t mutex_try_lock(mutex* m){
	if ( kernel.current_task == NULL || m == NULL )
		 return 1;
	if ( m->owner == kernel.current_task )
		return LOCKED;
	if ( m->owner == NULL ){
		m->owner = kernel.current_task;
		m->next = kernel.current_task->holding_mutex;
		kernel.current_task->holding_mutex = m;
		return LOCKED;
	}
	return NOT_LOCKED;
}

uint8_t mutex_unlock(mutex* m){
	mutex* previous;
	task_t * task;
	uint8_t interrupts, need_to_switch =0;
	if ( m == NULL )
		 return ERROR;
	if (  m->owner != kernel.current_task )
		return NO_PERMISSIONS;
	m->owner = NULL;
	if ( kernel.current_task->holding_mutex == m )
	{
		kernel.current_task->holding_mutex = m->next;
	}
	else{
		previous = kernel.current_task->holding_mutex;
		while ( previous != NULL ){
			if ( previous->next == m  ){
				previous->next = m->next;
				break;
			}
		}
	}
	if ( m->blocked_tasks == NULL ){
		return NOT_LOCKED;
	}
	task = m->blocked_tasks->next_task;
	m->blocked_tasks->state = TASK_READY;
	/* if the blocked task had higher priority than ours yield after unlock*/
	if ( m->blocked_tasks->priority >= kernel.current_task->priority )
		need_to_switch = 1;
	interrupts = GET_INTERRUPTS;
	cli();
	add_task_to_priority_list(m->blocked_tasks,&kernel.ready_tasks );
	RESTORE_INTERRUPTS(interrupts);
	m->blocked_tasks = task;
	if ( need_to_switch ){
		yield_fast();
		if (  m->owner != NULL )
			return LOCKED;
	}	
	return NOT_LOCKED;
}

#endif