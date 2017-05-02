from django.shortcuts import render  # noqa
from django.http import HttpResponse
from users.models import User


# Create your views here.
# def get_all_users(request):
# 	print(request.method)
# 	return HttpResponse(', '.join(map(str, User.objects.all())))


def get_all_users(request):
	if request.method == 'GET':
		if 'email' in request.GET:
			return HttpResponse(', '.join(map(str, User.objects.filter(email__contains=request.GET['email']))))
		return HttpResponse(', '.join(map(str, User.objects.all())))
