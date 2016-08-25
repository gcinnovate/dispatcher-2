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
    offpeak_start INTEGER NOT NULL DEFAULT 0, -- starting hour for off peak period
    offpeak_end INTEGER NOT NULL DEFAULT 1, -- ending hour for off peak period
    created timestamptz DEFAULT current_timestamp,
    updated timestamptz DEFAULT current_timestamp
);

CREATE TABLE requests(
    id bigserial PRIMARY KEY NOT NULL,
    source INTEGER REFERENCES servers(id), -- source app/server
    destination INTEGER REFERENCES servers(id), -- source app/server
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
