from rest_framework import serializers
from .models import Pengukuran


class PengukuranSerializer(serializers.ModelSerializer):
    class Meta:
        model = Pengukuran
        fields = ["id", "timestamp", "tanggal", "latitude", "longitude",
                  "pm25", "suhu", "kelembapan"]
        read_only_fields = ["id", "tanggal"]
        extra_kwargs = {"timestamp": {"required": False}}

    def validate_pm25(self, value):
        if value < 0:
            raise serializers.ValidationError("pm25 tidak boleh negatif")
        return value

    def validate_latitude(self, value):
        if value is not None and not -90 <= value <= 90:
            raise serializers.ValidationError("latitude di luar rentang")
        return value

    def validate_longitude(self, value):
        if value is not None and not -180 <= value <= 180:
            raise serializers.ValidationError("longitude di luar rentang")
        return value