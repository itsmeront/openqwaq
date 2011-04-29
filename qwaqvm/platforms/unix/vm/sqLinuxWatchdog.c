/****************************************************************************
*   PROJECT: Linux watchdog loop for monitoring heartbeat on linux.
*   FILE:    sqUnixWatchdog.c
*   CONTENT: 
*
*   AUTHOR:  Eliot Miranda
*   ADDRESS: 
*   EMAIL:   eliot@teleplace.com
*   RCSID:   $Id$
*
*   NOTES: 
*  Sep  23rd, 2010, EEM extracted watchdog so it can be compiled -O0 (sigh).
*
*****************************************************************************/

/* The latest insult is that occasionally the interval timer stops ticking,
 * for reasons we don't yet understand.  So we use a watchdog thread to check
 * for clock progress (sure sign that the heartbeat is beating).  If we don't
 * see clock progress we write an error message and reestablish the itimer.
 *
 * This needs to be in a separate unit because the code I've written doesn't
 * work when optimized.
 */

#include "sq.h"
#include "sqAssert.h"
#include "sqMemoryFence.h"
#include <sys/param.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>

extern void reInitHeartbeatTimer(void);

/* The watchdog observes the clock every 100ms and if it finds the clock has not
 * advanced it prints a warning and resets the interval timer.  To avoid floods
 * of error messages it throttles the error messages to a maximum of 1 every 10
 * seconds.
 */
int defibrilate = 1; /* by default reset the heartbeat.  -nocpr turns it off */
static struct timeval tenSeconds = { 10, 0 };
void *
watchdogThreadLoop(void *careLess)
{
#define WatchdogPeriodUsecs (100 * 1000)
	char anon[MAXPATHLEN+1];
	int fd, pid = getpid();
	fd_set rd;
	unsigned long long lastgasp = 0ULL;
	int arrestsSoFar = 0, arrestsDueToItimer = 0, falseSelectsSoFar = 0;
	unsigned long elapsed = 0, requested = 0;
	struct timeval selectTimeout, nextErrorDeadline;

	selectTimeout.tv_sec  = 0;
	selectTimeout.tv_sec  = WatchdogPeriodUsecs;

	if (!tmpnam(anon)) {
		fprintf(stderr, "tmp file create and/or unlink for watchdog failed\n");
		exit(1);
	}
	if ((fd = open(anon,O_CREAT|O_RDWR|O_EXCL)) < 0) {
		perror("open");
		exit(1);
	}
	if (unlink(anon) < 0) {
		perror("open");
		exit(1);
	}
	FD_ZERO(&rd);
	while (1) {
		struct timeval tv, preselect;
		tv = selectTimeout;
		gettimeofday(&preselect,0); /* be paranoid about the select timeout */
		FD_SET(fd,&rd);
		if (select(1, &rd, 0, 0, &tv) == 0) {
			struct timeval tv, postselect, duration;
			unsigned long long freshair = ioUTCMicroseconds();
			gettimeofday(&postselect,0); /* check the select timeout */
			timersub(&postselect, &preselect, &duration);
			if (!timercmp(&duration, &selectTimeout, >=)) {
				falseSelectsSoFar++;
				elapsed += duration.tv_sec * 1000 + duration.tv_usec / 1000;
				requested += WatchdogPeriodUsecs / 1000;
			}
			else if (lastgasp == freshair) {
				struct itimerval itimer;
				arrestsSoFar++;
#define THE_ITIMER ITIMER_REAL
				if (getitimer(THE_ITIMER, &itimer))
					perror("disable_heartbeat_handler getitimer");
				if (itimer.it_interval.tv_sec == 0
				 && itimer.it_interval.tv_usec == 0)
					arrestsDueToItimer++;
				if (defibrilate)
					reInitHeartbeatTimer();
			}
			lastgasp = freshair;
			if (arrestsSoFar > 0 || falseSelectsSoFar > 0) {
				struct timeval now;
				gettimeofday(&now,0);
				/* N.B. contemporary defs of timercmp /do/ work for >= & <= */
				if (timercmp(&now, &nextErrorDeadline, >=)) {
					char o[100];
					int  l = sprintf(o,"WATCHDOG pid:%d HEARTBEAT",pid);
					if (arrestsSoFar == 1)
						l += sprintf(o+l, " STOPPED");
					else if (arrestsSoFar)
						l += sprintf(o+l, " STOPPED %d TIMES", arrestsSoFar);
					if (arrestsSoFar || arrestsDueToItimer)
						l += sprintf(o+l, " (%d due to itimer)", arrestsDueToItimer);
					if (defibrilate && arrestsSoFar)
						l += sprintf(o+l, "; RESET.");
					if (falseSelectsSoFar)
						l += sprintf(o+l, " (false selects %d; %ld%% loss)",
								falseSelectsSoFar,
								(long)(100.0 * (double)(requested - elapsed) / (double)requested));
					fprintf(stderr,"%s\n", o);
					arrestsSoFar = arrestsDueToItimer = falseSelectsSoFar = 0;
					requested = elapsed = 0;
					timeradd(&now, &tenSeconds, &nextErrorDeadline);
				}
			}
		}
	}
}
