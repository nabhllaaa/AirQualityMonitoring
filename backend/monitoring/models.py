from django.db import models
from django.utils import timezone


class AirrespiraLog(models.Model):
    id = models.BigAutoField(primary_key=True)
    created_at = models.DateTimeField(auto_now_add=True)
    timestamp_wib = models.CharField(max_length=50, null=True, blank=True)
    latitude = models.FloatField(null=True, blank=True)
    longitude = models.FloatField(null=True, blank=True)
    satellites = models.IntegerField(null=True, blank=True)
    temperature = models.FloatField(null=True, blank=True)
    humidity = models.FloatField(null=True, blank=True)
    vo_volt = models.FloatField(null=True, blank=True)
    pm25 = models.FloatField(null=True, blank=True)

    class Meta:
        db_table = "airrespira_logs"
        managed = False

    def save(self, *args, **kwargs):
        if not self.timestamp_wib:
            self.timestamp_wib = timezone.localtime().strftime("%Y-%m-%d %H:%M:%S")
        super().save(*args, **kwargs)