from django.contrib.auth.models import User
from django.db import models

from sensors.models import Sensor

class Profile(models.Model):
    user = models.OneToOneField(User, on_delete=models.CASCADE)
    uuid = models.IntegerField(unique=True, default=0)
    sensors = models.ManyToManyField(Sensor)
