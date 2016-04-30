/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <interrupt.h>

#include <mm_address.h>

#include <sched.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1
#define BUFFER_SIZE 512

extern struct list_head freequeue;
extern struct list_head readyqueue;
extern int curr_quantum;

int check_fd(int fd, int permissions)
{
  	if (fd!=1) return -EBADF;
  	if (permissions!=ESCRIPTURA) return -EACCES;
  	return 0;
}

int sys_ni_syscall()
{
	return -ENOSYS;
}

int sys_getpid()
{
	return current()->PID;
}

int global_PID=1000;

int ret_from_fork()
{
  return 0;
}

int sys_fork(){
  	
 	struct list_head *lhcurrent = NULL;
	struct task_struct *father = current();
	struct task_struct *child;
  	union task_union *uchild;
	
 	/* Any free task_struct? */
	if (list_empty(&freequeue)) return -ENOMEM;

  	lhcurrent=list_first(&freequeue);
  
  	list_del(lhcurrent);
	
	child = list_head_to_task_struct(lhcurrent);	
  	
  	uchild=(union task_union*)child;
  
  	/* Copy the parent's task struct to child's */
  	copy_data(current(), uchild, sizeof(union task_union));
   
	allocate_DIR(child);

	//Buscar pagines per alocatar DATA+STACK
	int new_pag, pag, i;
	page_table_entry *child_PT = get_PT(child);
	for (pag = 0; pag < NUM_PAG_DATA; pag++){
		new_pag = alloc_frame();
		if (new_pag !=-1){
			set_ss_pag(child_PT,PAG_LOG_INIT_DATA+pag,new_pag);
		}
		else{
			for (i = 0; i < pag; i++){
				free_frame(get_frame(child_PT,PAG_LOG_INIT_DATA+i));
				del_ss_pag(child_PT,PAG_LOG_INIT_DATA+i);
			}
			list_add_tail(&(*child).list,&freequeue);
			return -EAGAIN;
		}
	}

	//Copiar codi de pare a fill
	page_table_entry *father_PT = get_PT(father);
	for (pag = 0; pag<NUM_PAG_KERNEL;pag++){
		set_ss_pag(child_PT,pag,get_frame(father_PT,pag));
	}
	for (pag = 0; pag<NUM_PAG_CODE;pag++){
		set_ss_pag(child_PT,PAG_LOG_INIT_CODE+pag,get_frame(father_PT,PAG_LOG_INIT_CODE+pag));
	}
	

	//Copiar DATA+STACK de pare a fill

	  for (pag=NUM_PAG_KERNEL+NUM_PAG_CODE; pag<NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA; pag++){
		set_ss_pag(father_PT,pag+NUM_PAG_DATA,get_frame(child_PT,pag));
		copy_data((void*)(pag<<12),(void*)((pag+NUM_PAG_DATA)<<12),PAGE_SIZE);
		del_ss_pag(father_PT,pag+NUM_PAG_DATA);
	}

	//Desactivar acces del pare a pagines del fill

	set_cr3(get_DIR(father));

	//Assignar un PID al process fill i guardarlo a PID
	
	(*child).PID=++global_PID;
  	(*child).state=ST_READY;

	//Preparar pila fill per fer el canvi de context

	
	int register_ebp;

 	__asm__ __volatile__ (
    		"movl %%ebp, %0\n\t"
      		: "=g" (register_ebp)
      		: );
  	register_ebp=(register_ebp - (int)current()) + (int)(uchild);

  	(*child).kernel_esp=register_ebp + sizeof(DWord);

	DWord temp_ebp=*(DWord*)register_ebp;

	(*child).kernel_esp-=sizeof(DWord);
 	 *(DWord*)((*child).kernel_esp)=(DWord)&ret_from_fork;
  	(*child).kernel_esp-=sizeof(DWord);
  	*(DWord*)((*child).kernel_esp)=temp_ebp;

	//Posa stats a 0
	init_stats(&(*child).stats);

	//Encuar proces fill a ReadyQueue
	(*child).state=ST_READY;
  	list_add_tail(&((*child).list), &readyqueue);
  
  	return (*child).PID;
}

void sys_exit(){  
	int pag;
	struct task_struct * current_pcb = current();
	page_table_entry * pt_current = get_PT(current_pcb);

	for (pag=0;pag<NUM_PAG_DATA ;pag++){
		free_frame(get_frame(pt_current,PAG_LOG_INIT_DATA+pag));
		del_ss_pag(pt_current,PAG_LOG_INIT_DATA+pag);
	}
	
	update_process_state_rr(current_pcb,&readyqueue);
	sched_next_rr();
}

int sys_gettime(){
	return zeos_ticks;
}

int sys_getstats(int pid, struct stats *st){
	int i;
  
  	if (!access_ok(VERIFY_WRITE, st, sizeof(struct stats))) return -EFAULT; 
  
  	if (pid<0) return -EINVAL;
  	for (i=0; i<NR_TASKS; i++)
  	{
    		if (task[i].task.PID==pid)
    	{
      	task[i].task.stats.remaining_ticks=curr_quantum;
      	copy_to_user(&(task[i].task.stats), st, sizeof(struct stats));
      return 0;
    }
  }
  return -ESRCH; /*ESRCH */
}

int sys_write(int fd, char * buffer, int size){
	char localbuffer [BUFFER_SIZE];
	int bytes_left;
	int ret;

	if ((ret = check_fd(fd, ESCRIPTURA)))
		return ret;
	if (size < 0)
		return -EINVAL;
	if (buffer == NULL)
		return -EFAULT;
	
	bytes_left = size;
	while (bytes_left > BUFFER_SIZE) {
		copy_from_user(buffer, localbuffer, BUFFER_SIZE);
		ret = sys_write_console(localbuffer, BUFFER_SIZE);
		bytes_left-=ret;
		buffer+=ret;
	}
	if (bytes_left > 0) {
		copy_from_user(buffer, localbuffer,bytes_left);
		sys_write_console(localbuffer, bytes_left);

	}
	return (size);
}
	
