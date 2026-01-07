#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

// --- MODÜLER ARİTMETİK ---
uint64_t mod_mul(uint64_t a, uint64_t b, uint64_t m) {
    return (__uint128_t)a * b % m;
}

uint64_t mod_pow(uint64_t base, uint64_t exp, uint64_t m) {
    uint64_t res = 1;
    base %= m;
    while (exp > 0) {
        if (exp % 2 == 1) res = mod_mul(res, base, m);
        base = mod_mul(base, base, m);
        exp /= 2;
    }
    return res;
}

// Miller-Rabin Doğrulayıcı
bool is_prime_miller_rabin(uint64_t n) {
    if (n < 2) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0) return false;
    uint64_t d = n - 1;
    int r = 0;
    while (d % 2 == 0) { d /= 2; r++; }
    uint64_t bases[] = {2, 3, 5, 7, 11, 13, 17, 19, 23};
    for (int i = 0; i < 9; i++) {
        uint64_t a = bases[i];
        if (n <= a) break;
        uint64_t x = mod_pow(a, d, n);
        if (x == 1 || x == n - 1) continue;
        bool composite = true;
        for (int j = 0; j < r - 1; j++) {
            x = mod_mul(x, x, n);
            if (x == n - 1) { composite = false; break; }
        }
        if (composite) return false;
    }
    return true;
}

int main() {
    uint64_t start_n = 10000000000000000ULL;
    uint64_t scan_range = 100000;
    uint64_t end_n = start_n + scan_range;

    printf("--- DME MULTI-FILE OUTPUT ENGINE ---\n");
    clock_t t_start = clock();

    bool *sieve = malloc(scan_range * sizeof(bool));
    for (uint64_t i = 0; i < scan_range; i++) sieve[i] = true;

    uint64_t limit = (uint64_t)sqrt((double)end_n);
    bool *temp_sieve = malloc((limit + 1) * sizeof(bool));
    for (uint64_t i = 0; i <= limit; i++) temp_sieve[i] = true;

    for (uint64_t p = 2; p <= limit; p++) {
        if (temp_sieve[p]) {
            uint64_t first_match = (p - (start_n % p)) % p;
            if (start_n + first_match == p) first_match += p;
            for (uint64_t curr = first_match; curr < scan_range; curr += p) sieve[curr] = false;
            for (uint64_t i = p * p; i <= limit; i += p) temp_sieve[i] = false;
        }
    }

    // --- DOSYALARA YAZDIRMA ---
    FILE *f_primes = fopen("primes.txt", "w");
    FILE *f_twins = fopen("twins.txt", "w");
    
    uint64_t prime_count = 0, twin_count = 0;
    uint64_t *primes_arr = malloc(scan_range * sizeof(uint64_t));

    for (uint64_t i = 0; i < scan_range; i++) {
        if (sieve[i]) {
            uint64_t p_val = start_n + i;
            primes_arr[prime_count++] = p_val;
            fprintf(f_primes, "%llu\n", p_val);
        }
    }

    for (uint64_t i = 0; i < prime_count - 1; i++) {
        if (primes_arr[i+1] - primes_arr[i] == 2) {
            fprintf(f_twins, "%llu - %llu\n", primes_arr[i], primes_arr[i+1]);
            twin_count++;
        }
    }

    fclose(f_primes);
    fclose(f_twins);
    double duration = (double)(clock() - t_start) / CLOCKS_PER_SEC;

    printf("Execution Success!\n");
    printf("Primes saved to: primes.txt (%llu found)\n", prime_count);
    printf("Twins saved to : twins.txt (%llu pairs found)\n", twin_count);
    printf("Time: %.3f sec\n", duration);

    free(sieve); free(temp_sieve); free(primes_arr);
    return 0;
}