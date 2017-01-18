/*
 * =====================================================================================
 *
 *       Filename:  conf.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/05/2016 12:03:57
 *       Revision:  none
 *       Copyright: Copyright (c) 2016, GoodCitizen Co. Ltd.
 *
 *         Author:  Samuel Sekiwere (SS), sekiskylink@gmail.com
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "conf.h"
#include "misc.h"

static int conf_init(dispatcher2conf_t config);
static int pg_init_db(char *dbhost, int dbport, char *dbname, char *dbuser, char *dbpass);

dispatcher2conf_t readconfig(char *conffile)
{
    FILE *f;
    dispatcher2conf_t config;

    assert(conffile);

    config = (dispatcher2conf_t) gw_malloc(sizeof(struct dispatcher2conf));
    if (config == NULL) {
        error(0, "readconfig: %s\n", strerror(errno));
        return NULL;
    }
    if (conf_init(config) < 0)
        return NULL;
    if (conffile) {
        f = fopen(conffile, "r");
        if (!f) {
            error(0, "readconfig: %s: %s\n", conffile, strerror(errno));
            return NULL;
        }
        info(0, "Reading configuration file: %s\n", conffile);
        if (parse_conf(f, config) < 0) {
            error(0, "readconfig: error parsing conf file.\n");
            return NULL;
        }
        fclose(f);
    }
    info(0, "readconfig: dbhost:%s,dbname:%s,dbuser:%s,\n",
        config->dbhost, config->dbname, config->dbuser);
    return config;
}


static int mygethostname(char hst[], int lim)
{
     struct hostent h;
     char buf[128], *p = NULL;

     gethostname(buf, sizeof buf);

     if (gw_gethostbyname(&h, buf, &p) < 0)
          strncpy(hst, buf, lim);
     else
          strncpy(hst, h.h_name, lim);
     if (p)
          gw_free(p);
     return 0;
}

static int conf_init(dispatcher2conf_t config)
{
    memset(config, 0, sizeof *config);

    snprintf(config->dbhost, sizeof config->dbhost, "localhost");
    snprintf(config->dbuser, sizeof config->dbuser, "postgres");
    snprintf(config->dbpass, sizeof config->dbpass, "postgres");
    snprintf(config->dbname, sizeof config->dbname, "dispatcher2");
    snprintf(config->logdir, sizeof config->logdir, "/var/log");

    mygethostname(config->myhostname, sizeof config->myhostname);

    config->dbport = 5432;
    config->http_port = 9090;
    config->max_retries = MAX_BATCH_RETRIES;
    config->request_process_interval = 1; /*  default. */
    config->use_ssl = 0;

    config->use_global_submission_period = 1;
    config->start_submission_period = 7;
    config->end_submission_period = 22;

    return 0;
}

