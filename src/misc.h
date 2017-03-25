/*
 * =====================================================================================
 *
 *       Filename:  misc.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/07/2016 16:55:48
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Samuel Sekiwere (SS), sekiskylink@gmail.com
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef __DISPATCHER2_MISC_INCLUDED__
#define __DISPATCHER2_MISC_INCLUDED__

#include <unistd.h>
#include "gwlib/gwlib.h"
#include "dispatcher2-config.h"

#include <libpq-fe.h>

typedef struct request_t {
    int64_t dbid;

    int source;
    int destination;

    Octstr *payload;
    Octstr *ctype;
    int year;
    Octstr *month;
    Octstr *week;
    int64_t msgid; /* Submission ID in source system*/
    Octstr *msisdn;
    Octstr *raw_msg;
    Octstr *facility;
    Octstr *district;
    Octstr *report_type;

} request_t;

int dispatcher2_init(char *dbuser, char *dbpass, char *dbname, char *host, int port);
char *strip_space(char x[]);

int auth_user(PGconn *c, char *user, char *pass);
int ba_auth_user(PGconn *c, List *rh); /* Basic Auth */
int64_t save_request(PGconn *c, request_t *req);
int get_server(PGconn *c, char *name);
int parse_cgivars(List *request_headers, Octstr *request_body,
        List **cgivars, List **cgivar_ctypes);
#endif
