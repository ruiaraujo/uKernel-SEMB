#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "default_idle.h"

void __internal_idle_task(void * data ) {
	//set_sleep_mode(SLEEP_MODE_IDLE);
	//sleep_enable();
	while ( 1)
		sei();
	///sleep_cpu();
	//sleep_disable();
}