/*
 * =====================================================================================
 *
 *       Filename:  misc.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  07/07/2016 16:58:19
 *       Revision:  none
 *       Copyright: Copyright (c) 2016, GoodCitizen Co. Ltd.
 *
 *         Author:  Samuel Sekiwere (SS), sekiskylink@gmail.com
 *   Organization:
 *
 * =====================================================================================
 */
#include <ctype.h>
#include "misc.h"
#include "gwlib/mime.h"

int dispatcher2_init(char *dbuser, char *dbpass, char *dbname, char *host, int port)
{
     PGconn *c;
     char port_str[32];
     int ret;

     /* Test connection to db */
     sprintf(port_str, "%d", port);
     c = PQsetdbLogin(host, port > 0 ? port_str : NULL, NULL, NULL,
                      dbname, dbuser,dbpass);

     if (PQstatus(c) != CONNECTION_OK) {
          error(0, "dispatcher2-init: Failed to connect to database [%s@%s] with user [%s]: %s",
                dbname, host, dbuser, PQerrorMessage(c));
          ret = -1;
     } else
          ret = 0;
     PQfinish(c);
     return ret;
}

char *strip_space(char x[])
{

     char *p = x, *q;

     while (*p && isspace(*p))
          p++;
     q = p + strlen(p);

     while (--q > p && isspace(*q))
          *q = 0;

     return p;
}

int auth_user(PGconn *c, char *user, char *pass)
{
    int ret;
    PGresult *r;
    const char *pvals[] = {user, pass};

    r = PQexecParams(c, "SELECT id FROM users WHERE username = $1 AND "
            "crypt($2, password) = password", 2, NULL, pvals, NULL, NULL, 0);
    if (PQresultStatus(r) == PGRES_TUPLES_OK && PQntuples(r) > 0 ) {
        ret = 0;
    } else
        return -1;
    PQclear(r);
    return ret;
}

int ba_auth_user(PGconn *c, List *rh){
    int ret = -1;
    Octstr *p;
    List *q = NULL, *logins = NULL;
    p = http_header_value(rh, octstr_imm("Authorization"));

    info(0, "The auth header is: %s", octstr_get_cstr(p));
    if (!p)
        return -1;
    q = octstr_split_words(p);
    if (q != NULL && gwlist_len(q) == 2) {
        Octstr *u = gwlist_get(q, 1);
        if (u) {
            octstr_base64_to_binary(u);
            logins = octstr_split(u, octstr_imm(":"));
            info(0, "The logins are %s", octstr_get_cstr(u));
            if (logins && gwlist_len(logins) == 2) {
                ret = auth_user(c, octstr_get_cstr(gwlist_get(logins, 0)),
                        octstr_get_cstr(gwlist_get(logins, 1)));
            }
        }
        octstr_destroy(u);
        gwlist_destroy(q, NULL);
        gwlist_destroy(logins, NULL);
    }
    return ret;
}

int64_t save_request(PGconn *c, request_t *req)
{
    char buf1[128];
    char buf2[128];
    char buf3[128];
    char buf4[128];

    const char *pvals[12];
    int plens[12] = {0};
    int pfrmt[12] = {0};
    int64_t xid = -1;

    sprintf(buf1, "%d", req->source);
    sprintf(buf2, "%d", req->destination);
    pvals[0] = buf1;
    pvals[1] = buf2;

    pvals[2] = req->payload ? octstr_get_cstr(req->payload) : "";
    pfrmt[2] = 1;
    plens[2] = octstr_len(req->payload);

    pvals[3] = req->ctype ? octstr_get_cstr(req->ctype) : "";

    sprintf(buf3, "%ld", req->msgid);
    pvals[4] = buf3;

    pvals[5] = req->week ? octstr_get_cstr(req->week) : "";
    pvals[6] = req->month ? octstr_get_cstr(req->month) : "";

    sprintf(buf4, "%d", req->year);
    pvals[7] = buf4;
    pvals[8] = req->msisdn ? octstr_get_cstr(req->msisdn) : "";
    pvals[9] = req->raw_msg ? octstr_get_cstr(req->raw_msg) : "";
    pvals[10] = req->facility ? octstr_get_cstr(req->facility) : "";
    pvals[11] = req->report_type ? octstr_get_cstr(req->report_type) : "";

    PGresult *r;
    r = PQexecParams(c,
            "INSERT INTO requests(source, destination, body, ctype, submissionid, week,"
            "month, year, msisdn, raw_msg, facility, report_type) "
            "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12) RETURNING id",
            12, NULL, pvals, plens, pfrmt, 0);

    if (PQresultStatus(r) != PGRES_TUPLES_OK || PQntuples(r) < 1) {
        error(0, "save_reuest: %s", PQresultErrorMessage(r));
    } else {
        char *s = PQgetvalue(r, 0,0);
        xid = s && s[0] ? strtoull(s, NULL, 10) : -1;
    }
    PQclear(r);
    return xid;
}

