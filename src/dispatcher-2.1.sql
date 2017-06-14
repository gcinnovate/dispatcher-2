CREATE EXTENSION IF NOT EXISTS pgcrypto;
CREATE EXTENSION IF NOT EXISTS plpythonu;
create EXTENSION IF NOT EXISTS xml2;

CREATE TABLE servers(
    id serial PRIMARY KEY NOT NULL,
    name text NOT NULL,
    username text NOT NULL DEFAULT '',
    password text NOT NULL DEFAULT '',
    ipaddress text NOT NULL DEFAULT '',
    url text NOT NULL DEFAULT '', -- endpoint
    auth_method text NOT NULL DEFAULT '',
    use_ssl BOOLEAN NOT NULL DEFAULT 'f', --whether ssl is enabled for this server/app
    ssl_client_certkey_file TEXT NOT NULL DEFAULT '',
    start_submission_period INTEGER NOT NULL DEFAULT 0, -- starting hour for submission period
    end_submission_period INTEGER NOT NULL DEFAULT 23, -- ending hour for submission period
    xml_response_xpath TEXT NOT NULL DEFAULT '',
    json_response_jsonpath TEXT NOT NULL DEFAULT '',
    created timestamptz DEFAULT current_timestamp,
    updated timestamptz DEFAULT current_timestamp
);

CREATE TABLE server_allowed_sources(
    id serial PRIMARY KEY NOT NULL,
    server_id INTEGER NOT NULL REFERENCES servers(id),
    allowed_sources INTEGER[] NOT NULL DEFAULT ARRAY[]::INTEGER[],
    created timestamptz DEFAULT current_timestamp,
    updated timestamptz DEFAULT current_timestamp
);


CREATE TABLE requests(
    id bigserial PRIMARY KEY NOT NULL,
    source INTEGER REFERENCES servers(id), -- source app/server
    destination INTEGER REFERENCES servers(id), -- destination app/server
    body TEXT NOT NULL DEFAULT '',
    ctype TEXT NOT NULL DEFAULT '',
    status VARCHAR(32) NOT NULL DEFAULT 'ready' CHECK( status IN('ready', 'inprogress', 'failed', 'error', 'expired', 'completed')),
    statuscode text DEFAULT '',
    retries INTEGER NOT NULL DEFAULT 0,
    errors text DEFAULT '', -- indicative response message
    submissionid INTEGER NOT NULL DEFAULT 0, -- message_id in source app -> helpful when check for already sent submissions
    week text DEFAULT '', -- reporting week
    month text DEFAULT '', -- reporting month
    year INTEGER, -- year of submission
    msisdn TEXT NOT NULL DEFAULT '', -- can be report sender in source"
    raw_msg TEXT NOT NULL DEFAULT '', -- raw message in source system
    facility TEXT NOT NULL DEFAULT '', -- facility owning report
    district TEXT NOT NULL DEFAULT '', -- district
    report_type TEXT NOT NULL DEFAULT '',
    created timestamptz DEFAULT current_timestamp,
    updated timestamptz DEFAULT current_timestamp
);

CREATE INDEX requests_idx1 ON requests(submissionid);
CREATE INDEX requests_idx2 ON requests(status);
CREATE INDEX requests_idx3 ON requests(statuscode);
CREATE INDEX requests_idx4 ON requests(week);
CREATE INDEX requests_idx5 ON requests(month);
CREATE INDEX requests_idx6 ON requests(year);
CREATE INDEX requests_idx7 ON requests(ctype);

INSERT INTO servers (name, username, password, ipaddress, url, auth_method)
    VALUES
        ('localhost', 'tester', 'foobar', '127.0.0.1', 'http://localhost:8080/test', 'Basic Auth'),
        ('mtrack', 'tester', 'foobar', '127.0.0.1', 'http://localhost:8080/test', 'Basic Auth'),
        ('mtrackpro', 'tester', 'foobar', '127.0.0.1', 'http://localhost:8080/test', 'Basic Auth'),
        ('dhis2', 'tester', 'foobar', '127.0.0.1', 'http://hmis2.health.go.ug/api/dataValueSets', 'Basic Auth');

-- Add some tables for the web interface

CREATE EXTENSION IF NOT EXISTS pgcrypto;

