from django.db import models
from users.models import Profile


class Board(models.Model):
    KINDS_OF_BOARDS = (
        "FRIDGE",
        "OVEN"
    )

    kind = models.CharField(max_length=255)
    uid = models.PositiveSmallIntegerField()  # NOTE: should be unique per Profile ForeignKey
    profile = models.ForeignKey(Profile, on_delete=models.CASCADE)

    class Meta:
        unique_together = (('uid', 'profile'),)  

    def __str__(self):
        return '{} {}'.format(self.uid, self.kind)


class Sensor(models.Model):
    KINDS_OF_SENSORS = (
        'TEMP',
        'PRES',
        'REED',
        'BATT',
    )

    PARAM_TO_INDEX = {
        's0': 0,
        's1': 1,
        's2': 2,
        's3': 3,
        's4': 4,
        's5': 5,
        's6': 6,
        's7': 7,
        's8': 8,
        's9': 9,
        'batt': 10,
    }

    kind = models.CharField(max_length=255)
    index = models.PositiveSmallIntegerField()  # NOTE: should be unique per Profile ForeignKey
    board = models.ForeignKey(Board, on_delete=models.CASCADE)

    class Meta:
        unique_together = (('index', 'board'),)  

    def __str__(self):
        return '{} {}'.format(self.index, self.kind)

class RawData(models.Model):
    NOT_AVAILABLE = -1

    created = models.DateTimeField(auto_now_add=True, db_index=True)
    sensor = models.ForeignKey(Sensor, related_name='rawdata', on_delete=models.CASCADE)
    datum = models.FloatField(default=NOT_AVAILABLE)

    def __str__(self):
        return '{} {}'.format(created, datum)
