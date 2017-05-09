from django.shortcuts import render
from django.http import HttpResponse, HttpResponseNotAllowed, HttpResponseBadRequest, JsonResponse
from django.views.decorators.csrf import csrf_exempt
from users.models import User


def createuser(request):
    if request.method != 'POST':
        return HttpResponseNotAllowed(['POST'])
    user = User.objects.createuser(request.POST.pop('username'), **request.POST)
    user.save()
    return 'Created user {}'.format(user)