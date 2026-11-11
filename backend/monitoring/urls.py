from django.urls import path
from .views import PengukuranList, TrenPerJam

urlpatterns = [
    path("pengukuran/", PengukuranList.as_view()),
    path("tren/", TrenPerJam.as_view()),
]