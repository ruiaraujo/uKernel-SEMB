#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "default_idle.h"

void idle_task(void * data ) {
	while (1)
	{
		sleep_enable();
        sei();
        sleep_cpu();
	}
}