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
 
#ifndef SCHEDULER_PRIVATE_H_
#define SCHEDULER_PRIVATE_H_
#include "scheduler.h"
#include <stdint.h>
#include "rtos_config.h"

/*Utilities to start and stop tasks*/
void task_starter(void *) __attribute__ ((naked));
void task_stopper(void) __attribute__ ((naked));

// The all-important task switch function
void switch_task() __attribute__ ((naked));

//Used in the Timer0 overflow interrupt
uint8_t reduce_delays(void);

void add_task_to_priority_list(task_t * task, task_t ** first);

void yield_fast(void) __attribute__ ((naked));

#if !USE_DYNAMIC_MEMORY
uint8_t * get_stack(uint16_t lenght );
#endif
#endif