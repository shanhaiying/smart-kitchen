from django.contrib.auth.models import AbstractBaseUser, PermissionsMixin
from django.db import models
from django.utils.translation import ugettext_lazy as _

from common.models import IndexedTimeStampedModel

from .managers import UserManager

from sensors.models import Sensor


class User(AbstractBaseUser, PermissionsMixin, IndexedTimeStampedModel):
    uuid = models.IntegerField(unique=True, default=0)
    name = models.CharField(max_length=255)
    email = models.EmailField(max_length=255, unique=True)
    sensors = models.ManyToManyField(Sensor)

    is_staff = models.BooleanField(
        default=False,
        help_text=_('Designates whether the user can log into this admin '
                    'site.'))
    is_active = models.BooleanField(
        default=True,
        help_text=_('Designates whether this user should be treated as '
                    'active. Unselect this instead of deleting accounts.'))

    objects = UserManager()
    USERNAME_FIELD = 'email'

    def __str__(self):
        return 'UUID: {}; name: {}; email: {}'.format(self.uid, self.name, self.email)
