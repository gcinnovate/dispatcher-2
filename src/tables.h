#ifndef __DB_TABLES_INCLUDED__
#define __DB_TABLES_INCLUDED__
static char *table_cmds[] = {
"CREATE EXTENSION IF NOT EXISTS pgcrypto;\n"
"CREATE EXTENSION IF NOT EXISTS plpythonu;\n"
"CREATE EXTENSION xml2;\n"
"CREATE TABLE servers(\n"
"    id serial PRIMARY KEY NOT NULL,\n"
"    name text NOT NULL,\n"
"    username text NOT NULL DEFAULT '',\n"
"    password text NOT NULL DEFAULT '',\n"
"    ipaddress text NOT NULL DEFAULT '',\n"
"    url text NOT NULL DEFAULT '', -- endpoint\n"
"    auth_method text NOT NULL DEFAULT '',\n"
"    use_ssl BOOLEAN NOT NULL DEFAULT 'f', --whether ssl is enabled for this server/app\n"
"    ssl_client_certkey_file TEXT NOT NULL DEFAULT '',\n"
"    start_submission_period INTEGER NOT NULL DEFAULT 0, -- starting hour for off peak period\n"
"    end_submission_period INTEGER NOT NULL DEFAULT 1, -- ending hour for off peak period\n"
"    xml_response_xpath TEXT NOT NULL DEFAULT '',\n"
"    json_response_xpath TEXT NOT NULL DEFAULT '',\n"
"    created timestamptz DEFAULT current_timestamp,\n"
"    updated timestamptz DEFAULT current_timestamp\n"
");\n"
"\n"
,
"CREATE TABLE server_allowed_sources(\n"
"    id serial PRIMARY KEY NOT NULL,\n"
"    server_id INTEGER NOT NULL REFERENCES servers(id),\n"
"    allowed_sources INTEGER[] NOT NULL DEFAULT ARRAY[]::INTEGER[],\n"
"    created timestamptz DEFAULT current_timestamp,\n"
"    updated timestamptz DEFAULT current_timestamp\n"
");\n"
"\n"
,
"CREATE TABLE requests(\n"
"    id bigserial PRIMARY KEY NOT NULL,\n"
"    source INTEGER REFERENCES servers(id), -- source app/server\n"
"    destination INTEGER REFERENCES servers(id), -- source app/server\n"
"    body TEXT NOT NULL DEFAULT '',\n"
"    ctype TEXT NOT NULL DEFAULT '',\n"
"    status VARCHAR(32) NOT NULL DEFAULT 'ready' CHECK( status IN('pending', 'ready', 'inprogress', 'failed', 'error', 'expired', 'completed', 'canceled')),\n"
"    statuscode text DEFAULT '',\n"
"    retries INTEGER NOT NULL DEFAULT 0,\n"
"    errors TEXT DEFAULT '', -- indicative response message\n"
"    submissionid INTEGER NOT NULL DEFAULT 0, -- message_id in source app -> helpful when check for already sent submissions\n"
"    week TEXT DEFAULT '', -- reporting week\n"
"    month TEXT DEFAULT '', -- reporting month\n"
"    year INTEGER, -- year of submission\n"
"    msisdn TEXT NOT NULL DEFAULT '', -- can be report sender in source\n"
"    raw_msg TEXT NOT NULL DEFAULT '', -- raw message in source system\n"
"    facility TEXT NOT NULL DEFAULT '', -- facility owning report\n"
"    district TEXT NOT NULL DEFAULT '', -- district\n"
"    report_type TEXT NOT NULL DEFAULT '',\n"
"    created timestamptz DEFAULT current_timestamp,\n"
"    updated timestamptz DEFAULT current_timestamp\n"
");\n"
"\n"
"CREATE INDEX requests_idx1 ON requests(submissionid);\n"
"CREATE INDEX requests_idx2 ON requests(status);\n"
"CREATE INDEX requests_idx3 ON requests(statuscode);\n"
"CREATE INDEX requests_idx4 ON requests(week);\n"
"CREATE INDEX requests_idx5 ON requests(month);\n"
"CREATE INDEX requests_idx6 ON requests(year);\n"
"CREATE INDEX requests_idx7 ON requests(ctype);\n"
"CREATE INDEX requests_idx8 ON requests(msisdn);\n"
"CREATE INDEX requests_idx9 ON requests(facility);\n"
"CREATE INDEX requests_idx10 ON requests(district);\n"
"\n"
,
"INSERT INTO servers (name, username, password, ipaddress, url, auth_method)\n"
"    VALUES\n"
"        ('localhost', 'tester', 'foobar', '127.0.0.1', 'http://localhost:8080/test', 'Basic Auth'),\n"
"        ('mtrack', 'tester', 'foobar', '127.0.0.1', 'http://localhost:8080/test', 'Basic Auth'),\n"
"        ('mtrackpro', 'tester', 'foobar', '127.0.0.1', 'http://localhost:8080/test', 'Basic Auth'),\n"
"        ('dhis2', 'tester', 'foobar', '127.0.0.1', 'http://hmis2.health.go.ug/api/dataValueSets', 'Basic Auth');\n"
"\n"
"-- Add some tables for the web interface\n"
"\n"
,
"CREATE EXTENSION IF NOT EXISTS pgcrypto;\n"
"\n"
"-- for web.py sessions\n"
"CREATE TABLE sessions (\n"
"    session_id CHAR(128) UNIQUE NOT NULL,\n"
"    atime TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,\n"
"    data TEXT\n"
");\n"
"\n"
,
"CREATE TABLE user_roles (\n"
"    id BIGSERIAL NOT NULL PRIMARY KEY,\n"
"    cdate TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,\n"
"    name TEXT NOT NULL UNIQUE,\n"
"    descr text DEFAULT ''\n"
");\n"
"\n"
,
"CREATE TABLE user_role_permissions (\n"
"    id bigserial NOT NULL PRIMARY KEY,\n"
"    user_role BIGINT NOT NULL REFERENCES user_roles ON DELETE CASCADE ON UPDATE CASCADE,\n"
"    Sys_module TEXT NOT NULL, -- the name of the module - defined above this level\n"
"    sys_perms VARCHAR(16) NOT NULL,\n"
"    created timestamptz DEFAULT current_timestamp,\n"
"    updated timestamptz DEFAULT current_timestamp,\n"
"    UNIQUE(sys_module, user_role)\n"
");\n"
"\n"
,
"CREATE TABLE users (\n"
"    id bigserial NOT NULL PRIMARY KEY,\n"
"    cdate timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,\n"
"    firstname TEXT NOT NULL,\n"
"    lastname TEXT NOT NULL,\n"
"    username TEXT NOT NULL UNIQUE,\n"
"	 telephone TEXT NOT NULL DEFAULT '',\n"
"    password TEXT NOT NULL, -- blowfish hash of password\n"
"    email TEXT,\n"
"    user_role  BIGINT NOT NULL REFERENCES user_roles ON DELETE RESTRICT ON UPDATE CASCADE,\n"
"    transaction_limit TEXT DEFAULT '0/'||to_char(NOW(),'yyyymmdd'),\n"
"    is_active BOOLEAN NOT NULL DEFAULT 't',\n"
"    is_system_user BOOLEAN NOT NULL DEFAULT 'f',\n"
"    created timestamptz DEFAULT current_timestamp,\n"
"    updated timestamptz DEFAULT current_timestamp\n"
"\n"
");\n"
"\n"
,
"CREATE TABLE audit_log (\n"
"        id BIGSERIAL NOT NULL PRIMARY KEY,\n"
"        logtype VARCHAR(32) NOT NULL DEFAULT '',\n"
"        actor TEXT NOT NULL,\n"
"        action text NOT NULL DEFAULT '',\n"
"        remote_ip INET,\n"
"        detail TEXT NOT NULL,\n"
"        created_by INTEGER REFERENCES users(id), -- like actor id\n"
"        created TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP\n"
");\n"
"\n"
"CREATE INDEX au_idx1 ON audit_log(created);\n"
"CREATE INDEX au_idx2 ON audit_log(logtype);\n"
"CREATE INDEX au_idx4 ON audit_log(action);\n"
,
"-- FUNCTIONS\n"
"-- Check if source is an allowed 'source' for destination server/app dest\n"
"CREATE OR REPLACE FUNCTION is_allowed_source(source integer, dest integer) RETURNS BOOLEAN AS $delim$\n"
"    DECLARE\n"
"     t boolean;\n"
"    BEGIN\n"
"        select source = ANY(allowed_sources) INTO t FROM server_allowed_sources WHERE server_id = dest;\n"
"        RETURN t;\n"
"    END;\n"
"$delim$ LANGUAGE plpgsql;\n"
"\n"
,
"CREATE OR REPLACE FUNCTION in_submission_period(server_id integer) RETURNS BOOLEAN AS $delim$\n"
"    DECLARE\n"
"    t boolean;\n"
"    BEGIN\n"
"       SELECT\n"
"        to_char(current_timestamp, 'HH24')::int >= start_submission_period\n"
"        AND\n"
"        to_char(current_timestamp, 'HH24')::int <= end_submission_period INTO t\n"
"        FROM servers WHERE id = server_id;\n"
"        RETURN t;\n"
"    END;\n"
"$delim$ LANGUAGE plpgsql;\n"
"\n"
,
"CREATE OR REPLACE FUNCTION pp_json(j TEXT, sort_keys BOOLEAN = TRUE, indent TEXT = '    ')\n"
"RETURNS TEXT AS $$\n"
"  import simplejson as json\n"
"  if not j:\n"
"      j = []"
"  return json.dumps(json.loads(j), sort_keys=sort_keys, indent=indent)\n"
"$$ LANGUAGE plpythonu;\n"
"\n"
,
"CREATE OR REPLACE FUNCTION body_pprint(body text)\n"
"    RETURNS TEXT AS $$\n"
"    BEGIN\n"
"        IF xml_is_well_formed_document(body) THEN\n"
"            return xml_pretty(body)::text;\n"
"        ELSE\n"
"            return pp_json(body, 't', '    ');\n"
"        END IF;\n"
"    END;\n"
"$$ LANGUAGE plpgsql;\n"
"\n"
,
"-- Data Follows\n"
"INSERT INTO user_roles(user_role, descr)\n"
"VALUES('Administrator','For the Administrators');\n"
"\n"
,
"INSERT INTO user_role_permissions(user_role, sys_module,sys_perms)\n"
"VALUES\n"
"        ((SELECT id FROM user_roles WHERE user_role ='Administrator'),'Users','rmad');\n"
"\n"
,
"INSERT INTO users(firstname,lastname,username,password,email,user_role,is_system_user)\n"
"VALUES\n"
"        ('Samuel','Sekiwere','admin',crypt('admin',gen_salt('bf')),'sekiskylink@gmail.com',\n"
"        (SELECT id FROM user_roles WHERE user_role ='Administrator'),'t');\n"
"\n"
,
"INSERT INTO server_allowed_sources (server_id, allowed_sources)\n"
"    VALUES((SELECT id FROM servers where name = 'dhis2'),\n"
"        (SELECT array_agg(id) FROM servers WHERE name in ('mtrack', 'mtrackpro')));\n"
"\n"
,NULL
};
#endif
