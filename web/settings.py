# -*- coding: utf-8 -*-

"""Default options for the application.
"""

DEBUG = False

SESSION_TIMEOUT = 3600  # 1 Hour

HASH_KEY = ''
VALIDATE_KEY = ''
ENCRYPT_KEY = ''
SECRET_KEY = ''

PAGE_LIMIT = 25


def absolute(path):
    """Get the absolute path of the given file/folder.

    ``path``: File or folder.
    """
    import os
    PROJECT_DIR = os.path.dirname(os.path.abspath(__file__))
    return os.path.normpath(os.path.join(PROJECT_DIR, path))

config = {
    'db_name': 'dispatcher2',
    'db_host': 'localhost',
    'db_user': '',
    'db_passwd': '',
    'db_port': '5432',
    'logfile': '/tmp/dispatcher2-web.log',
}

try:
    from local_settings import *
except ImportError:
    pass
