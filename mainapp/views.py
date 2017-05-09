import simplejson as json

from django.shortcuts import render  # noqa
from django.http import HttpResponse, HttpResponseNotAllowed, HttpResponseBadRequest, JsonResponse
from django.views.decorators.csrf import csrf_exempt

def input_(request):
    pass

def pong(request):
    print('Recieved: PING')
    return HttpResponse('PONG')

def login(request):
    if request.method != 'POST':
        return HttpResponseNotAllowed(['POST'])
    try:
        username = request.POST['username']
        password = request.POST['password']
    except ValueError:
        return HttpResponseBadRequest()
    try:
        go = eval(go)
        if go == True:
            return HttpResponse('SUCCESS')
        elif go == False:
            return HttpResponse('FAILURE')
        else:
            return HttpResponseBadRequest()
    except:
        return HttpResponseBadRequest()


@csrf_exempt
def echo(request):
    j = {'type': request.method}
    if request.method == 'GET':
        j['query'] = dict(request.GET)
    elif request.method == 'POST':
        j['form'] = dict(request.POST)

    return JsonResponse(j)
