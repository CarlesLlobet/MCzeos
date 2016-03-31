/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>

/**
 * Container for the Task array and 2 additional pages (the first and the last one)
 * to protect against out of bound accesses.
 */
union task_union protected_tasks[NR_TASKS+2]
  __attribute__((__section__(".data.task")));

union task_union *task = &protected_tasks[1]; /* == union task_union task[NR_TASKS] */

struct list_head freequeue;
struct list_head readyqueue;
struct task_struct *idle_task;

struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return l&0xfffff000;
  //return list_entry( l, struct task_struct, list); Valia antiguamente pero ara ja no
}

extern struct list_head blocked;


/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");

	while(1)
	{
	;
	}
}

void init_idle (void)
{
	struct task_struct *idle = list_head_to_task_struct(&freequeue);
	list_del(&idle.list);
	idle.PID = 0;
	allocate_DIR(&idle);
	//Falta inicialitzar contexte d'execució
	union task_union *p;
	p = (union task_union*) idle;
	p -> stack[1023] = cpu_idle;
	p -> stack[1022] = 0;
	idle.kernel_esp = (unsigned long) &stack[1022];
	//L'assignem a l'idle global
	idle_task = p;
}

void init_task1(void)
{
}


void init_sched(){
//Inicialitzar freequeue buida
INIT_LIST_HEAD(&freequeue);
//Posar totes les task struct a freequeue ja que estan buides
int i = 0;
for (i = 0; i < NR_TASKS; i++){
	list_add( &(task[i].list), &freequeue );
}
//Inicialitzar readyqueue buida
INIT_LIST_HEAD(&readyqueue);
}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

