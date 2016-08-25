/*
 * =====================================================================================
 *
 *       Filename:  log.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/05/2016 17:18:43
 *       Revision:  none
 *       Copyright: Copyright (c) 2016, GoodCitizen Co. Ltd.
 *
 *         Author:  Samuel Sekiwere (SS), sekiskylink@gmail.com
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include "log.h"
#include "conf.h"

int print_debug_messages = 1;

extern struct dispatcher2conf config;
#if 0
static char *logf = "dispatcher2.log";
void dispatcher2log(int prio, char *fmt, ...)
{
    char fname[512];
    FILE *f;

    if (!print_debug_messages && prio == LOG_DEBUG)
      return;

    sprintf(fname, "%s/%s", config.logdir, logf ? logf : "anon-log");
    f = fopen(fname, "a");
    if (f) {
      va_list ap;
      struct tm _tm, *tm  = &_tm;
      time_t t = time(NULL);
      _tm = gw_localtime(t);
      fprintf(f, "[%d] %02d/%02d %02d:%02d:%02d: ",
              getpid(),
              tm->tm_mday, tm->tm_mon+1,
              tm->tm_hour, tm->tm_min, tm->tm_sec);
      va_start(ap, fmt);
      vfprintf(f, fmt, ap);
      va_end(ap);
      fclose(f);
    }
}
#endif
