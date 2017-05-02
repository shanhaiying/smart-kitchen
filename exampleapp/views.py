from django.shortcuts import render  # noqa
from django.http import HttpResponse

# Create your views here.
def xbee_ping(request):
	print('PING')
	return HttpResponse('PONG')
