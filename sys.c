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

int sys_fork(){
  	int PID=-1;
//Pilla un nou PCB pel fill i el del pare
  	if (list_empty(&freequeue)) return -1; // No hi ha PCB lliures;
	struct task_struct *father = current();
  	struct task_struct *child = list_head_to_task_struct(&freequeue);
  	list_del(&(*child).list);
	union task_union *child_stack = (union task_union *) child;

  
  	return PID;
}

void sys_exit(){  
	int pag;
	struct task_struct * current_pcb = current();
	page_table_entry * pt_current = get_PT(current_pcb);

	for (pag=PAG_LOG_INIT_DATA_P0;pag<TOTAL_PAGES ;pag++){
		free_frame(pt_current[pag].bits.pbase_addr);
	}
	
	update_process_state_rr(current_pcb,&freequeue);
	sched_next_rr();
}

int sys_gettime(){
	return zeos_ticks;
}

int sys_getstats(int pid, struct stats *st){

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
	