int get_server(PGconn *c, char *name)
{
    int ret = -1;
    PGresult *r;
    const char *pvals[] = {name};

    r = PQexecParams(c, "SELECT id FROM servers WHERE name = $1",
            1, NULL, pvals, NULL, NULL, 0);
    if (PQresultStatus(r) == PGRES_TUPLES_OK && PQntuples(r) > 0) {
        ret = strtoul(PQgetvalue(r, 0, 0), NULL, 10);
    }
    PQclear(r);
    return ret;
}

int parse_cgivars(List *request_headers, Octstr *request_body,
        List **cgivars, List **cgivar_ctypes)
{
    Octstr *ctype = NULL, *charset = NULL;
    int ret = 0;
    info(0, "request_body = %s", octstr_get_cstr(request_body));
    if (request_body == NULL ||
            octstr_len(request_body) == 0 || cgivars == NULL)
        return 0; /* Nothing to do, this is a normal GET request. */

    http_header_get_content_type(request_headers, &ctype, &charset);

    if (*cgivars == NULL)
        *cgivars = gwlist_create();

    if (*cgivar_ctypes == NULL)
        *cgivar_ctypes = gwlist_create();

    if (!ctype) {
        warning(0, "dispatcher2: Parse CGI Vars: Missing Content Type!");
        ret = -1;
        goto done;
    }
    info(0, "Our content-type is %s", octstr_get_cstr(ctype));

    if (octstr_case_compare(ctype, octstr_imm("application/x-www-form-urlencoded")) == 0) {
        /* This is a normal POST form */
        List *l = octstr_split(request_body, octstr_imm("&"));
        Octstr *v;

        while ((v = gwlist_extract_first(l)) != NULL) {
            List *r = octstr_split(v, octstr_imm("="));

            if (gwlist_len(r) == 0)
                warning(0, "dispatcher2: Parse CGI Vars: Missing CGI var name/value in POST data: %s",
                        octstr_get_cstr(request_body));
            else {
                HTTPCGIVar *x = gw_malloc(sizeof *x);
                x->name =  gwlist_extract_first(r);
                x->value = gwlist_extract_first(r);
                if (!x->value)
                    x->value = octstr_create("");

                octstr_strip_blanks(x->name);
                octstr_strip_blanks(x->value);

                octstr_url_decode(x->name);
                octstr_url_decode(x->value);

                gwlist_append(*cgivars, x);
            }
            octstr_destroy(v);
            gwlist_destroy(r, octstr_destroy_item);
        }
        gwlist_destroy(l, NULL);
    } else if (octstr_case_compare(ctype, octstr_imm("multipart/form-data")) == 0) {
        /* multi-part form data */
        MIMEEntity *m = mime_http_to_entity(request_headers, request_body);
        int i, n;

        if (!m) {
            warning(0, "dispatcher2: Parse CGI Vars: Failed to parse multipart/form-data body: %s",
                    octstr_get_cstr(request_body));
            ret = -1;
            goto done;
        }
        /* Go through body parts, pick out what we need. */
        for (i = 0, n = mime_entity_num_parts(m); i < n; i++) {
            MIMEEntity *mp = mime_entity_get_part(m, i);
            List   *headers = mime_entity_headers(mp);
            Octstr *body = mime_entity_body(mp);
            Octstr *ct = http_header_value(headers,
                    octstr_imm("Content-Type"));
            Octstr *cd = http_header_value(headers,
                    octstr_imm("Content-Disposition"));
            Octstr *name = http_get_header_parameter(cd, octstr_imm("name"));

            if (name) {
                HTTPCGIVar *x = gw_malloc(sizeof *x);

                /* Strip quotes */
                if (octstr_get_char(name, 0) == '"') {
                    octstr_delete(name, 0, 1);
                    octstr_truncate(name, octstr_len(name) - 1);
                }

                x->name = octstr_duplicate(name);
                x->value = octstr_duplicate(body);

                gwlist_append(*cgivars, x);

                if (ct) { /* If the content type is set, use it. */
                    x = gw_malloc(sizeof *x);
                    x->name = octstr_duplicate(name);
                    x->value = octstr_duplicate(ct);

                    gwlist_append(*cgivar_ctypes, x);
                }
                octstr_destroy(name);
            }

            octstr_destroy(ct);
            octstr_destroy(cd);
            octstr_destroy(body);
            http_destroy_headers(headers);
            mime_entity_destroy(mp);
        }
        mime_entity_destroy(m);

    } else {
        /* else it is nothing that we know about, so simply go away... */
        info(0, "Unknown content-type: %s", octstr_get_cstr(ctype));
        ret = 0; /*XXX make it -1 */
    }
done:
    octstr_destroy(ctype);
    octstr_destroy(charset);
    return ret;
}
