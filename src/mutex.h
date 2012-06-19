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

#ifndef MUTEX_H_
#define MUTEX_H_


#define LOCKED 0
#define ERROR 1
#define NOT_LOCKED 2

#define MUTEX_DEFAULT_INIT { .owner = NULL, .blocked_tasks = NULL };

typedef struct mutex{
	struct task_t * owner;
	struct task_t * blocked_tasks;
} mutex;
	
uint8_t mutex_init(mutex* m);
uint8_t mutex_lock(mutex * m);
uint8_t mutex_try_lock(mutex * m);
uint8_t mutex_unlock(mutex * m);

#endif