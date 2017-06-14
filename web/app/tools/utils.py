import web
import re
import base64
import phonenumbers
import simplejson
import datetime
import psycopg2.extras
from settings import config


def format_msisdn(msisdn=None):
    """ given a msisdn, return in E164 format """
    assert msisdn is not None
    msisdn = msisdn.replace(' ', '')
    num = phonenumbers.parse(msisdn, getattr(config, 'country', 'UG'))
    is_valid = phonenumbers.is_valid_number(num)
    if not is_valid:
        return None
    return phonenumbers.format_number(
        num, phonenumbers.PhoneNumberFormat.E164)


def lit(**keywords):
    return keywords


def default(*args):
    p = [i for i in args if i or i == 0]
    if p.__len__():
        return p[0]
    if args.__len__():
        return args[args.__len__() - 1]
    return None


def auth_user(db, username, password):
    sql = (
        "SELECT a.id, a.firstname, a.lastname, b.name as role "
        "FROM users a, user_roles b "
        "WHERE username = $username AND password = crypt($passwd, password) "
        "AND a.user_role = b.id AND is_active = 't'")
    res = db.query(sql, {'username': username, 'passwd': password})
    if not res:
        return False, "Wrong username or password"
    else:
        return True, res[0]


def audit_log(db, log_dict={}):
    sql = (
        "INSERT INTO audit_log (logtype, actor, action, remote_ip, detail, created_by) "
        " VALUES ($logtype, $actor, $action, $ip, $descr, $user) "
    )
    db.query(sql, log_dict)
    return None


def get_basic_auth_credentials():
    auth = web.ctx.env.get('HTTP_AUTHORIZATION')
    if not auth:
        return (None, None)
    auth = re.sub('^Basic ', '', auth)
    username, password = base64.decodestring(auth).split(':')
    return username, password


def queue_schedule(db, params, run_time, user=None, stype='sms'):  # params has the text, recipients and other params
    res = db.query(
        "INSERT INTO schedules (params, run_time, type, created_by) "
        " VALUES($params, $runtime, $type, $user) RETURNING id",
        {
            'params': psycopg2.extras.Json(params, dumps=simplejson.dumps),
            'runtime': run_time,
            'user': user,
            'type': stype
        })
    if res:
        return res[0]['id']
    return None


def update_queued_sms(db, sched_id, params, run_time, user=None):
    db.query(
        "UPDATE schedules SET params=$params, run_time=$runtime, updated_by=$user, "
        " status='ready', updated=now() WHERE id=$id",
        {
            'params': psycopg2.extras.Json(params, dumps=simplejson.dumps),
            'runtime': run_time,
            'user': user,
            'id': sched_id
        })


def log_schedule(db, distribution_log_id, sched_id, level, triggered_by=1):
    db.query(
        "INSERT INTO distribution_log_schedules(distribution_log_id, schedule_id, level, triggered_by) "
        "VALUES($log_id, $sched_id, $level, $triggered_by)", {
            'log_id': distribution_log_id, 'sched_id': sched_id,
            'level': level, 'triggered_by': triggered_by})


def get_reporting_week(date):
    """Given date, return the reporting week in the format 2016W01
    reports coming in this week are for previous one.
    """
    offset_from_last_sunday = date.weekday() + 1
    last_sunday = date - datetime.timedelta(days=offset_from_last_sunday)
    year, weeknum, _ = last_sunday.isocalendar()
    return "%sW%d" % (year, weeknum)
