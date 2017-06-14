# -*- coding: utf-8 -*-

"""Mako template options which are used, basically, by all handler modules in
controllers of the app.
"""

# from web.contrib.template import render_mako
import web
import datetime
from web.contrib.template import render_jinja
from settings import (absolute, config)

db_host = config['db_host']
db_name = config['db_name']
db_user = config['db_user']
db_passwd = config['db_passwd']
db_port = config['db_port']

db = web.database(
    dbn='postgres',
    user=db_user,
    pw=db_passwd,
    db=db_name,
    host=db_host,
    port=db_port
)


def get_current_week(date=datetime.datetime.now()):
    """Given date, return the reporting week in the format 2016, 01
    reports coming in this week are for previous one.
    """
    offset_from_last_sunday = datetime.datetime.now().weekday() + 1
    last_sunday = date - datetime.timedelta(days=offset_from_last_sunday)
    year, weeknum, _ = last_sunday.isocalendar()
    return (year, weeknum)

SESSION = ''
APP = None

userRolesByName = {}
rs = db.query("SELECT id, name from user_roles order by name")
for r in rs:
    userRolesByName[r['name']] = r['id']

ourServers = []
serversById = {}
rs = db.query("SELECT id, name FROM servers order BY name")
for r in rs:
    serversById[r['id']] = r['name']
    ourServers.append({'id': r['id'], 'name': r['name']})


def put_app(app):
    global APP
    APP = app


def get_app():
    global APP
    return APP


def get_session():
    global SESSION
    return SESSION


def datetimeformat(value, fmt='%Y-%m-%d'):
    if not value:
        return ''
    return value.strftime(fmt)

myFilters = {
    'datetimeformat': datetimeformat,
}

# Jinja2 Template options
render = render_jinja(
    absolute('app/views'),
    encoding='utf-8'
)

render._lookup.globals.update(
    ses=get_session(), ourServers=ourServers
)
render._lookup.filters.update(myFilters)


def put_session(session):
    global SESSION
    SESSION = session
    render._lookup.globals.update(ses=session)


def csrf_token():
    session = get_session()
    if 'csrf_token' not in session:
        from uuid import uuid4
        session.csrf_token = uuid4().hex
    return session.csrf_token


def csrf_protected(f):
    def decorated(*args, **kwargs):
        inp = web.input()
        session = get_session()
        if not ('csrf_token' in inp and inp.csrf_token == session.pop('csrf_token', None)):
            raise web.HTTPError(
                "400 Bad request",
                {'content-type': 'text/html'},
                """Cross-site request forgery (CSRF) attempt (or stale browser form).
<a href="/"></a>.""")  # Provide a link back to the form
        return f(*args, **kwargs)
    return decorated

render._lookup.globals.update(csrf_token=csrf_token)


def require_login(f):
    """usage
    @require_login
    def GET(self):
        ..."""
    def decorated(*args, **kwargs):
        session = get_session()
        if not session.loggedin:
            session.logon_err = "Please Logon"
            return web.seeother("/")
        else:
            session.logon_err = ""
        return f(*args, **kwargs)

    return decorated
