#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

// --- MODULAR ARITHMETIC FOR MILLER-RABIN ---
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

// Miller-Rabin Primality Test (Independent Validator)
bool is_prime_miller_rabin(uint64_t n) {
    if (n < 2) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0) return false;

    uint64_t d = n - 1;
    int r = 0;
    while (d % 2 == 0) {
        d /= 2;
        r++;
    }

    // Bases for 64-bit integers
    uint64_t bases[] = {2, 3, 5, 7, 11, 13, 17, 19, 23};
    for (int i = 0; i < 9; i++) {
        uint64_t a = bases[i];
        if (n <= a) break;
        uint64_t x = mod_pow(a, d, n);
        if (x == 1 || x == n - 1) continue;
        bool composite = true;
        for (int j = 0; j < r - 1; j++) {
            x = mod_mul(x, x, n);
            if (x == n - 1) {
                composite = false;
                break;
            }
        }
        if (composite) return false;
    }
    return true;
}

int main() {
    // --- SETTINGS ---
    uint64_t start_n = 1000000000000000ULL;
    uint64_t scan_range = 10000;
    uint64_t end_n = start_n + scan_range;

    printf("--- DME MODULAR COLLISION & PURITY ANALYSIS (C CORE) ---\n");
    printf("Target Range: %llu - %llu\n\n", start_n, end_n);

    clock_t start_time = clock();

    // Memory Allocation
    bool *sieve = (bool *)malloc(scan_range * sizeof(bool));
    uint32_t *hit_counts = (uint32_t *)calloc(scan_range, sizeof(uint32_t));
    for (uint64_t i = 0; i < scan_range; i++) sieve[i] = true;

    // 1. Generate Module Pool (Sieve up to sqrt)
    uint64_t limit = (uint64_t)sqrt((double)end_n);
    bool *temp_sieve = (bool *)malloc((limit + 1) * sizeof(bool));
    for (uint64_t i = 0; i <= limit; i++) temp_sieve[i] = true;

    uint64_t total_commands = 0;
    for (uint64_t p = 2; p <= limit; p++) {
        if (temp_sieve[p]) {
            // DME Direct Offset Elimination
            uint64_t first_match = (p - (start_n % p)) % p;
            if (start_n + first_match == p) first_match += p;

            for (uint64_t curr = first_match; curr < scan_range; curr += p) {
                sieve[curr] = false;
                hit_counts[curr]++;
                total_commands++;
            }
            // Update module sieve
            for (uint64_t i = p * p; i <= limit; i += p) temp_sieve[i] = false;
        }
    }

    double duration = (double)(clock() - start_time) / CLOCKS_PER_SEC;

    // 2. Statistics & Reporting
    uint64_t prime_count = 0;
    uint64_t deleted_count = 0;
    uint32_t max_hits = 0;
    uint64_t twin_count = 0;
    uint64_t *primes_found = (uint64_t *)malloc(scan_range * sizeof(uint64_t));

    for (uint64_t i = 0; i < scan_range; i++) {
        if (sieve[i]) {
            primes_found[prime_count++] = start_n + i;
        } else {
            deleted_count++;
            if (hit_counts[i] > max_hits) max_hits = hit_counts[i];
        }
    }

    // Twin Detection
    for (uint64_t i = 0; i < prime_count - 1; i++) {
        if (primes_found[i+1] - primes_found[i] == 2) twin_count++;
    }

    // Accuracy Validation
    uint64_t valid_primes = 0;
    for (uint64_t i = 0; i < prime_count; i++) {
        if (is_prime_miller_rabin(primes_found[i])) valid_primes++;
    }
    double accuracy = (prime_count > 0) ? ((double)valid_primes / prime_count) * 100.0 : 100.0;

    // Output
    printf("=============================================\n");
    printf("DME PERFORMANCE & PURITY REPORT (C/GCC)\n");
    printf("=============================================\n");
    printf("Total Elimination Commands : %llu\n", total_commands);
    printf("Unique Deleted Numbers     : %llu\n", deleted_count);
    printf("Detected Pure Primes       : %llu\n", prime_count);
    printf("Detected Twin Pairs        : %llu\n", twin_count);
    printf("---------------------------------------------\n");
    printf("Processing Time            : %.6f sec\n", duration);
    printf("Accuracy Rate (Verified)   : %.2f\n", accuracy);
    printf("=============================================\n");

    // Cleanup
    free(sieve); free(hit_counts); free(temp_sieve); free(primes_found);
    return 0;
}