int parse_conf(FILE *f, dispatcher2conf_t config)
{
    char field[32], xvalue[512], buf[1024], *xbuf;

    /*struct in_addr addr; */
    int loglevel = 0;

#ifdef HAVE_LIBSSL
    Octstr *ssl_client_certfile = NULL, *ssl_serv_certfile = NULL;
    Octstr *ssl_ca_file = NULL;
#endif

    conf_init(config);
    while(fgets(buf, sizeof buf, f) != NULL &&
        (xbuf = strip_space(buf)) != NULL &&
        (xbuf[0] == 0 || xbuf[0] == '#' ||
        sscanf(xbuf, "%32[^:]:%256[^\n]\n", field, xvalue) == 2)) {

        char *value = (xbuf[0] == 0 || xbuf[0] == '#') ? xbuf : strip_space(xvalue);
        int ch = (xbuf[0] == 0 || xbuf[0] == '#') ? '#' : tolower(field[0]);

        switch(ch) {
            case '#':
                break;
            case 'd':
                if (strcasecmp(field, "database") == 0)
                    snprintf(config->dbname, sizeof config->dbname,"%s", value);
                break;
            case 'e':
                if (strcasecmp(field, "end-submission-period") == 0)
                    config->end_submission_period = atoi(value);
                break;
            case 'm':
                if (strcasecmp(field, "max-concurrent") == 0)
                    config->num_threads  = strtoul(value, NULL, 16);
                else if  (strstr(field, "max-retries") != NULL)
                    config->max_retries = atoi(value);
                break;
            case 'h': /* host: database host or http_port */
                if (strcasecmp(field, "host") == 0)
                    snprintf(config->dbhost, sizeof config->dbhost, "%s", value);
                else if (strcasecmp(field, "http-port") == 0)
                    config->http_port = atoi(value);
                break;
            case 'l': /*  log dir */
                if (strstr(field, "logdir") != NULL)
                    strncpy(config->logdir, value, sizeof config->logdir);
                else if (strcasecmp(field, "loglevel") == 0)
                    loglevel = atoi(value);
                break;
            case 'p':
                if (strcasecmp(field, "password") == 0)
                    snprintf(config->dbpass, sizeof config->dbpass, "%s", value);
                else if (strcasecmp(field, "port") == 0)
                    config->dbport = atoi(value);
                break;
            case 'r':
                if (strcasecmp(field,"request-process-interval") == 0)
                    config->request_process_interval = atof(value);
            case 's':
                if (strcasecmp(field, "start-submission-period") == 0)
                    config->start_submission_period = atoi(value);
#ifdef HAVE_LIBSSL
                else if (strcasecmp(field, "ssl-client-certkey-file") == 0)
                    ssl_client_certfile = octstr_create(value);
                else if (strcasecmp(field, "ssl-server-certkey-file") == 0)
                    ssl_serv_certfile = octstr_create(value);
                else if (strcasecmp(field, "ssl-trusted-ca-file") == 0)
                    ssl_ca_file = octstr_create(value);
#endif
                 else
                    fprintf(stderr, "unknown/unsupported config option %s!\n", field);
                break;
            case 'u':
                if (strcasecmp(field, "user") == 0)
                    snprintf(config->dbuser, sizeof config->dbuser, "%s", value);
                else if (strcasecmp(field, "use-global-submission-period") == 0)
                    config->use_global_submission_period = atoi(value);
#ifdef HAVE_LIBSSL
                else if (strcasecmp(field, "use-ssl") == 0)
                    config->use_ssl = (strcasecmp(value, "true") == 0);
#endif
                    break;
        }

    }

#ifdef HAVE_LIBSSL
    if (ssl_client_certfile)
        use_global_client_certkey_file(ssl_client_certfile);

    if (ssl_serv_certfile)
        use_global_server_certkey_file(ssl_serv_certfile, ssl_serv_certfile);
    if (ssl_ca_file)
        use_global_trusted_ca_file(ssl_ca_file);

    octstr_destroy(ssl_client_certfile);
    octstr_destroy(ssl_serv_certfile);
    octstr_destroy(ssl_ca_file);
#endif

    if (config->logdir[0]) {
         char buf[512];

         sprintf(buf, "%s/dispatcher2.log", config->logdir);
         log_open(buf, loglevel, GW_NON_EXCL);
    }

    log_set_output_level(loglevel); /*  Set stderr level of logging as well */
    if (config->num_threads < DEFAULT_NUM_THREADS)
        config->num_threads = DEFAULT_NUM_THREADS;

    if (pg_init_db(config->dbhost, config->dbport, config->dbname, config->dbuser, config->dbpass) < 0)
        return -1;
    else
        return 0;
}

#include <libpq-fe.h>

#define DEFAULT_DB "template1"
#define MIN_PG_VERSION 80400 /*  v8.4 */

static int check_db_structure(PGconn *c);
static int handle_db_init(char *dbhost, char *dbport, char *dbname, char *dbuser, char *dbpass);

static int pg_init_db(char *dbhost, int dbport, char *dbname, char *dbuser, char *dbpass)
{
     PGconn *c;
     char xport[32], *port_str;
     int x;

     gw_assert(dbname);


     if (dbport > 0) {
          sprintf(xport, "%d", dbport);
          port_str = xport;
     } else
          port_str = NULL;

     /* Let's make a test connection to the DB. If it fails, we try to init the db. */
     if ((c = PQsetdbLogin(dbhost, port_str, NULL, NULL, dbname, dbuser, dbpass)) == NULL ||
         PQstatus(c) != CONNECTION_OK ||
         check_db_structure(c) < 0) {
          int x = handle_db_init(dbhost, port_str, dbname, dbuser, dbpass);
          PQfinish(c);
          if (x < 0)
               return -1;
     }  else if ((x = PQserverVersion(c)) < MIN_PG_VERSION) {
          error(1, "Current database version [%d.%d.%d] is not supported. Minimum should be v%d.%d.%d",
                (x/10000), (x/100) % 100, x % 100,
                (MIN_PG_VERSION/10000), (MIN_PG_VERSION/100) % 100, MIN_PG_VERSION % 100);
          PQfinish(c);
          return -1;
     } else
          PQfinish(c);
     return 0;
}

