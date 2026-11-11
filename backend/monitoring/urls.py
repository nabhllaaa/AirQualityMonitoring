from django.urls import path
from .views import PengukuranList

urlpatterns = [
    path("pengukuran/", PengukuranList.as_view()),
]