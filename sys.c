/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#define LECTURA 0
#define ESCRIPTURA 1
#define BUFFER_SIZE 512

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -38; /*ENOSYS*/
}

int sys_getpid()
{
	return current()->PID;
}

int sys_fork()
{
  int PID=-1;

  // creates the child process
  
  return PID;
}

void sys_exit()
{  
}


int sys_write(int fd, char * buffer, int size){
	char localbuffer [BUFFER_SIZE];
	int bytes_left;
	int ret;

	if ((ret = check_fd(fd, ESCRIPTURA)))
		return ret;
	if (size < 0)
		return -1;
	if (!access_ok(VERIFY_READ, buffer, size))
		return -1;
	
	bytes_left = size;
	while (bytes_left > BUFFER_SIZE) {
		copy_from_user(buffer, localbuffer, BUFFER_SIZE);
		ret = sys_write_console(localbuffer, BUFFER_SIZE);
		bytes_left-=ret;
		buffer+=ret;
	}
	if (bytes_left > 0) {
		copy_from_user(buffer, localbuffer,bytes_left);
		ret = sys_write_console(localbuffer, bytes_left);
		bytes_left-=ret;
	}
	return (size-bytes_left);
}
	
