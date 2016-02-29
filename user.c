#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */



	//TEST GETTIME

  //No se si el gettime funciona, sempre escriu 1, tot i que intentem perdre temps

  //PERDRE TEMPS
  /*int c,i;
  i = c = 0;
  while (i<9999999999) {
    c = 3*i;
    i++;
  }
  //AGAFAR TEMPS DE SISTEMA
  int time = gettime();
  char *buf = "fail";
  //PASARLO A ASCII
  itoa(time, buf);
  //ESCRIURE'L
  write(1,buf,strlen(buf));*/

	//Jocs de prova

  runjp();
  while(1);	
}
