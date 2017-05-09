from django.db import models

class Datum(models.Model):
    value = models.DecimalField(decimal_places=2, max_digits=10)
    timestamp = models.DateTimeField()


class Sensor(models.Model):
    kind = models.CharField(max_length=20, default='any')
    uid = models.IntegerField()
    data = models.ManyToManyField(Datum)
