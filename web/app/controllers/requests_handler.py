import web
from . import db, require_login, csrf_protected, render, get_session
from app.tools.pagination2 import doquery, countquery, getPaginationString
from app.tools.utils import default, lit, audit_log
from settings import PAGE_LIMIT


class Requests:
    @require_login
    def GET(self):
        params = web.input(page=1)
        try:
            page = int(params.page)
        except:
            page = 1

        limit = PAGE_LIMIT
        start = (page - 1) * limit if page > 0 else 0

        dic = lit(relations='requests', fields="*", criteria="", order="id desc", limit=limit, offset=start)
        res = doquery(db, dic)
        count = countquery(db, dic)
        pagination_str = getPaginationString(default(page, 0), count, limit, 2, "requests", "?page=")

        l = locals()
        del l['self']
        return render.requests(**l)

    @csrf_protected
    @require_login
    def POST(self):
        session = get_session()
        username = session.username
        params = web.input(page=1, ed="", d_id="", reqid=[])
        try:
            page = int(params.page)
        except:
            page = 1

        with db.transaction():
            if params.abtn == 'Retry Selected':
                if params.reqid:
                    for val in params.reqid:
                        db.update('requests', status='ready', where="id = %s" % val)
                    log_dict = {
                        'logtype': 'Web', 'action': 'Retry Requests', 'actor': username,
                        'ip': web.ctx['ip'],
                        'descr': 'User %s retried %s request(s)' % (username, len(params.reqid)),
                        'user': session.sesid
                    }
                    audit_log(db, log_dict)
                db.transaction().commit()
                return web.seeother("/requests")
            if params.abtn == 'Cancel Selected':
                if params.reqid:
                    for val in params.reqid:
                        db.update('requests', status='canceled', where="id = %s" % val)
                    log_dict = {
                        'logtype': 'Web', 'action': 'Cancel Requests', 'actor': username,
                        'ip': web.ctx['ip'],
                        'descr': 'User %s canceled %s request(s)' % (username, len(params.reqid)),
                        'user': session.sesid
                    }
                    audit_log(db, log_dict)
                db.transaction().commit()
                return web.seeother("/requests")

        limit = PAGE_LIMIT
        start = (page - 1) * limit if page > 0 else 0

        dic = lit(relations='requests', fields="*", criteria="", order="id desc", limit=limit, offset=start)
        res = doquery(db, dic)
        count = countquery(db, dic)
        pagination_str = getPaginationString(default(page, 0), count, limit, 2, "requests", "?page=")

        l = locals()
        del l['self']
        return render.requests(**l)
