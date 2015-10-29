#include	"unpipc.h"
#include "jwHash.h" //can change

#define	SERV_FIFO	"/tmp/fifo.serv"

/* 2015.10.29 
 * a simple protocol
 * hash table command in a line for simple
 * pid + cmd_str + key + value
 * pid_t str       str    int
 *
 *TODO
 * need a strong protocol btw client and server
 */
