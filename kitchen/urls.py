from django.conf.urls import include, url
from django.contrib import admin
from django.views.generic import TemplateView
from users.views import get_all_users
from mainapp.views import xbee_ping

urlpatterns = [
    url(r'^admin/', admin.site.urls),
    url(r'^djangojs/', include('djangojs.urls')),
    url(r'^users$', get_all_users),
    url(r'^xbee$', xbee_ping),
    url(r'^$', TemplateView.as_view(template_name='mainapp/itworks.html'), name='home'),
    url(r'^.*/$',  TemplateView.as_view(template_name='mainapp/error.html'), name='error'),
]
