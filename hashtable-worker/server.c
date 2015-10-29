#include	"fifo.h"

#define WORKERDEBUG     1


void	server(int, int);


#ifdef WORKERDEBUG
# define WORKER_DEBUG(fmt,args...) printf(fmt, ## args)
#else
# define WORKER_DEBUG(fmt,args...) do {} while (0);
#endif

static jwHashTable *table = NULL;

void hash_worker_init(int buckets){
	int		readfifo, writefifo, dummyfd, fd;
	char	*ptr, buff[MAXLINE], fifoname[MAXLINE];
	pid_t	pid;
	ssize_t	n;

	table = create_hash(buckets);
	if(!table) {
		// fail
		return ;
	}

	/* create server's well-known FIFO; OK if already exists */
	if ((mkfifo(SERV_FIFO, FILE_MODE) < 0) && (errno != EEXIST))
		err_sys("can't create %s", SERV_FIFO);

	/* open server's well-known FIFO for reading and writing */
	readfifo = Open(SERV_FIFO, O_RDONLY, 0);
	dummyfd = Open(SERV_FIFO, O_WRONLY, 0);		/* never used */

	while ( (n = Readline(readfifo, buff, MAXLINE)) > 0) {
		if (buff[n-1] == '\n')
			n--;			/* delete newline from readline() */
		buff[n] = '\0';		/* null terminate <pid + cmd_str + key + value> */

		printf("%s\n", buff);

		if ( (ptr = strchr(buff, ',')) == NULL) {
			err_msg("bogus request: %s", buff);
			continue;
		}

		*ptr++ = 0;			/* null terminate PID, ptr = <cmd_str + key + value> */
		pid = atol(buff);
		WORKER_DEBUG("**pid=%ld**\n", pid);
		snprintf(fifoname, sizeof(fifoname), "/tmp/fifo.%ld", (long) pid);
		if ( (writefifo = open(fifoname, O_WRONLY, 0)) < 0) {
			err_msg("cannot open: %s", fifoname);
			continue;
		}

		char *tmp = strchr(ptr, ',');
		*tmp++ = 0;
		char *cmd_str = ptr;
		WORKER_DEBUG("**command string=%s**\n", cmd_str);

		ptr = tmp;
		tmp = strchr(ptr, ',');
		*tmp++ = 0;
		char *key = ptr;
		WORKER_DEBUG("**key=%s**\n", key);

		char *val = tmp;
		WORKER_DEBUG("**value=%s**\n", val);

		// hash table op and send the reply to the client by writing to pipe
		char reply[MAXLINE];
		if(strcmp(cmd_str, "add_int_by_str") == 0){
			add_int_by_str(table, key, atoi(val));
			snprintf(reply, sizeof(reply), "add_int_by_str ok");

		}else if(strcmp(cmd_str, "get_int_by_str") == 0){
			int r = 0;
			get_int_by_str(table, key, &r);
			WORKER_DEBUG("get_int_by_str key=%s, val=%d\n", key, r);
			snprintf(reply, sizeof(reply), "get_int_by_str %d", r);

		}else{
			printf("**Unsupported hash table command**\n");
		}

		Write(writefifo, reply, strlen(reply));
		Close(writefifo);
	}
}

int
main(int argc, char **argv){
	hash_worker_init(10);
	return 0;
}
