#!/usr/bin/env python
# -*- coding: utf-8 -*-

__author__ = "Sekiwere Samuel"

# import web
# import psycopg2
# import psycopg2.extras
# import logging
import requests
import base64
# import json
# import datetime
# import time
# import re

conf = {
    'dispatcher2_queue_url': (
        "https://localhost:9090/queue?"
        "source=mtrack&destination=dhis2&msgid=10001&week=W33&year=2016")
    # "http://localhost:9090/queue?username=admin&password=admin&"
}
coded = base64.b64encode("admin:admin")
print coded


def post_request(data, url=conf['dispatcher2_queue_url'], ctype="xml"):
    response = requests.post(url, data=data, headers={
        'Content-Type': 'text/xml' if ctype == "xml" else 'application/json',
        'Authorization': 'Basic ' + coded}, verify=False)
    return response


# def post_request(data, url=conf['dispatcher2_queue_url'], ctype="xml"):
#     response = requests.post(url, data=data, auth=('admin', 'admin'), headers={
#         'Content-Type': 'text/xml' if ctype == "xml" else 'application/json'})
#     # 'Authorization': 'Basic ' + coded})
#     return response

with open('t3.xml', 'r') as f:
    s = f.read().strip()
    # print s
    r = post_request(s)
    print r.text
