from django.urls import path
from .views import (PengukuranList, TrenPerJam, PetaGeoJSON,
                    DataTerkini, RingkasanHarian)

urlpatterns = [
    path("pengukuran/", PengukuranList.as_view()),
    path("tren/", TrenPerJam.as_view()),
    path("peta/", PetaGeoJSON.as_view()),
    path("terkini/", DataTerkini.as_view()),
    path("ringkasan/", RingkasanHarian.as_view()),
]