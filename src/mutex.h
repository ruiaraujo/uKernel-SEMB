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

#define LOCK 0x01
#define UNLOCK 0x00
#define NULL 0xFF

typedef struct mutexe{
	uint8_t state;
	task_t* owner; 
	
}mutexe;

void mutexe_init(mutexe* m);//inicialzar mutex
void mutexe_lock(mutexe* m);//adquirir mutex
int mutexe_try_lock(mutexe* m);//tentar adquirir mutex
void mutexe_unlock(mutexe* m);//libertar mutex

#endif