use std::time::Instant;
use std::collections::BTreeMap;
use std::fs::File;
use std::io::Write;

// Bağımsız Doğrulayıcı: Miller-Rabin Primality Test
fn is_prime_miller_rabin(n: u64) -> bool {
    if n < 2 { return false; }
    if n == 2 || n == 3 { return true; }
    if n % 2 == 0 { return false; }

    let mut r = 0;
    let mut d = n - 1;
    while d % 2 == 0 {
        r += 1;
        d /= 2;
    }

    // 10^16'ya kadar olan sayılar için bu şahitler (bases) yeterlidir
    let bases = [2, 3, 5, 7, 11, 13, 17, 19, 23];
    for &a in bases.iter() {
        if n <= a { break; }
        let mut x = mod_pow(a, d, n);
        if x == 1 || x == n - 1 { continue; }
        let mut composite = true;
        for _ in 0..r - 1 {
            x = mod_pow(x, 2, n);
            if x == n - 1 {
                composite = false;
                break;
            }
        }
        if composite { return false; }
    }
    true
}

fn mod_pow(mut base: u64, mut exp: u64, modulus: u64) -> u64 {
    let mut res = 1;
    base %= modulus;
    while exp > 0 {
        if exp % 2 == 1 { res = (res as u128 * base as u128 % modulus as u128) as u64; }
        base = (base as u128 * base as u128 % modulus as u128) as u64;
        exp /= 2;
    }
    res
}

fn main() -> std::io::Result<()> {
    // --- SETTINGS ---
    let start_n: u64 = 1_000_000_000_000_000;
    let scan_range: u64 = 10_000;
    let end_n = start_n + scan_range;
    let dogrula = true;

    println!("--- DME MODULAR COLLISION & PURITY ANALYSIS (RUST CORE) ---");
    println!("Target Range: {} - {}\n", start_n, end_n);

    let start_time = Instant::now();
    
    // Sieve and Hit Tracker
    let mut sieve = vec![true; scan_range as usize];
    let mut hit_counts = vec![0u32; scan_range as usize];
    let mut total_commands = 0u64;

    // 1. Generate Module Pool (Sieve of Eratosthenes up to sqrt)
    let limit = (end_n as f64).sqrt() as u64;
    let mut temp_sieve = vec![true; (limit + 1) as usize];
    let mut modules = Vec::new();
    for p in 2..=limit {
        if temp_sieve[p as usize] {
            modules.push(p);
            let mut i = p * p;
            while i <= limit {
                temp_sieve[i as usize] = false;
                i += p;
            }
        }
    }

    // 2. DME ELIMINATION LOOP (Direct Offset)
    for &p in &modules {
        let mut first_match = (p - (start_n % p)) % p;
        if start_n + first_match == p { first_match += p; }

        let mut current = first_match;
        while current < scan_range {
            total_commands += 1;
            hit_counts[current as usize] += 1;
            sieve[current as usize] = false;
            current += p;
        }
    }

    let duration = start_time.elapsed();

    // 3. ANALYSIS & REPORTING
    let mut primes = Vec::new();
    let mut collision_stats: BTreeMap<u32, u32> = BTreeMap::new();
    let mut deleted_count = 0;

    for i in 0..scan_range as usize {
        let num = start_n + i as u64;
        if sieve[i] {
            primes.push(num);
        } else {
            deleted_count += 1;
            let hits = hit_counts[i];
            *collision_stats.entry(hits).or_insert(0) += 1;
        }
    }

    // 4. TWIN RESONANCE DETECTION
    let mut twins = Vec::new();
    for i in 0..primes.len().saturating_sub(1) {
        if primes[i+1] - primes[i] == 2 {
            twins.push((primes[i], primes[i+1]));
        }
    }

    // 5. AUTO-VALIDATION
    let mut accuracy = 100.0;
    if dogrula && !primes.is_empty() {
        let valid_count = primes.iter().filter(|&&p| is_prime_miller_rabin(p)).count();
        accuracy = (valid_count as f64 / primes.len() as f64) * 100.0;
    }

    // FILE OUTPUT
    let file_name = format!("DME_Q15_Rust_{}k.txt", scan_range / 1000);
    let mut file = File::create(&file_name)?;
    writeln!(file, "DME PURITY REPORT (RUST) | Range: {} - {}", start_n, end_n)?;
    for p in &primes { writeln!(file, "{}", p)?; }

    // REPORT
    println!("=============================================");
    println!("DME PERFORMANCE & PURITY REPORT (RUST)");
    println!("=============================================");
    println!("Total Elimination Commands : {}", total_commands);
    println!("Unique Deleted Numbers     : {}", deleted_count);
    println!("Detected Pure Primes       : {}", primes.len());
    println!("Detected Twin Pairs        : {}", twins.len());
    println!("Result File                : {}", file_name);
    println!("---------------------------------------------");
    println!("HIT DISTRIBUTION:");
    for (hits, count) in &collision_stats {
        println!(" {} hits: {} numbers", hits, count);
    }
    println!("---------------------------------------------");
    println!("Processing Time            : {:.6} sec", duration.as_secs_f64());
    println!("Accuracy Rate (Verified)   : %{:.2}", accuracy);
    println!("=============================================");

    Ok(())
}