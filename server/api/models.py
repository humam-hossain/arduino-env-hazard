from django.db import models

# Create your models here.
class SensorData(models.Model):
    timestamp = models.DateTimeField(auto_now_add=True)
    t = models.FloatField(null=True)
    samples = models.IntegerField(null=True)
    r_25um = models.FloatField(null=True)
    ugm3_25um = models.FloatField(null=True)
    pcs_25um = models.FloatField(null=True)
    r_1um = models.FloatField(null=True)
    ugm3_1um = models.FloatField(null=True)
    pcs_1um = models.FloatField(null=True)
    temp = models.FloatField(null=True)
    humidity = models.FloatField(null=True)
    flame = models.FloatField(null=True)