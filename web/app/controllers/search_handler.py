import web
from . import db, require_login, get_session, render
from app.tools.pagination2 import doquery, countquery, getPaginationString
from app.tools.utils import default, lit
from settings import PAGE_LIMIT


class Search:
    @require_login
    def GET(self):
        params = web.input(page=1)
        try:
            page = int(params.page)
        except:
            page = 1
        session = get_session()
        limit = PAGE_LIMIT
        start = (page - 1) * limit if page > 0 else 0

        dic = lit(relations='servers', fields="*", criteria="", order="id desc", limit=limit, offset=start)
        servers = doquery(db, dic)
        servers2 = doquery(db, dic)

        dic = lit(
            relations='requests', fields="*",
            criteria=session.criteria,
            order="id desc",
            limit=limit, offset=start)
        res = doquery(db, dic)
        count = countquery(db, dic)
        pagination_str = getPaginationString(default(page, 0), count, limit, 2, "search", "?page=")

        l = locals()
        del l['self']
        return render.search(**l)

    def POST(self):
        params = web.input(
            page=1, reqid=[], submissionid="", body="", sdate="", edate="",
            status="", year="", week="", pbtn="", msisdn="", facility="")
        try:
            page = int(params.page)
        except:
            page = 1
        session = get_session()

        limit = PAGE_LIMIT
        start = (page - 1) * limit if page > 0 else 0

        dic = lit(relations='servers', fields="*", criteria="", order="id desc", limit=limit, offset=start)
        servers = doquery(db, dic)
        servers2 = doquery(db, dic)

        with db.transaction():
            if params.pbtn == 'Retry Selected':
                if params.reqid:
                    for val in params.reqid:
                        db.update('requests', where="id = %s" % val, status='ready')
            if params.pbtn == 'Cancel Selected':
                if params.reqid:
                    for val in params.reqid:
                        db.update('requests', where="id = %s" % val, status='canceled')
            db.transaction().commit()

        criteria = "TRUE "
        if params.submissionid:
            criteria += " AND submissionid = %s" % params.submissionid
        if params.status:
            criteria += " AND status = '%s' " % params.status
        if params.sdate:
            criteria += " AND created >= '%s'" % params.sdate
        if params.edate:
            criteria += " AND created <= '%s'" % params.edate
        if params.week:
            criteria += " AND week = '%s'" % params.week
        if params.body:
            criteria += " AND body ILIKE '%%%s%%'" % params.body
        if params.year:
            criteria += " AND year = '%s'" % params.year
        if params.msisdn:
            criteria += " AND msisdn = '%s'" % params.msisdn
        if params.facility:
            criteria += " AND facility = '%s'" % params.facility
        if params.formatting:
            if params.formatting == "xml":
                criteria += " AND xml_is_well_formed(body)"

        print criteria
        if len(criteria) > 5:
            session.criteria = criteria
        dic = lit(
            relations='requests', fields="*",
            criteria=criteria,
            order="id desc",
            limit=limit, offset=start)
        res = doquery(db, dic)
        count = countquery(db, dic)
        pagination_str = getPaginationString(default(page, 0), count, limit, 2, "search", "?page=")

        l = locals()
        del l['self']
        return render.search(**l)
