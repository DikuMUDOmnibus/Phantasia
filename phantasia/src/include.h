/*
 * include.h - includes all important files for Phantasia
 */

/* Logs the size of packets sent to the client */
/*
#define SEND_DEBUG
*/

/* Logs the size of packets received from the client */
/*
#define RECEIVE_DEBUG
*/

/* Logs the contents of packets to and from the clients (Verbose!) */
/*
#define SEND_PACKET_DEBUG
#define RECEIVE_PACKET_DEBUG
*/

/* Logs when a thread or server suspends */
/*
#define SUSPEND_DEBUG
*/

/* Lists what get's queued */
/*
#define SEND_QUEUE_DEBUG
*/

/* log mutext lock requests and unlocks */
/*
#define MUTEX_DEBUG
*/

    /* pthread.h must be first */
#include <pthread.h>			/* mutex_t */
#include <fcntl.h>			/* fcntl() */
#include <errno.h>
#include <math.h>			/* floor() */
#include <netinet/in.h> 		/* sockaddr_in */
#include <signal.h>			/* for signal(SIGINT, *) */
#include <stdio.h>
#include <stdlib.h>			/* rand() */
#include <sys/socket.h>			/* AF_INET, SOCK_STREAM, SOMAXCONN */
#include <netdb.h>			/* gethostbyaddr for DNS queries */
#include <sys/time.h>			/* timeval */

#include <sys/resource.h>		/* ulimit routines */
#include <unistd.h>

#include "global.h"
#include "macros.h"
#include "md5.h"
#include "phantdefs.h"
#include "phantstruct.h"
#include "pathnames.h"

