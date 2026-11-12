from datetime import date, timedelta

from django.conf import settings
from django.db.models import Avg, Count, Max, Min
from django.db.models.functions import TruncHour
from django.utils import timezone
from rest_framework import generics
from rest_framework.permissions import BasePermission, SAFE_METHODS
from rest_framework.response import Response
from rest_framework.views import APIView

from .ispu import hitung_ispu
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

        hasil = list(data)
        for baris in hasil:
            baris["ispu"], baris["kategori"] = hitung_ispu(baris["pm25_rata"])
        return Response(hasil)


class PetaGeoJSON(APIView):
    def get(self, request):
        try:
            jam = int(request.query_params.get("jam", 24))
        except ValueError:
            jam = 24
        jam = min(max(jam, 1), 168)

        batas = timezone.now() - timedelta(hours=jam)
        data = (
            AirrespiraLog.objects.filter(
                created_at__gte=batas,
                latitude__isnull=False,
                longitude__isnull=False,
                pm25__isnull=False,
            )
            .exclude(latitude=0, longitude=0)
            .order_by("-created_at")[:1000]
        )

        fitur = []
        for d in data:
            ispu, kategori = hitung_ispu(d.pm25)
            fitur.append({
                "type": "Feature",
                "geometry": {"type": "Point", "coordinates": [d.longitude, d.latitude]},
                "properties": {
                    "id": d.id,
                    "waktu": timezone.localtime(d.created_at).isoformat(),
                    "pm25": d.pm25,
                    "ispu": ispu,
                    "kategori": kategori,
                },
            })
        return Response({"type": "FeatureCollection", "features": fitur})


class DataTerkini(APIView):
    def get(self, request):
        terakhir = AirrespiraLog.objects.order_by("-created_at").first()
        if terakhir is None:
            return Response({"ada_data": False})
        umur = (timezone.now() - terakhir.created_at).total_seconds()
        return Response({
            "ada_data": True,
            "alat_aktif": umur < 120,
            "umur_detik": round(umur),
            "data": AirrespiraLogSerializer(terakhir).data,
        })


class RingkasanHarian(APIView):
    def get(self, request):
        tanggal = request.query_params.get("tanggal") or timezone.localdate().isoformat()
        try:
            date.fromisoformat(tanggal)
        except ValueError:
            return Response({"detail": "Format tanggal harus YYYY-MM-DD"}, status=400)

        qs = AirrespiraLog.objects.filter(created_at__date=tanggal, pm25__isnull=False)
        hasil = qs.aggregate(
            pm25_rata=Avg("pm25"),
            pm25_min=Min("pm25"),
            pm25_maks=Max("pm25"),
            suhu_rata=Avg("temperature"),
            kelembapan_rata=Avg("humidity"),
            jumlah_data=Count("id"),
        )
        if hasil["jumlah_data"] == 0:
            return Response({"tanggal": tanggal, "jumlah_data": 0})
        hasil["ispu"], hasil["kategori"] = hitung_ispu(hasil["pm25_rata"])
        hasil["tanggal"] = tanggal
        return Response(hasil)