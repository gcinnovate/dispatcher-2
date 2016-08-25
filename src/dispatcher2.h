/*
 * =====================================================================================
 *
 *       Filename:  dispatcher2.h
 *
 *    Description:
 *
 *        Version:  2.1
 *        Created:  06/03/2016 11:20:36
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Samuel Sekiwere (SS), sekiskylink@gmail.com
 *   Organization:
 *
 * =====================================================================================
 */

#ifndef DISPATCHER2_H
#define DISPATCHER2_H

#include "gwlib/gwlib.h"
#include <libpq-fe.h>

struct HTTPData {
    Octstr *url;
    HTTPClient *client;
    Octstr *body;
    List *cgivars;
    List *cgi_ctypes;
    Octstr *ip;
    List *reqh;
    PGconn *dbconn;
};

void free_HTTPData(struct HTTPData *x, int free_enclosed);
void usage(int exit_status);
int decode_switches(int argc, char *argv[]);

#define NELEMS(a) (sizeof (a))/(sizeof (a)[0])
#define TEST_URL "/test"
#define NUM_DBS 2

#endif
