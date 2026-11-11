from django.db import models
from django.utils import timezone


class Pengukuran(models.Model):
    timestamp = models.DateTimeField()
    tanggal = models.DateField(editable=False)
    longitude = models.FloatField(null=True, blank=True)
    latitude = models.FloatField(null=True, blank=True)
    pm25 = models.FloatField()
    suhu = models.FloatField(null=True, blank=True)
    kelembapan = models.FloatField(null=True, blank=True)

    class Meta:
        db_table = "tb_pengukuran"
        managed = False

    def save(self, *args, **kwargs):
        if self.timestamp is None:
            self.timestamp = timezone.now()
        self.tanggal = timezone.localtime(self.timestamp).date()
        super().save(*args, **kwargs)