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

void start_request_processor(dispatcher2conf_t conf, List *server_req_list);
void stop_request_processor(void);
#endif
