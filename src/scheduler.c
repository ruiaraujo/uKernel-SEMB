#include "scheduler.h"
#include <stdlib.h>
Sched_Task_t Task_List[20];
void Sched_Init(void){
	int x;
	for(x=0; x<20; x++)
		Task_List[x].func = 0;
/* - Configura interrupção
* que periodicamente
* corre Sched_Schedule().
* P periodo da
* interrupção define a
* resolução do relógio.
* (Hardware specific!)
*/
};


int Sched_AddT(void (*f)(void *),int d, int p){
	int x;
	for(x=0; Task_List[x].func; x++);
	
	if (x >= 20)
		return -1;
	Task_List[x].period = p;
	Task_List[x].delay = d;
	Task_List[x].exec = 0;
	Task_List[x].func = f;
	return x;
}


void Sched_Schedule(void){
	int x;
	for(x=0; x<20; x++) {
		if (!Task_List[x].func)
			continue;
		if ( Task_List[x].delay >= 0 ) {
			Task_List[x].delay--;
		} else {
			/* Schedule Task */
			Task_List[x].exec++;
			Task_List[x].delay = Task_List[x].period;
		}
	}
}

void Sched_Dispatch(void){
	int x;
	for(x=0; x<20; x++) {
		if ( Task_List[x].exec == 0 ) {
			Task_List[x].exec--;
			Task_List[x].func(NULL);
			/* Delete task
			* if one-shot
			*/
			if ( !Task_List[x].period )
				Task_List[x].func = 0;
		}
	}
}