-- for web.py sessions
CREATE TABLE sessions (
    session_id CHAR(128) UNIQUE NOT NULL,
    atime TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    data TEXT
);

CREATE TABLE user_roles (
    id BIGSERIAL NOT NULL PRIMARY KEY,
    cdate TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,
    user_role TEXT NOT NULL UNIQUE,
    descr text DEFAULT ''
);

CREATE TABLE user_role_permissions (
    id bigserial NOT NULL PRIMARY KEY,
    user_role BIGINT NOT NULL REFERENCES user_roles ON DELETE CASCADE ON UPDATE CASCADE,
    Sys_module TEXT NOT NULL, -- the name of the module - defined above this level
    sys_perms VARCHAR(16) NOT NULL,
    created timestamptz DEFAULT current_timestamp,
    updated timestamptz DEFAULT current_timestamp,
    UNIQUE(sys_module, user_role)
);

CREATE TABLE users (
    id bigserial NOT NULL PRIMARY KEY,
    cdate timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
    firstname TEXT NOT NULL,
    lastname TEXT NOT NULL,
    username TEXT NOT NULL UNIQUE,
    password TEXT NOT NULL, -- blowfish hash of password
    email TEXT,
    user_role  BIGINT NOT NULL REFERENCES user_roles ON DELETE RESTRICT ON UPDATE CASCADE,
    transaction_limit TEXT DEFAULT '0/'||to_char(NOW(),'yyyymmdd'),
    is_active BOOLEAN NOT NULL DEFAULT 't',
    is_system_user BOOLEAN NOT NULL DEFAULT 'f',
    created timestamptz DEFAULT current_timestamp,
    updated timestamptz DEFAULT current_timestamp

);

-- FUNCTIONS
-- Check if source is an allowed "source" for destination server/app dest
CREATE OR REPLACE FUNCTION is_allowed_source(source integer, dest integer) RETURNS BOOLEAN AS $delim$
    DECLARE
     t boolean;
    BEGIN
        select source = ANY(allowed_sources) INTO t FROM server_allowed_sources WHERE server_id = dest;
        RETURN t;
    END;
$delim$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION in_submission_period(server_id integer) RETURNS BOOLEAN AS $delim$
    DECLARE
    t boolean;
    BEGIN
       SELECT
        to_char(current_timestamp, 'HH24')::int >= start_submission_period
        AND
        to_char(current_timestamp, 'HH24')::int <= end_submission_period INTO t
        FROM servers WHERE id = server_id;
        RETURN t;
    END;
$delim$ LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION pp_json(j TEXT, sort_keys BOOLEAN = TRUE, indent TEXT = '    ')
RETURNS TEXT AS $$
    import simplejson as json
    if not j:
        return ''
    return json.dumps(json.loads(j), sort_keys=sort_keys, indent=indent)
$$ LANGUAGE plpythonu;

CREATE OR REPLACE FUNCTION xml_pretty(xml text)
RETURNS xml AS $$
	SELECT xslt_process($1,
'<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:strip-space elements="*" />
<xsl:output method="xml" indent="yes" />
<xsl:template match="node() | @*">
<xsl:copy>
<xsl:apply-templates select="node() | @*" />
</xsl:copy>
</xsl:template>
</xsl:stylesheet>')::xml
$$ LANGUAGE SQL IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION body_pprint(body text)
    RETURNS TEXT AS $$
    BEGIN
        IF xml_is_well_formed_document(body) THEN
            return xml_pretty(body)::text;
        ELSE
            return pp_json(body, 't', '    ');
        END IF;
    END;
$$ LANGUAGE plpgsql;

-- Data Follows
INSERT INTO user_roles(user_role, descr)
VALUES('Administrator','For the Administrators');

INSERT INTO user_role_permissions(user_role, sys_module,sys_perms)
VALUES
        ((SELECT id FROM user_roles WHERE user_role ='Administrator'),'Users','rmad');

INSERT INTO users(firstname,lastname,username,password,email,user_role,is_system_user)
VALUES
        ('Samuel','Sekiwere','admin',crypt('admin',gen_salt('bf')),'sekiskylink@gmail.com',
        (SELECT id FROM user_roles WHERE user_role ='Administrator'),'t');
