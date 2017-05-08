from django.contrib.auth.models import AbstractBaseUser, PermissionsMixin
from django.db import models
from django.utils.translation import ugettext_lazy as _

from common.models import IndexedTimeStampedModel

from .managers import UserManager


class Datum(models.Model):
    value = models.DecimalField(decimal_places=2, max_digits=10)
    timestamp = models.DateTimeField()


class Sensor(models.Model):
    kind = models.CharField(max_length=20, default='any')
    uid = models.IntegerField()
    data = models.ManyToManyField(Datum)


class User(AbstractBaseUser, PermissionsMixin, IndexedTimeStampedModel):
    uid = models.IntegerField(unique=True, default=0)
    sensors = models.ManyToManyField(Sensor)

    email = models.EmailField(max_length=255, unique=True)
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

    def get_full_name(self):
        return self.email

    def get_short_name(self):
        return self.email

    def __str__(self):
        return 'User: {}; email: {}'.format(self.uid, self.email)