#ifndef __DISPATCHER2_CONF_H__INCLUDED__
#define __DISPATCHER2_CONF_H__INCLUDED__

#include "gwlib/gwlib.h"
#include "dispatcher2-config.h"

#define CONFFILE "/etc/dispatcher2.conf"

#define DEFAULT_NUM_THREADS 4
#define MAX_BATCH_RETRIES 10
struct dispatcher2conf {
    char dbhost[128];
    char dbuser[128];
    char dbpass[128];
    char dbname[128];

    char myhostname[128];
    int dbport;
    int  http_port;
    unsigned num_threads;

    int use_ssl;
    char logdir[128];
    int max_retries;
    double request_process_interval;
    int use_global_submission_period;
    int start_submission_period;
    int end_submission_period;
};

typedef struct dispatcher2conf *dispatcher2conf_t;

dispatcher2conf_t readconfig(char *conffile);
int parse_conf(FILE * f, dispatcher2conf_t config);

#endif
