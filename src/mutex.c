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
 
 void mutexe_lock(mutexe* m){
	int state = m->state;
	 
	cli(); //disable interrupts
	while (state != UNLOCK)
		yield();
	m->state = LOCK;
	sei();//enable interrupts
}
int mutexe_try_lock(task_t* t, mutexe* m){
	/*
	if (m->owner == t){
			return -1;
		
	}
	else{
		mutexe
	}
	*/
}
void mutexe_unlock(mutexe* m){
	cli();//disable interrupts
	m->state = 	UNLOCK;
	sei();//enable interrupts
	
}