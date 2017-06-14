import json
import web
from . import db, get_current_week
from app.tools.utils import get_basic_auth_credentials, auth_user


class ReportsThisWeek:
    def GET(self, facilitycode):
        year, week = get_current_week()
        reports = db.query(
            "SELECT raw_msg, msisdn FROM requests WHERE year = $yr AND week = $wk "
            "AND facility = $fcode AND status IN ('pending', 'ready', 'completed', 'failed') "
            " ORDER BY id DESC",
            {'yr': year, 'wk': '%s' % week, 'fcode': facilitycode})

        html_str = '<table class="table table-striped table-bordered table-hover">'
        reporters = []
        if reports:
            html_str += "<tr><td><strong>Accepted Reports</strong></td>"
            html_str += '<td><table class="table table-striped table-bordered table-hover">'
            html_str += '<tr><th>#</th><th>Message</th></tr>'

            for x, r in enumerate(reports):
                html_str += "<tr><td>%s" % (x + 1) + "</td><td>" + r['raw_msg'] + "</td></tr>"
                if r['msisdn'] not in reporters:
                    reporters.append(r['msisdn'])
            html_str += "</table></td></tr>"
            html_str += "<tr><td><strong>Reporters</strong></td>"
            html_str += '<td><table class="table table-striped table-bordered table-hover">'
            html_str += '<tr><th>#</th><th>Name</th><th>Phone Number</th></tr>'

            for p, reporter in enumerate(reporters):
                html_str += "<tr><td>%s" % (p + 1) + "</td><td></td><td>" + reporter + "</td></tr>"
            html_str += "</table></td></tr>"
        html_str += "</table>"
        return html_str


class ReportersXLEndpoint:
    def GET(self):
        username, password = get_basic_auth_credentials()
        r = auth_user(db, username, password)
        if not r[0]:
            web.header("Content-Type", "application/json; charset=utf-8")
            web.header('WWW-Authenticate', 'Basic realm="Auth API"')
            web.ctx.status = '401 Unauthorized'
            return json.dumps({'detail': 'Authentication failed!'})
        web.header("Content-Type", "application/zip; charset=utf-8")
        # web.header('Content-disposition', 'attachment; filename=%s.csv'%file_name)
        web.seeother("/static/downloads/reporters_all.xls.zip")


class RequestDetails:
    def GET(self, request_id):
        rs = db.query(
            "SELECT source, destination, body_pprint(body) as body, "
            "xml_is_well_formed_document(body) as is_xml, "
            "raw_msg, status, week, month, year, facility, msisdn, statuscode, errors "
            "FROM requests WHERE id = $request_id", {'request_id': request_id})

        html_str = '<table class="table table-striped table-bordered table-hover">'
        html_str += "<thead><tr><th>Field</th><th>Value</th></tr></thead><tbody>"
        if rs:
            ret = rs[0]
            html_str += "<tr><td>Facility</td><td>%s</td></tr>" % ret['facility']
            html_str += "<tr><td>Reporter</td><td>%s</td></tr>" % ret['msisdn']
            html_str += "<tr><td>Report</td><td>%s</td></tr>" % ret['raw_msg']
            html_str += "<tr><td>Week</td><td>%sW%s</td></tr>" % (ret['year'], ret['week'])
            html_str += "<tr><td>Status</td><td>%s</td></tr>" % (ret['status'])
            html_str += "<tr><td>StatusCode</td><td>%s</td></tr>" % (ret['statuscode'],)
            html_str += "<tr><td>Errors</td>"
            if ret['errors']:
                html_str += "<td><textarea rows='4' cols='25' wrap='off' readonly>%s</textarea></td></tr>" % (ret['errors'])
            else:
                html_str += "<td></td></tr>"
            if ret['is_xml']:
                html_str += "<tr><td>Body</td><td><textarea rows='10' cols='40' "
                html_str += "wrap='off' readonly>%s</textarea></td></tr>" % ret['body']

            else:
                html_str += "<tr><td>Body</td><td><pre class='language-js'>"
                html_str += "<code class='language-js'>%s<code></pre></td></tr>" % ret['body']

        html_str += "</tbody></table>"
        return html_str


class DeleteServer:
    def GET(self, server_id):
        try:
            db.query("DELETE FROM server_allowed_sources WHERE server_id = $id", {'id': server_id})
            db.query("DELETE FROM servers WHERE id = $id", {'id': server_id})
        except:
            return json.dumps({"message": "failed"})
        return json.dumps({"message": "success"})


class DeleteRequest:
    def GET(self, id):
        try:
            db.query("DELETE FROM requests WHERE id = $id", {'id': id})
        except:
            return json.dumps({"message": "failed"})
        return json.dumps({"message": "success"})
