# Batas konsentrasi PM2.5 (µg/m3) -> rentang ISPU, untuk interpolasi linear
BATAS_PM25 = [
    # (Xb,   Xa,    Ib,  Ia)
    (0.0,    15.5,   0,   50),
    (15.5,   55.4,  50,  100),
    (55.4,   150.4, 100, 200),
    (150.4,  250.4, 200, 300),
    (250.4,  500.0, 300, 500),
]


def kategori_ispu(nilai):
    if nilai <= 50:
        return "Baik"
    if nilai <= 100:
        return "Sedang"
    if nilai <= 200:
        return "Tidak Sehat"
    if nilai <= 300:
        return "Sangat Tidak Sehat"
    return "Berbahaya"


def hitung_ispu(pm25):
    """Mengembalikan (nilai ISPU, kategori). None jika pm25 kosong."""
    if pm25 is None:
        return None, None
    pm25 = max(pm25, 0)
    ispu = 500
    for xb, xa, ib, ia in BATAS_PM25:
        if pm25 <= xa:
            ispu = (ia - ib) / (xa - xb) * (pm25 - xb) + ib
            break
    ispu = round(ispu)
    return ispu, kategori_ispu(ispu)