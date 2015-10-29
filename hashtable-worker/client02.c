#include	"fifo.h"

void
sig_chld(int signo)
{
	pid_t	pid;
	int		stat;

	while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
		printf("**child process %d terminated**\n", pid);
	return;
}

void exec_hashtable_cmd(char *cmd, char *key, int val){
	int		readfifo, writefifo;
	size_t	len;
	ssize_t	n;
	char	*ptr, fifoname[MAXLINE], buff[MAXLINE];
	pid_t	pid;

	/* create FIFO with our PID as part of name */
	pid = getpid();
	snprintf(fifoname, sizeof(fifoname), "/tmp/fifo.%ld", (long) pid);
	if ((mkfifo(fifoname, FILE_MODE) < 0) && (errno != EEXIST))
		err_sys("can't create %s", fifoname);

	// readline need a \n 
	snprintf(buff, sizeof(buff), "%ld,%s,%s,%d\n", (long) pid, cmd, key, val);
	len = strlen(buff);

	/* open FIFO to server and write hashtable command to FIFO */
	writefifo = Open(SERV_FIFO, O_WRONLY, 0);
	Write(writefifo, buff, len);

	/* now open our FIFO; blocks until server opens for writing */
	readfifo = Open(fifoname, O_RDONLY, 0);

	/* read from IPC, get hashtable op result */
	memset(buff, 0, sizeof(buff));
	n = Read(readfifo, buff, MAXLINE);
	printf("**reply from server : %s\n", buff);

	Close(readfifo);
	Unlink(fifoname);
}


int main(int argc, char *argv[]){
	int childpid;
	Signal(SIGCHLD, sig_chld);	/* must call waitpid() */
	if ( (childpid = Fork()) == 0) {	/* child process */
		exec_hashtable_cmd("add_int_by_str", "a", 1);
		exec_hashtable_cmd("add_int_by_str", "b", 2);
		exit(0);
	}

	sleep(1000);

	if ( (childpid = Fork()) == 0) {	/* child process */
		exec_hashtable_cmd("get_int_by_str", "c", -1);
		exec_hashtable_cmd("get_int_by_str", "b", -1);
		exit(0);
	}
	sleep(1000);
	return 0;
}
