from django.conf.urls import include, url
from django.contrib import admin
from django.views.generic import TemplateView
from users.views import createuser
from mainapp.views import pong, login, echo, input_

urlpatterns = [
    url(r'^admin/', admin.site.urls),
    url(r'^djangojs/', include('djangojs.urls')),
    url(r'^ping$', pong),
    url(r'^login$', login),
    url(r'^createuser$', createuser),
    url(r'^echo$', echo),
    url(r'^input$', input_),
    url(r'^$', TemplateView.as_view(template_name='mainapp/itworks.html'), name='home'),
    url(r'^.*/$',  TemplateView.as_view(template_name='mainapp/error.html'), name='error'),
]
