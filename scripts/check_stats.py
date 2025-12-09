#!/usr/bin/env python3
import argparse
import csv
import math
import subprocess
import sys


def compute_from_csv(path: str):
    count = 0
    s = 0.0
    s2 = 0.0

    with open(path, newline='', encoding='utf-8') as f:
        reader = csv.reader(f)
        # Skip header if present
        header = next(reader, None)
        for row in reader:
            try:
                # Match C++ logic:
                # - use column 0 as id (must be non-empty)
                # - use column 9 as price (non-negative number)
                if len(row) <= 9:
                    continue
                id_field = row[0]
                if not id_field:
                    continue
                price_str = row[9]
                if price_str == '' or price_str is None:
                    continue
                price = float(price_str)
                if price < 0:
                    continue
            except Exception:
                # mimic C++: on any parse error, skip line
                continue

            s += price
            s2 += price * price
            count += 1

    if count == 0:
        return 0, float('nan'), float('nan'), float('nan')

    mean = s / count
    var = (s2 / count) - (mean * mean)
    # guard tiny negatives due to FP error
    if var < 0:
        var = 0.0
    std = math.sqrt(var)
    return count, mean, var, std


def run_pipeline(mapper_path: str, reducer_path: str, csv_path: str) -> str:
    # Stream file -> mapper -> reducer, capture reducer stdout
    with open(csv_path, 'rb') as f:
        p1 = subprocess.Popen([mapper_path], stdin=f, stdout=subprocess.PIPE)
        p2 = subprocess.Popen([reducer_path], stdin=p1.stdout, stdout=subprocess.PIPE)
        p1.stdout.close()  # allow p1 to receive SIGPIPE if p2 exits
        out, _ = p2.communicate()
    return out.decode('utf-8', errors='ignore').strip()


def main() -> int:
    ap = argparse.ArgumentParser(description="Check mean and variance against C++ map/reduce outputs")
    ap.add_argument('--csv', default='input/AB_NYC_2019.csv', help='Path to input CSV')
    ap.add_argument('--compare-binaries', action='store_true', help='Also run mapper/reducer binaries and compare')
    ap.add_argument('--mapper-mean', default='build-release/src/mapper_mean')
    ap.add_argument('--reducer-mean', default='build-release/src/reducer_mean')
    ap.add_argument('--mapper-var', default='build-release/src/mapper_var')
    ap.add_argument('--reducer-var', default='build-release/src/reducer_var')
    args = ap.parse_args()

    count, mean, var, std = compute_from_csv(args.csv)

    print(f"Rows used: {count}")
    print(f"Python mean: {mean}")
    print(f"Python variance: {var}")
    print(f"Python stddev: {std}")

    if args.compare_binaries:
        try:
            mean_out = run_pipeline(args.mapper_mean, args.reducer_mean, args.csv)
            var_out = run_pipeline(args.mapper_var, args.reducer_var, args.csv)
            print(f"C++ mean: {mean_out}")
            print(f"C++ variance: {var_out}")

            def parse_float(s: str) -> float:
                try:
                    return float(s)
                except Exception:
                    return float('nan')

            mean_cpp = parse_float(mean_out)
            var_cpp = parse_float(var_out)

            if math.isfinite(mean) and math.isfinite(mean_cpp):
                print(f"Mean abs diff: {abs(mean - mean_cpp)}")
            if math.isfinite(var) and math.isfinite(var_cpp):
                print(f"Variance abs diff: {abs(var - var_cpp)}")
        except FileNotFoundError as e:
            print(f"Binary not found: {e.filename}", file=sys.stderr)
            return 1
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
