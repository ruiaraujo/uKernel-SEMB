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
#ifndef RTOS_CONFIG_H_
#define RTOS_CONFIG_H_


#define USE_MUTEX 1

#define USE_SEMAPHORES 1

/* With this active there is no need to define an idle task*/
#define USE_DEFAULT_IDLE 1

#define TEST_STACK_OVERFLOW 0
#define ACTION_IN_STACK_OVERFLOW PORTC = 0xFF;
#if TEST_STACK_OVERFLOW
	#ifndef ACTION_IN_STACK_OVERFLOW
		#error An action must be specified when detecting stack overflow
	#endif
#endif


#if USE_DEFAULT_IDLE
#include "default_idle.h"
#endif

#endif /* RTOS_CONFIG_H_ */
