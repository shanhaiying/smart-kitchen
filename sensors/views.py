from django.shortcuts import render
from django.http import HttpResponse, HttpResponseNotAllowed, HttpResponseBadRequest, JsonResponse
from django.views.decorators.csrf import csrf_exempt
from users.models import Profile
from sensors.models import Board, Sensor, RawData
from django.core.exceptions import ObjectDoesNotExist

class FormError(KeyError):
    pass


@csrf_exempt
def input_data(request):
    if request.method != 'POST':
        return HttpResponseNotAllowed(['POST'])
    form = {k: v for k, v in request.POST.items()}
    try:
        datum = {}
        datum['profile'] = form.pop('uuid')
        datum['board'] = form.pop('board_id')
        for param, value in form.items():
            datum['index'] = Sensor.PARAM_TO_INDEX[value]
            create_datum(datum)
    except FormError as e:
        return HttpResponseBadRequest()
    except KeyError:
        return HttpResponseBadRequest('Missing some form data')
    except:
        return HttpResponseBadRequest("Invalid format, talk to Michael")


def create_datum(datum):
    try:
        profile = Profile.objects.get(uuid=int(datum.pop('profile')))
        board = Board.objects.get(profile=profile, uid=int(datum.pop('board')))
        sensor = Sensor.objects.get(board=board, index=int(datum.pop('index')))
    except:
        raise FormError("Invalid datum: missing profile, board, or index (as int)")
    try:
        raw_data, created = RawData.objects.update_or_create(sensor=sensor,
                                                             datum=float(datum.pop('value')),
                                                             **datum)
    except:
        raise FormError('Invalid form, need "data" as float or None')
    raw_data.save()


@csrf_exempt
def create_board(request):
    if request.method != 'POST':
        return HttpResponseNotAllowed(['POST'])
    form = {k: v for k, v in request.POST.items()}
    try:
        profile = Profile.objects.get(uuid=int(form.pop('profile')))
    except:
        return HttpResponseBadRequest("Invalid form: missing profile or board (as int)")
    if form.get('kind') not in Board.KINDS_OF_BOARDS:
        return HttpResponseBadRequest('Invalid sensor type {}'.format(form['kind']))
    try:
        board, created = Board.objects.update_or_create(profile=profile,
                                                        uid=int(form.pop('uid')),
                                                        **form)
    except Exception as e:
        print(e)
        return HttpResponseBadRequest('Invalid form, need "uid", "kind"')
    board.save()
    return HttpResponse('Created board: {}'.format(board))


@csrf_exempt
def delete_board(request):
    if request.method != 'POST':
        return HttpResponseNotAllowed(['POST'])
    form = {k: v for k, v in request.POST.items()}
    try:
        profile = Profile.objects.get(uuid=int(form.pop('profile')))
    except:
        return HttpResponseBadRequest("Invalid form: missing profile or board (as int)")
    try:
        board = Board.objects.get(profile=profile,
                                  uid=int(form.pop('uid')),
                                  **form)
    except:
        return HttpResponseBadRequest('Invalid form, need "uid" as int')
    board.save()
    return HttpResponse('Created board: {}'.format(board))


@csrf_exempt
def create_sensor(request):
    if request.method != 'POST':
        return HttpResponseNotAllowed(['POST'])
    form = {k: v for k, v in request.POST.items()}
    try:
        profile = Profile.objects.get(uuid=int(form.pop('profile')))
        board = Board.objects.get(profile=profile, uid=int(form.pop('board')))
    except:
        return HttpResponseBadRequest("Invalid form: missing profile or board (as int)")
    if form.get('kind') not in Sensor.KINDS_OF_SENSORS:
        return HttpResponseBadRequest('Invalid sensor type {}'.format(form['kind']))
    try:
        sensor, created = Sensor.objects.update_or_create(board=board,
                                                          index=int(form.pop('index')),
                                                          **form)
    except:
        return HttpResponseBadRequest('Invalid form, need "index", "kind"')
    sensor.save()
    return HttpResponse('Created sensor: {}'.format(sensor))


@csrf_exempt
def delete_sensor(request):
    if request.method != 'POST':
        return HttpResponseNotAllowed(['POST'])
    form = {k: v for k, v in request.POST.items()}
    try:
        profile = Profile.objects.get(uuid=int(form.pop('profile')))
        board = Board.objects.get(profile=profile, uid=int(form.pop('board')))
    except:
        return HttpResponseBadRequest("Invalid form: missing profile or board (as int)")
    try:
        sensor = Sensor.objects.get(board=profile,
                                    index=int(form.pop('index')),
                                    **form)
    except:
        return HttpResponseBadRequest('Invalid form, need "index" as int')
    sensor.delete()
    return HttpResponse('Deleted sensor: {}'.format(sensor))
