#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   scripts/mapreduce.sh <mean|var> <input_csv>
# Example:
#   scripts/mapreduce.sh mean ./input/AB_NYC_2019.csv

MODE=${1:-}
INPUT=${2:-}

if [[ -z "${MODE}" || -z "${INPUT}" ]]; then
  echo "Usage: $0 <mean|var> <input_csv>" >&2
  exit 2
fi

case "${MODE}" in
  mean)
    MAPPER="mapper_mean"
    REDUCER="reducer_mean"
    ;;
  var)
    MAPPER="mapper_var"
    REDUCER="reducer_var"
    ;;
  *)
    echo "Unknown mode: ${MODE} (expected: mean|var)" >&2
    exit 2
    ;;
esac

if ! command -v "${MAPPER}" >/dev/null 2>&1 || ! command -v "${REDUCER}" >/dev/null 2>&1; then
  echo "Binaries not found in PATH: ${MAPPER} or ${REDUCER}. Install first, e.g.:" >&2
  echo "  cmake -S . -B build && cmake --build build -j && cmake --install build" >&2
  exit 1
fi

export LC_ALL=C
cat "${INPUT}" | "${MAPPER}" | sort -k1,1 | "${REDUCER}"
