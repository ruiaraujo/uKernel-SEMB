#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "default_idle.h"

void __internal_idle_task(void * data ) {
	sleep_enable();
	sei();
	sleep_cpu();
	sleep_disable();
}