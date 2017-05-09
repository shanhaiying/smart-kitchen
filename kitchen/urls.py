from django.conf.urls import include, url
from django.contrib import admin
from django.views.generic import TemplateView
from users.views import create_user, delete_user
from sensors.views import create_sensor, delete_sensor, create_board, delete_board, input_data
from mainapp.views import pong, login, echo

urlpatterns = [
    url(r'^admin/', admin.site.urls),
    url(r'^djangojs/', include('djangojs.urls')),
    url(r'^ping$', pong),
    url(r'^login$', login),
    url(r'^usercreate$', create_user),
    url(r'^userdelete$', delete_user),
    url(r'^sensorcreate$', create_sensor),
    url(r'^sensordelete$', delete_sensor),
    url(r'^boardcreate$', create_board),
    url(r'^boarddelete$', delete_board),
    url(r'^echo$', echo),
    url(r'^input$', input_data),
    url(r'^$', TemplateView.as_view(template_name='mainapp/itworks.html'), name='home'),
    url(r'^.*/$',  TemplateView.as_view(template_name='mainapp/error.html'), name='error'),
]
