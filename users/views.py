from django.shortcuts import render
from django.http import HttpResponse, HttpResponseNotAllowed, HttpResponseBadRequest, JsonResponse
from django.views.decorators.csrf import csrf_exempt
from users.models import Profile

@csrf_exempt
def create_user(request):
    if request.method != 'POST':
        return HttpResponseNotAllowed(['POST'])
    form = {k: v for k, v in request.POST.items()}
    try:
        profile, created = Profile.objects.update_or_create(
            username=form.pop('username').lower(),
            # password=request.POST.pop('password'),
            uuid=int(form.pop('uuid')),
            defaults=form)
    except:
        return HttpResponseBadRequest()
    profile.save()
    return HttpResponse('Created profile: {}'.format(profile))

@csrf_exempt
def delete_user(request):
    if request.method != 'POST':
        return HttpResponseNotAllowed(['POST'])
    form = {k: v for k, v in request.POST.items()}
    if 'uuid' in form:
        form['uuid'] = int(form['uuid'])
    try:
        profile = Profile.objects.get(**form)
    except:
        return HttpResponseBadRequest()
    profile.delete()
    return HttpResponse('Deleted profile: {}'.format(profile))