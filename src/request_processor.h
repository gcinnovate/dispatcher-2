/*
 * =====================================================================================
 *
 *       Filename:  request_processor.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/20/2016 14:41:28
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Samuel Sekiwere (SS), sekiskylink@gmail.com
 *   Organization:
 *
 * =====================================================================================
 */
#ifndef __REQUEST_PROCESSOR_H
#define __REQUEST_PROCESSOR_H

#include "dispatcher2.h"
#include "misc.h"
#include "conf.h"

typedef struct serverconf_t {
    int server_id;
    Octstr *name;
    Octstr *username;
    Octstr *password;
    Octstr *ipaddress;
    Octstr *url;
    Octstr *auth_method;
    int use_ssl;
    Octstr *ssl_client_certkey_file;
    int start_submission_period;
    int end_submission_period;
} serverconf_t;

void start_request_processor(dispatcher2conf_t conf, List *server_req_list);
void stop_request_processor(void);
void free_serverconf(serverconf_t *d);
#endif
