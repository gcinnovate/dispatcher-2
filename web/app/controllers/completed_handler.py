import web
import parsedatetime
from . import db, require_login, render, get_session
from app.tools.pagination2 import doquery, countquery, getPaginationString
from app.tools.utils import default, lit
from settings import PAGE_LIMIT

cal = parsedatetime.Calendar()


class Completed:
    @require_login
    def GET(self):
        params = web.input(page=1)
        try:
            page = int(params.page)
        except:
            page = 1

        limit = PAGE_LIMIT
        start = (page - 1) * limit if page > 0 else 0

        dic = lit(
            relations='requests', fields="*",
            criteria="status='completed'",
            order="id desc",
            limit=limit, offset=start)
        res = doquery(db, dic)
        count = countquery(db, dic)
        pagination_str = getPaginationString(default(page, 0), count, limit, 2, "completed", "?page=")

        l = locals()
        del l['self']
        return render.completed(**l)

    @require_login
    def POST(self):
        params = web.input(page=1, reqid=[])
        try:
            page = int(params.page)
        except:
            page = 1
        limit = PAGE_LIMIT
        start = (page - 1) * limit if page > 0 else 0

        with db.transaction():
            if params.abtn == 'Resend Selected':
                if params.reqid:
                    for val in params.reqid:
                        db.update(
                            'requests', where="id = %s" % val, status='ready', updated='NOW()')
                    log_dict = {
                        'logtype': 'Web', 'action': 'Resend Requests', 'actor': username,
                        'ip': web.ctx['ip'],
                        'descr': 'User %s resent %s request(s)' % (username, len(params.reqid)),
                        'user': session.sesid
                    }
                    audit_log(db, log_dict)
                db.transaction().commit()
            if params.abtn == 'Delete Selected':
                if params.reqid:
                    for val in params.reqid:
                        db.delete('requests', where="id = %s" % val)
                    log_dict = {
                        'logtype': 'Web', 'action': 'Delete Requests', 'actor': username,
                        'ip': web.ctx['ip'],
                        'descr': 'User %s deleted %s request(s)' % (username, len(params.reqid)),
                        'user': session.sesid
                    }
                    audit_log(db, log_dict)
                db.transaction().commit()

        dic = lit(
            relations='requests', fields="*",
            criteria="status='completed'",
            order="id desc",
            limit=limit, offset=start)
        res = doquery(db, dic)
        count = countquery(db, dic)
        pagination_str = getPaginationString(default(page, 0), count, limit, 2, "completed", "?page=")

        l = locals()
        del l['self']
        return render.completed(**l)
