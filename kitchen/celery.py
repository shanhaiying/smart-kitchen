# coding: utf-8

from __future__ import absolute_import

import os

from django.conf import settings

from celery import Celery

from celery.utils.log import get_task_logger
logger = get_task_logger(__name__)

os.environ.setdefault("DJANGO_SETTINGS_MODULE", "kitchen.settings.local")

app = Celery('kitchen_tasks')
app.config_from_object('django.conf:settings', namespace='CELERY')
app.autodiscover_tasks(lambda: settings.INSTALLED_APPS)
