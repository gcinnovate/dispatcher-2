# -*- coding: utf-8 -*-

"""URL definitions of the application. Regex based URLs are mapped to their
class handlers.
"""

from app.controllers.main_handler import Index, Logout
from app.controllers.api import RequestDetails, DeleteServer, DeleteRequest, ServerDetails
from app.controllers.users_handler import Users
from app.controllers.groups_handler import Groups
from app.controllers.dashboard_handler import Dashboard
from app.controllers.auditlog_handler import AuditLog
from app.controllers.requests_handler import Requests
from app.controllers.completed_handler import Completed
from app.controllers.failed_handler import Failed
from app.controllers.ready_handler import Ready
from app.controllers.search_handler import Search
from app.controllers.appsettings_handler import AppSettings
from app.controllers.settings_handler import Settings
from app.controllers.forgotpass_handler import ForgotPass
from app.controllers.downloads_handler import Downloads

URLS = (
    r'^/', Index,
    r'/downloads', Downloads,
    r'/auditlog', AuditLog,
    r'/settings', Settings,
    r'/dashboard', Dashboard,
    r'/users', Users,
    r'/groups', Groups,
    r'/logout', Logout,
    r'/forgotpass', ForgotPass,
    # Dispatcher2 URIs
    r'/requests', Requests,
    r'/completed', Completed,
    r'/failed', Failed,
    r'/search', Search,
    r'/ready', Ready,
    r'/appsettings', AppSettings,
    # API stuff follows
    r'/api/v1/request_details/(\d+)/?', RequestDetails,
    r'/api/v1/server_details/(\d+)/?', ServerDetails,
    r'/api/v1/server_del/(\d+)/?', DeleteServer,
    r'/api/v1/request_del/(\d+)/?', DeleteRequest,
)
