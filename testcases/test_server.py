#!/usr/bin/python
# Author: Samuel Sekiwere <sekiskylink@gmail.com>

import os
import sys
import web
import urllib
import logging
# import requests
# from web.contrib.template import render_jinja

filedir = os.path.dirname(__file__)
sys.path.append(os.path.join(filedir))


class AppURLopener(urllib.FancyURLopener):
    version = "DHIS2-Dummy-Server  /1.0"

urllib._urlopener = AppURLopener()

logging.basicConfig(
    format='%(asctime)s:%(levelname)s:%(message)s', filename='/tmp/test_server.log',
    datefmt='%Y-%m-%d %I:%M:%S', level=logging.DEBUG
)
urls = (
    "/api/dataValueSets", "DummyResp",
)

app = web.application(urls, globals())

web.config.debug = False
# render = render_jinja(
#     'templates',
#     encoding='utf-8'
# )


class DummyResp:
    def GET(self):
        resp = ""
        web.header('Content-Type', 'text/xml')
        with open('resp.xml', 'r') as f:
            resp = f.read()
        return resp

    def POST(self):
        resp = ""
        web.header('Content-Type', 'text/xml')
        with open('resp.xml', 'r') as f:
            resp = f.read()
        return resp


if __name__ == "__main__":
    app.run()

# makes sure apache wsgi sees our app
application = web.application(urls, globals()).wsgifunc()
