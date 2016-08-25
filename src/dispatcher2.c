/*
 * =====================================================================================
 *
 *       Filename:  dispatcher2.c
 *
 *    Description:  Data Exchange Middleware - main dispatcher2 server
 *
 *        Version:  2.1
 *        Created:  06/03/2016 11:07:54
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Samuel Sekiwere (sekiskylink@gmail.com),
 *   Organization:  GoodCitizen Co. Ltd
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <getopt.h>
#include "dispatcher2.h"

#include "conf.h"
#include "log.h"
#include "request_processor.h"
#include "misc.h"

#define DISPATCHER2CONF "/etc/dispatcher2.conf"

static struct option long_options [] = {
    {"version", no_argument, 0, 'V'},
    /* {"debug", no_argument, 0, 'd'}, */
    {"help", no_argument, 0, 'h'},
    {"conf", required_argument, 0, 'c'},
    {NULL, 0, NULL, 0}

};

struct dispatcher2conf config; /*configuration stuff*/
char conffile[512];
static int conf_parsed = 0;

static char pidfile[128] = "/var/run/dispatcer2.pid";

void usage(int exit_status)
{
     fprintf(stdout, "%s [-c config] [-d] | -V | -h \n", PACKAGE);

     gwlib_shutdown();
     exit(exit_status);
}

int decode_switches(int argc, char *argv[])
{
    int c;
    int option_index = 0;
    FILE *f;

    while (1) {
        c = getopt_long(argc, argv, "hdVc:p:", long_options, &option_index);
        if (c == -1)
            break;
        switch (c) {
            case 'c':
                f = fopen(optarg, "r");
                if (f) {
                    strncpy(conffile, optarg, sizeof(conffile));
                    if (parse_conf(f, &config) < 0)
                        panic(0, "Initialisation failed! Failed to read config file/connect to db?");
                    conf_parsed = 1;
                    fclose(f);
                } else
                    usage(-1);
                break;
            case 'V':
               fprintf(stdout, "%s %s\n", PACKAGE, VERSION);
               exit(0);
            case 'p':
               strncpy(pidfile, optarg, sizeof pidfile);
               break;
            case 'h':
            default:
               usage(EXIT_FAILURE);
               break;
        }
    }
    return 0;
}

static int stop = 0;

/*URLs and their handlers*/
typedef const char *(*request_handler_t)(List *rh, struct HTTPData *x, Octstr *rbody, int *status);
static request_handler_t uri2handler(Octstr *);
static int supporteduri(Octstr *);
static void dispatch_processor(void *data);
static void dispatch_request(struct HTTPData *x);

