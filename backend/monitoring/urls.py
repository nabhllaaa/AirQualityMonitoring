from django.urls import path
from .views import PengukuranList, TrenPerJam, PetaGeoJSON

urlpatterns = [
    path("pengukuran/", PengukuranList.as_view()),
    path("tren/", TrenPerJam.as_view()),
    path("peta/", PetaGeoJSON.as_view()),
]