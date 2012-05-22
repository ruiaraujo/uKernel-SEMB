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
 #include <stdint.h>
 
void mutexe_init(mutexe* m){
	m->state = UNLOCK;
	m->owner = NULL;
}
 
 
 void mutexe_lock(mutexe* m){
	uint8_t state = m->state, interrupts;
	 
	interrupts = GET_INTERRUPTS; //disable interrupts
	cli();
	while (state != UNLOCK)
		yield();
	m->state = LOCK;
	//m->owner = 
	RESTORE_INTERRUPTS(interrupts);//enable interrupts
}
int mutexe_try_lock(mutexe* m){
	
	if (m->owner == NULL){
		mutexe_lock(m);	
		return 0;
		
	}
	return -1;
}
void mutexe_unlock(mutexe* m){
	uint8_t interrupts;
	
	interrupts = GET_INTERRUPTS;//disable interrupts
	cli();
	m->state = 	UNLOCK;
	m->owner = NULL;
	RESPORE_INTERRUPTS(interrupts);//enable interrupts
	
}