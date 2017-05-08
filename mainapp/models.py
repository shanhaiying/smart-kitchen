from __future__ import unicode_literals

from django.db import models  # noqa


class Update(models.Model):
    last_modified = models.DateTimeField(auto_now=True)
    last_time_stamp = models.DateTimeField()
