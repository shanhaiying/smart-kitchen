from django.db import models
from django.contrib.auth.models import User
from django.db.models.signals import post_save
from django.dispatch import receiver


class Profile(models.Model):
    username = models.CharField(max_length=255)
    # password = models.CharField(max_length=255)
    uuid = models.PositiveSmallIntegerField(unique=True)
    first_name = models.CharField(max_length=255, blank=True)
    last_name = models.CharField(max_length=255, blank=True)

    def __str__(self):
        return '({}) {}'.format(self.uuid, self.username)