static const char *queue_request(List *rh, struct HTTPData *x, Octstr *rbody, int *status)
{
    request_t *req;
    info(0, "We have called queue_request");

    Octstr *user = http_cgi_variable(x->cgivars, "username");
    Octstr *pass = http_cgi_variable(x->cgivars, "password");
    /*
    Octstr *ctype = http_cgi_variable(x->cgivars, "ctype");
    Octstr *payload = http_cgi_variable(x->cgivars, "payload");
    */
    Octstr *ct = NULL;
    Octstr *ctype = http_header_value(x->reqh, octstr_imm("Content-Type"));

    Octstr *source = http_cgi_variable(x->cgivars, "source");
    Octstr *dest = http_cgi_variable(x->cgivars, "destination");

    Octstr *msgid = http_cgi_variable(x->cgivars, "msgid"); /* id of submission in source */

    Octstr *week = http_cgi_variable(x->cgivars, "week");
    Octstr *month = http_cgi_variable(x->cgivars, "month");
    Octstr *year = http_cgi_variable(x->cgivars, "year");

    /*Use Basic Auth or GCI username and password to authenticate request*/
    if (ba_auth_user(x->dbconn, x->reqh) != 0  &&
            auth_user(x->dbconn, user ? octstr_get_cstr(user): "",
                pass ? octstr_get_cstr(pass): "") != 0) {
        *status = HTTP_UNAUTHORIZED;
        octstr_format_append(rbody, "error: ERR001: auth failed, user=%S", user);
        info(0, "Error: 0001 auth failed, user=%s", octstr_get_cstr(user));
        goto done;
    } else if (x->dbconn == NULL) {
        *status = HTTP_INTERNAL_SERVER_ERROR;
        octstr_append_cstr(rbody, "ERR002: Database not connected.");
        info(0, "Error: 0002");
        goto done;
    }

    if (ctype && octstr_case_search(ctype, octstr_imm("xml"), 0) >= 0) {
        ct = octstr_imm("xml");

    } else if (ctype && octstr_case_search(ctype, octstr_imm("json"), 0) >= 0) {
        ct = octstr_imm("json");
    }

    info(0, "Creating Request with ctype:%s", octstr_get_cstr(ctype));
    req = gw_malloc(sizeof *req);
    memset(req, 0, sizeof *req);

    req->source = get_server(x->dbconn, octstr_get_cstr(source));
    req->destination = get_server(x->dbconn, octstr_get_cstr(dest));
    req->month = octstr_duplicate(month);
    req->week = octstr_duplicate(week);
    req->msgid = (msgid) ? strtoull(octstr_get_cstr(msgid), NULL, 10) : -1;
    req->year = (year) ? strtoul(octstr_get_cstr(year), NULL, 10) : 0;
    req->payload = octstr_duplicate(x->body);
    req->ctype = octstr_duplicate(ct);

    if (save_request(x->dbconn, req) < 0) {
        *status = HTTP_INTERNAL_SERVER_ERROR;
        octstr_format_append(rbody, "error: E0003: Failed to save request in database");
        info(0, "Error: 0003");
    }
    *status = HTTP_ACCEPTED;
    octstr_destroy(ct);

done:
    http_header_add(rh, "Content-Type", "text/plain");
    octstr_format_append(rbody, "Request Queued");

    return "";
}

static struct {
    char *uri;
    request_handler_t func;

} uri_funcs[] = {
    {TEST_URL, NULL},
    {"/queue", queue_request}
};

static void quit_now(int unused)
{
     stop = 1;
     if (config.http_port > 0)
          http_close_port(config.http_port);
}

static void reopen_logs(int unused)
{
     warning(0, "SIGHUP received, catching and re-opening logs");
     log_reopen();
     alog_reopen();
}

static List *server_req_list;

int main(int argc, char *argv[])
{
    List *rh = NULL, *cgivars = NULL;
    Octstr *ip = NULL, *url = NULL, *body = NULL;
    int i;
    HTTPClient *client = NULL;

    gwlib_init();

    printf("Dispatcher2 v%s (Build %s).\n"
            "(c) 2016, GoodCitizen Ltd, All Rights Reserved.\n",
            VERSION, DISPATCHER2_BUILD_VERSION);

     strncpy(conffile, DISPATCHER2CONF, sizeof(conffile));
     decode_switches(argc, argv);

     if (!conf_parsed) {
          FILE *f;
          /* use the default or die */
          f = fopen(conffile, "r");
          if (f) {
               parse_conf(f, &config);
               conf_parsed = 1;
               fclose(f);
          } else
               usage(-1);
     }


    if (http_open_port(config.http_port, config.use_ssl) < 0)
        panic(0,"Dispatcher2 Failed to open port %d: %s!", config.http_port, strerror(errno));

    info(0, "started http port - use-ssl:%d", config.use_ssl);


    if (dispatcher2_init(config.dbuser, config.dbpass, config.dbname, config.dbhost, config.dbport) < 0)
          panic(0, "Initialisation failed! Perhaps no DB conn?");

    server_req_list = gwlist_create();
    gwlist_add_producer(server_req_list);

    start_request_processor(&config, server_req_list);

    /*We start processor threads to handle the HTTP request we get*/
    for(i = 0; i < config.num_threads; i++)
        gwthread_create((gwthread_func_t *) dispatch_processor, server_req_list);


    signal(SIGHUP, reopen_logs);
    signal(SIGTERM, quit_now);
    signal(SIGINT, quit_now);
    signal(SIGPIPE, SIG_IGN); /* Ignore piping us*/


    /* Deal with PID file issues */
     {
          FILE *f = fopen(pidfile, "w");
          if (f) {
               fprintf(f, "%d\n", getpid());
               fclose(f);
          }
     }

    info(0, "Entering Processing loop");
    while (!stop && (client = http_accept_request(config.http_port, &ip, &url, &rh, &body, &cgivars)) != NULL)
    {
        struct HTTPData *x;
        List *cgi_ctypes = NULL;
        int tparse = parse_cgivars(rh, body, &cgivars, &cgi_ctypes);

        info(0,"dispatcher2 Incoming Request [IP = %s] [URI=%s] :: %d",octstr_get_cstr(ip), octstr_get_cstr(url), tparse);

        if (tparse == 0 && url && supporteduri(url)) {
            x = gw_malloc(sizeof *x);
            memset(x, 0, sizeof *x);
            x->url = url;
            x->client = client;
            x->ip = ip;
            x->body = body;
            x->reqh = rh;
            x->cgivars = cgivars;
            x->cgi_ctypes = cgi_ctypes;

            gwlist_produce(server_req_list, x);
        } else {
            info(0, "We're gonna close things!");
            http_close_client(client); /* silently close things. */

            octstr_destroy(body);
            octstr_destroy(url);
            octstr_destroy(ip);
            http_destroy_cgiargs(cgivars);
            http_destroy_cgiargs(cgi_ctypes);
            http_destroy_headers(rh);
        }
    }

    stop_request_processor();

    gwlist_remove_producer(server_req_list);
    gwthread_join_every((void *)dispatch_processor);
    info(0, "dispatcher shutdown complete");

    gwlist_destroy(server_req_list, NULL);


    gwlib_shutdown();
    xmlCleanupParser();
     unlink(pidfile); /* Quit */
    return 0;
}       /* ----------  end of function main  ---------- */

