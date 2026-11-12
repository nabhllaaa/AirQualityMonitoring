from datetime import timedelta

from django.conf import settings
from django.db.models import Avg, Count, Max
from django.db.models.functions import TruncHour
from django.utils import timezone
from rest_framework import generics
from rest_framework.permissions import BasePermission, SAFE_METHODS
from rest_framework.response import Response
from rest_framework.views import APIView

from .models import AirrespiraLog
from .serializers import AirrespiraLogSerializer


class HasDeviceKey(BasePermission):
    def has_permission(self, request, view):
        if request.method in SAFE_METHODS:
            return True
        return request.headers.get("X-API-Key") == settings.DEVICE_API_KEY


class PengukuranList(generics.ListCreateAPIView):
    serializer_class = AirrespiraLogSerializer
    permission_classes = [HasDeviceKey]

    def get_queryset(self):
        qs = AirrespiraLog.objects.order_by("-created_at")
        tanggal = self.request.query_params.get("tanggal")
        if tanggal:
            return qs.filter(created_at__date=tanggal)
        return qs[:500]


class TrenPerJam(APIView):
    def get(self, request):
        try:
            jam = int(request.query_params.get("jam", 24))
        except ValueError:
            jam = 24
        jam = min(max(jam, 1), 168)  # batasi 1 jam sampai 7 hari

        batas = timezone.now() - timedelta(hours=jam)
        data = (
            AirrespiraLog.objects.filter(created_at__gte=batas)
            .annotate(jam=TruncHour("created_at"))
            .values("jam")
            .annotate(
                pm25_rata=Avg("pm25"),
                pm25_maks=Max("pm25"),
                suhu_rata=Avg("temperature"),
                kelembapan_rata=Avg("humidity"),
                jumlah_data=Count("id"),
            )
            .order_by("jam")
        )
        return Response(list(data))