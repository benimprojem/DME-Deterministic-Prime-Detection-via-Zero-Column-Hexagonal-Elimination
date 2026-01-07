import time
import os

def is_prime_miller_rabin(n):
    """Asallık testi için Miller-Rabin algoritması (Bağımsız Doğrulayıcı)"""
    if n < 2: return False
    if n == 2 or n == 3: return True
    if n % 2 == 0: return False
    
    # Küçük asallarla hızlı eleme
    for p in [3, 5, 7, 11, 13, 17, 19, 23]:
        if n == p: return True
        if n % p == 0: return False

    r, d = 0, n - 1
    while d % 2 == 0:
        r += 1
        d //= 2
    for a in [2, 3, 5, 7, 11, 13, 17, 19, 23]: # 10^15 için bu şahitler yeterlidir
        if n <= a: break
        x = pow(a, d, n)
        if x == 1 or x == n - 1:
            continue
        for _ in range(r - 1):
            x = pow(x, 2, n)
            if x == n - 1:
                break
        else:
            return False
    return True

def dme_collision_analysis():
    # --- AYARLAR ---
    start_n = 10**15
    scan_range = 10000
    end_n = start_n + scan_range
    dogrula = True  # OPSİYONEL DOĞRULAMA: Hız için False yapılabilir.
    
    print(f"--- DME ÇALIŞMA VE DOĞRULUK ANALİZİ ---")
    print(f"Hedef Bölge: {start_n} - {end_n}\n")

    # Başlangıçta tüm adaylar saf (True)
    sieve = {num: True for num in range(start_n, end_n)}
    
    # İsabet takip sözlüğü
    hit_counts = {num: 0 for num in range(start_n, end_n)}
    
    # Modül Havuzu (Sabit Karekök Limiti)
    limit = int(end_n**0.5)
    modules = []
    temp_sieve = [True] * (limit + 1)
    for p in range(2, limit + 1):
        if temp_sieve[p]:
            modules.append(p)
            for i in range(p*p, limit + 1, p):
                temp_sieve[i] = False

    start_time = time.time()
    total_commands = 0

    # DME İMHA DÖNGÜSÜ
    for p in modules:
        # Direct Offset Formülü
        first_match = start_n + (p - (start_n % p)) % p
        if first_match < start_n: first_match += p
        if first_match == p: first_match += p
        
        for multiple in range(first_match, end_n, p):
            if multiple in sieve:
                total_commands += 1
                hit_counts[multiple] += 1
                sieve[multiple] = False

    end_time = time.time()

    # İSTATİSTİKSEL ANALİZ
    primes = [n for n, pure in sieve.items() if pure]
    deleted_numbers = [n for n, pure in sieve.items() if not pure]
    
    # İkiz Asallar
    twins = [(primes[i], primes[i+1]) for i in range(len(primes)-1) if primes[i+1] - primes[i] == 2]

    # OTOMATİK DOĞRULAMA İŞLEMİ
    dogruluk_orani = 100.00
    if dogrula:
        print("DME Sonuçları Doğrulanıyor (Miller-Rabin)...")
        dogru_asallar = sum(1 for p in primes if is_prime_miller_rabin(p))
        if len(primes) > 0:
            dogruluk_orani = (dogru_asallar / len(primes)) * 100

    # DOSYA YAZMA (Kısa Format: DME_Q15_10k.txt)
    file_name = f"DME_Q{len(str(start_n))-1}_{scan_range//1000}k.txt"
    with open(file_name, "w") as f:
        f.write(f"DME PURITY REPORT | Range: {start_n} - {end_n}\n")
        f.write("-" * 40 + "\n")
        for p in primes:
            f.write(str(p) + "\n")

    # İsabet Dağılımı
    collision_stats = {}
    for n in deleted_numbers:
        c = hit_counts[n]
        collision_stats[c] = collision_stats.get(c, 0) + 1

    # RAPORLAMA
    print("="*45)
    print("DME VERİMLİLİK VE ÇAKIŞMA RAPORU")
    print("="*45)
    print(f"Toplam İmha Komutu (Ateşleme) : {total_commands}")
    print(f"Benzersiz Silinen Sayı         : {len(deleted_numbers)}")
    print(f"Tespit Edilen Saf Asal         : {len(primes)}")
    print(f"Tespit Edilen İkiz Asal        : {len(twins)}")
    print(f"Veri Dosyası Oluşturuldu       : {file_name}")
    print("-" * 45)
    
    print("İSABET DAĞILIMI:")
    for hits in sorted(collision_stats.keys()):
        print(f" {hits} kez vurulan: {collision_stats[hits]} sayı")

    print("-" * 45)
    print(f"İşlem Süresi                   : {end_time - start_time:.2f} sn")
    print(f"Doğruluk Payı (Onaylı)         : %{dogruluk_orani:.2f}")
    print("="*45)
    
    # KAPANMAYI ENGELLEYEN KOMUT
    print("\nAnaliz tamamlandı. Dosya kaydedildi.")
    input("Pencereyi kapatmak için ENTER tuşuna basın...")

if __name__ == "__main__":
    try:
        dme_collision_analysis()
    except Exception as e:
        print(f"Bir hata oluştu: {e}")
        input("Hatayı incelemek için ENTER'a basın...")