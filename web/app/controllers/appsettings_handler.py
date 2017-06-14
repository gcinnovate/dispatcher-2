import web
from . import csrf_protected, db, require_login, render, get_session


class AppSettings:
    @require_login
    def GET(self):
        params = web.input(page=1, ed="", d_id="")
        edit_val = params.ed
        allow_edit = False
        session = get_session()

        try:
            edit_val = int(params.ed)
            allow_edit = True
        except ValueError:
            pass

        if params.ed and allow_edit:
            res = db.query("SELECT * FROM servers WHERE id = $id", {'id': edit_val})
            if res:
                r = res[0]
                name = r.name
                username = r.username
                passwd = r.password
                url = r.url
                auth_method = r.auth_method
                start = r.start_submission_period
                end = r.end_submission_period
                xml_xpath = r.xml_response_xpath
                json_xpath = r.json_response_xpath
                use_ssl = r.use_ssl
                ssl_client_certkey_file = r.ssl_client_certkey_file

        if params.d_id:
            if session.role == 'Administrator':
                db.query(
                    "DELETE FROM server_allowed_sources WHERE server_id = $id", {'id': params.d_id})
                db.query("DELETE FROM servers WHERE id = $id", {'id': params.d_id})

        servers_opts = db.query("SELECT id, name FROM servers")
        servers = db.query("SELECT * FROM servers")
        l = locals()
        del l['self']
        return render.appsettings(**l)

    @csrf_protected
    @require_login
    def POST(self):
        params = web.input(
            page="1", ed="", d_id="", name="", username="", cpasswd="", url="", allowed_apps=[],
            auth_method="", start="", end="", xml_xpath="", json_xpath="", use_ssl="",
            ssl_client_certkey_file="", ssl_server_certkey_file="")
        try:
            page = int(params.page)
        except:
            page = 1
        use_ssl = 't' if params.use_ssl == "on" else 'f'
        with db.transaction():
            if params.ed:
                db.query(
                    "UPDATE servers SET (name, username, password, url, auth_method, "
                    "start_submission_period, end_submission_period, xml_response_xpath,"
                    "json_response_xpath, use_ssl, ssl_client_certkey_file) = "
                    "($name, $username, $password, $url, $auth_method, $start, $end, "
                    "$xml_xpath, $json_xpath, $use_ssl, $certkey) WHERE id=$id", {
                        'name': params.name, 'username': params.username,
                        'password': params.cpasswd, 'url': params.url, 'auth_method': params.auth_method,
                        'start': params.start, 'end': params.end,
                        'certkey': params.ssl_client_certkey_file, 'use_ssl': use_ssl,
                        'xml_xpath': params.xml_xpath, 'json_xpath': params.json_xpath,
                        'id': params.ed})
                myapps = str(map(int, params.allowed_apps))
                apps_array = str(myapps).replace(
                    '[', '{').replace(']', '}').replace('\'', '\"')
                db.query(
                    "UPDATE server_allowed_sources SET allowed_sources = $apps::INTEGER[] "
                    "WHERE server_id = $id", {
                        'id': params.ed, 'apps': apps_array})

                return web.seeother("/appsettings")
            else:
                res = db.query(
                    "INSERT INTO servers (name, username, password, url, auth_method, "
                    "start_submission_period, end_submission_period, ssl_client_certkey_file,"
                    "use_ssl, xml_response_xpath, json_response_xpath) VALUES("
                    "$name, $username, $password, $url, $auth_method, $start, $end, "
                    "$certkey, $use_ssl, $xml_xpath, $json_xpath) RETURNING id", {
                        'name': params.name, 'username': params.username,
                        'password': params.cpasswd, 'url': params.url, 'auth_method': params.auth_method,
                        'start': params.start, 'end': params.end,
                        'certkey': params.ssl_client_certkey_file, 'use_ssl': params.use_ssl,
                        'xml_xpath': params.xml_xpath, 'json_xpath': params.json_xpath})

                if res:
                    server_id = res[0]['id']
                    myapps = str(map(int, params.allowed_apps))
                    apps_array = str(myapps).replace(
                        '[', '{').replace(']', '}').replace('\'', '\"')
                    db.query(
                        "INSERT INTO server_allowed_sources (server_id, allowed_sources) "
                        " VALUES($server_id, $apps::INTEGER[])", {
                            'server_id': server_id,
                            'apps': apps_array})
                return web.seeother("/appsettings")

        l = locals()
        del l['self']
        return render.appsettings(**l)