static request_handler_t uri2handler(Octstr *uri) {
    int i;
    for(i = 0; i<NELEMS(uri_funcs); i++)
        if (octstr_str_case_compare(uri, uri_funcs[i].uri) == 0)
            return uri_funcs[i].func;
    return NULL;
}

static int supporteduri(Octstr *uri){
    int i;
    for(i =0; i<NELEMS(uri_funcs); i++)
        if (octstr_str_case_compare(uri, uri_funcs[i].uri) == 0)
            return 1;
    return 0;
}

static void dispatch_request(struct HTTPData  *x) {
    int status = HTTP_OK;
    Octstr *rbody = NULL;
    List *rh;
    const char *cmd = "";
    request_handler_t func;

    rh = http_create_empty_headers();
    rbody = octstr_create("");

    if ((func = uri2handler(x->url)) != NULL){
        cmd = func(rh, x, rbody, &status);
    }

    http_header_add(rh, "Server", "Dispatcher2");
    if (x->client != NULL)
        http_send_reply(x->client, status, rh, rbody);

    http_destroy_headers(rh);
    octstr_destroy(rbody);

}

void free_HTTPData(struct HTTPData *x, int free_enclosed)
{
     octstr_destroy(x->body);
     octstr_destroy(x->url);
     octstr_destroy(x->ip);

     if (x->reqh)
          http_destroy_headers(x->reqh);
     if (x->cgivars)
          http_destroy_cgiargs(x->cgivars);
     http_destroy_cgiargs(x->cgi_ctypes);

     if (free_enclosed)
          gw_free(x);
}

static void dispatch_processor(void *data)
{
    struct HTTPData *x;
    List *req_list = (List *)data;
    char port[32];
    PGconn *c; /*= PQconnectdb(config.db_conninfo);*/

     sprintf(port, "%d", config.dbport);
     c = PQsetdbLogin(config.dbhost, config.dbport > 0 ? port : NULL, NULL, NULL,
                      config.dbname, config.dbuser, config.dbpass);

    if (PQstatus(c) != CONNECTION_OK) {
        error(0, "Failed to connect to database: %s",
                PQerrorMessage(c));
        PQfinish(c);
        c = NULL;
    }

    while ((x = gwlist_consume(req_list)) != NULL) {
        x->dbconn = c;
        if (c) {
            PGresult *r = PQexec(c, "BEGIN;"); /*transactionize*/
            PQclear(r);
        }
        dispatch_request(x);
        if (c) {
            PGresult *r = PQexec(c, "COMMIT");
            PQclear(r);
        }
    }
    if (c != NULL)
        PQfinish(c);
}
