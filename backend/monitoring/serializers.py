from rest_framework import serializers
from .ispu import hitung_ispu
from .models import AirrespiraLog


class AirrespiraLogSerializer(serializers.ModelSerializer):
    ispu = serializers.SerializerMethodField()
    kategori = serializers.SerializerMethodField()

    class Meta:
        model = AirrespiraLog
        fields = ["id", "created_at", "timestamp_wib", "latitude", "longitude",
                  "satellites", "temperature", "humidity", "vo_volt", "pm25",
                  "ispu", "kategori"]
        read_only_fields = ["id", "created_at"]
        extra_kwargs = {"pm25": {"required": True, "allow_null": False}}

    def get_ispu(self, obj):
        return hitung_ispu(obj.pm25)[0]

    def get_kategori(self, obj):
        return hitung_ispu(obj.pm25)[1]

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

    def validate_satellites(self, value):
        if value is not None and value < 0:
            raise serializers.ValidationError("satellites tidak boleh negatif")
        return value