from django.db import models

# Create your models here.
class SensorData(models.Model):
    timestamp = models.DateTimeField(auto_now_add=True)
    t = models.FloatField()
    samples = models.IntegerField()
    r_25um = models.FloatField()
    ugm3_25um = models.FloatField()
    pcs_25um = models.FloatField()
    r_1um = models.FloatField()
    ugm3_1um = models.FloatField()
    pcs_1um = models.FloatField()