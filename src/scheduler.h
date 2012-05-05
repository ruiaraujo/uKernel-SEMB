


typedef struct {
/* period in ticks */
int period;
/* ticks to execute */
int delay;
/* function pointer */
void (*func)(void );
/* 'execute bit' */
int exec;
} Sched_Task_t;



void Sched_Dispatch(void);

void Sched_Schedule(void);

int Sched_AddT(void (*f)(void),int d, int p);

void Sched_Init(void);
