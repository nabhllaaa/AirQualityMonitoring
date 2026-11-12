from django.test import SimpleTestCase, override_settings
from rest_framework.test import APIRequestFactory

from .ispu import hitung_ispu
from .views import HasDeviceKey


class IspuTest(SimpleTestCase):
    def test_kategori_pada_nilai_batas(self):
        # Nilai batas yang sama dengan skenario uji klasifikasi di proposal
        kasus = [
            (15.5, "Baik"),
            (15.6, "Sedang"),
            (55.4, "Sedang"),
            (55.5, "Tidak Sehat"),
            (150.4, "Tidak Sehat"),
            (150.5, "Sangat Tidak Sehat"),
            (250.4, "Sangat Tidak Sehat"),
            (250.5, "Berbahaya"),
        ]
        for pm25, harapan in kasus:
            with self.subTest(pm25=pm25):
                self.assertEqual(hitung_ispu(pm25)[1], harapan)

    def test_nilai_ispu(self):
        self.assertEqual(hitung_ispu(0)[0], 0)
        self.assertEqual(hitung_ispu(15.5)[0], 50)
        self.assertEqual(hitung_ispu(55.4)[0], 100)
        self.assertEqual(hitung_ispu(162.7)[0], 213)  # 200 + 12,3 = 212,3 dibulatkan ke atas

    def test_pm25_kosong(self):
        self.assertEqual(hitung_ispu(None), (None, None))

    def test_pm25_negatif_diperlakukan_nol(self):
        self.assertEqual(hitung_ispu(-5), (0, "Baik"))


@override_settings(DEVICE_API_KEY="kunci-tes")
class IzinPerangkatTest(SimpleTestCase):
    def setUp(self):
        self.factory = APIRequestFactory()

    def test_get_terbuka_tanpa_key(self):
        request = self.factory.get("/api/pengukuran/")
        self.assertTrue(HasDeviceKey().has_permission(request, None))

    def test_post_tanpa_key_ditolak(self):
        request = self.factory.post("/api/pengukuran/", {})
        self.assertFalse(HasDeviceKey().has_permission(request, None))

    def test_post_key_salah_ditolak(self):
        request = self.factory.post("/api/pengukuran/", {}, HTTP_X_API_KEY="salah")
        self.assertFalse(HasDeviceKey().has_permission(request, None))

    def test_post_key_benar_diterima(self):
        request = self.factory.post("/api/pengukuran/", {}, HTTP_X_API_KEY="kunci-tes")
        self.assertTrue(HasDeviceKey().has_permission(request, None))