#!/bin/bash
# runs one process with the example hosts file and perfect links config
# usage: ./run_one.sh <id> [config]

if [ -z "$1" ]; then
  echo "usage: $0 <id> [config]" >&2
  exit 1
fi

cd "$(dirname "$0")"
config="${2:-../example/configs/perfect-links.config}"

exec ./run.sh --id "$1" --hosts ../example/hosts --output "../example/output/$1.output" "$config"
