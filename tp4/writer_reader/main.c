#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#define MAX_BUF 1024

char *reader(int fd)
{
	int		err;
	char	buf[MAX_BUF];

	do {
		if ((err = read(fd, buf, MAX_BUF)) == 0) break;
		
		printf("%s \n", buf);
        	fflush(stdout);

	}while (strcmp(buf,"end"));

	return buf;
}

int writer(int	fd,char *buf)
{
	int i,len;
	write(fd, buf, len);
	write(fd, "end\n", sizeof("end"));		

	return 0;
}


int main()
{
	char    *cToPython = "cToPython";
	char    *pythonToC = "pythonToC";
	int	fd1,fd2;
	char a;

	char	buf[MAX_BUF];
	
	/* create the FIFO (named pipe) */
	mkfifo(cToPython, 0777);
	mkfifo(pythonToC, 0777);

	printf("open \n");
	fd1 = open(cToPython, O_RDWR|O_NONBLOCK);
	fd2 = open(pythonToC, O_RDWR|O_NONBLOCK);

	dup2(STDIN_FILENO, fd1);
	dup2(fd2, STDOUT_FILENO);

	writer(fd1,"toto");

	scanf("%s",&a);

	reader(fd2);

	/* remove the FIFO */
	close(fd1);
	close(fd2);
	unlink(cToPython);
	unlink(pythonToC);

return 0;
}















