#include <stdio.h>

#define NBLED 2
#define NBBTN 1
char led[NBLED];
char btn[NBBTN];
 
int main()
{
   int i;
   int fd = open("/dev/ledbtn", O_RDWR);
   if (fd < 0) {
      fprintf(stderr, "Erreur d'ouverture du pilote LED et Boutons\n");
      exit(1);
   }
   for( i = 0; i < NBLED; i ++)
      led[i] = '0';
   do { 
      led[O] = (led[O] == '0') ? '1' : '0';
      write( fd, led, NBLED);
      sleep( 1);
      read( fd, btn, NBBTN);
   } while (btn[0] == '1');
   return 0;
}
