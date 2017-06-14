import web
from . import csrf_protected, db, require_login, render, get_session
from app.tools.pagination2 import doquery, countquery, getPaginationString
from app.tools.utils import default, lit
from settings import PAGE_LIMIT


class Facilities:
    @require_login
    def GET(self):
        params = web.input(page=1, ed="", d_id="")
        edit_val = params.ed
        session = get_session()
        try:
            page = int(params.page)
        except:
            page = 1
        roles = db.query(
            "SELECT id, name from reporter_groups WHERE name "
            "IN ('VHT', 'HC', 'Incharge', 'Records Assistant') order by name")
        limit = PAGE_LIMIT
        start = (page - 1) * limit if page > 0 else 0
        if session.role == 'District User':
            criteria = "district='%s'" % session.username.capitalize()
        else:
            criteria = ""
        dic = lit(
            relations='healthfacilities', fields="id, name, type_id, district, location_name, is_033b, code",
            criteria=criteria,
            order="district, name asc",
            limit=limit, offset=start)
        facilities = doquery(db, dic)
        count = countquery(db, dic)
        pagination_str = getPaginationString(default(page, 0), count, limit, 2, "facilities", "?page=")

        l = locals()
        del l['self']
        return render.facilities(**l)

    @csrf_protected
    @require_login
    def POST(self):
        params = web.input(page="1", ed="", d_id="")
        try:
            page = int(params.page)
        except:
            page = 1

        with db.transaction():
            if params.ed:
                return web.seeother("/facilities")
            else:
                return web.seeother("/facilities")

        l = locals()
        del l['self']
        return render.facilities(**l)
