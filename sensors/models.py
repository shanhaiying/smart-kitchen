from datetime import timedelta
from django.db import models
from timeseries.utils import TimeSeriesModel, TimeSeriesManager

class Sensor(models.Model):
    TEMPERATURE = 'TEMP'
    PRESSURE    = 'PRES'
    REED        = 'REED'
    BATTERY     = 'BATT'
    KINDS_OF_SENSORS = (
        (TEMPERATURE, 'Temperature'),
        (PRESSURE, 'Pressure'),
        (REED, 'Reed'),
        (BATTERY, 'Batter'),
    )
    kind = models.CharField(max_length=255, choices=KINDS_OF_SENSORS)
    uid = models.IntegerField()

    objects = TimeSeriesManager()

    def __str__(self):
        return '{} {}'.format(self.uid, self.kind)

class RawData(TimeSeriesModel):

    TIMESERIES_INTERVAL = timedelta(seconds=1)

    NOT_AVAILABLE = -1

    sensor = models.ForeignKey(Sensor, related_name='rawdata')
    datum = models.FloatField(default=NOT_AVAILABLE)