/* checks DB structure by looking for certain key tables. */
#define CHECK_TABLE(tbl) do {                                           \
          int res;                                                      \
          PGresult *r;                                                  \
          r = PQexec(c, "SELECT id FROM " tbl " LIMIT 1");              \
          res = (PQresultStatus(r) == PGRES_TUPLES_OK);                 \
          if (res != 1) {                                               \
               error(0, "Database not (fully) setup? Table: [" tbl "] is missing: %s", \
                     PQresultErrorMessage(r));                          \
               PQclear(r);                                              \
               return -1;                                               \
          }                                                             \
          PQclear(r);                                                   \
     } while (0)

static int check_db_structure(PGconn *c)
{

     CHECK_TABLE("servers");
     CHECK_TABLE("requests");
     CHECK_TABLE("user_roles");
     CHECK_TABLE("users");
     return 0;
}

#include "tables.h"

static int handle_db_init(char *dbhost, char *dbport, char *dbname, char *dbuser, char *dbpass)
{
     char buf[512];
     PGconn *c;
     PGresult *r;
     int i, x, err;

     info(0, "Attempting to initialise the database [%s] on host [%s] with user [%s]",
          dbname, dbhost, dbuser);
     /* first try to create the database. */
     c = PQsetdbLogin(dbhost, dbport, NULL, NULL, DEFAULT_DB, dbuser, dbpass);

     if (PQstatus(c) != CONNECTION_OK) {
          error(0, "Failed to even connect to the default PostgreSQL DB [%s], err [%s]. Quiting!",
                DEFAULT_DB, PQerrorMessage(c));
          PQfinish(c);
          return -1;
     } else if ((x = PQserverVersion(c)) < MIN_PG_VERSION) {
          error(0, "Current database version [%d.%d.%d] is not supported. Minimum should be v%d.%d.%d",
                (x/10000), (x/100) % 100, x % 100,
                (MIN_PG_VERSION/10000), (MIN_PG_VERSION/100) % 100, MIN_PG_VERSION % 100);
          PQfinish(c);
          return -1;
     }

     /* attempt to create the database. */
     sprintf(buf, "CREATE DATABASE %s WITH TEMPLATE=template0 ENCODING = \'SQL_ASCII\'", dbname);
     r = PQexec(c, buf);
     if (PQresultStatus(r) != PGRES_COMMAND_OK)
          warning(0, "pg_init: Trying to create database %s returned an error "
                  "[%s]. Proceeding with connection anyway", dbname, PQresultErrorMessage(r));
     PQclear(r);
     PQfinish(c);

     /* attempt to connect to it. */
     c = PQsetdbLogin(dbhost, dbport, NULL, NULL, dbname, dbuser, dbpass);
     if (PQstatus(c) != CONNECTION_OK) {
          error(0, "Failed to connect to DB [%s], err [%s]. Quiting!",
                dbname, PQerrorMessage(c));
          PQfinish(c);
          return -1;
     }

     info(0, "We have a connection to [%s].  will now attempt to initialise database structure. "
          "Watch out for errors, but note that some errors can be safely ignored!", dbname);
     /* we have a connection: Try to create the DB structure. */
     for (i = 0, err = 0; table_cmds[i]; i++) {
          r = PQexec(c, table_cmds[i]);
          if (PQresultStatus(r) != PGRES_COMMAND_OK) {
               warning(0, "Initialising command %d failed: %s", i+1, PQresultErrorMessage(r));
               err++;
          }
          PQclear(r);
     }

     PQfinish(c);
     if (err == i) {
          error(0, "All initialiser commands failed. Please seek help to create the database!");
          return -1;
     } else
          info(0, "Hopefully we are done initialising the database [%s] [%d error(s)], we'll try to connect to it", dbname, err);
     return 0;
}
