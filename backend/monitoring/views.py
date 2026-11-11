from django.conf import settings
from rest_framework import generics
from rest_framework.permissions import BasePermission, SAFE_METHODS

from .models import Pengukuran
from .serializers import PengukuranSerializer


class HasDeviceKey(BasePermission):
    def has_permission(self, request, view):
        if request.method in SAFE_METHODS:
            return True
        return request.headers.get("X-API-Key") == settings.DEVICE_API_KEY


class PengukuranList(generics.ListCreateAPIView):
    serializer_class = PengukuranSerializer
    permission_classes = [HasDeviceKey]

    def get_queryset(self):
        qs = Pengukuran.objects.order_by("-timestamp")
        tanggal = self.request.query_params.get("tanggal")
        if tanggal:
            return qs.filter(tanggal=tanggal)
        return qs[:500]