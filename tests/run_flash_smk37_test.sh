#!/bin/sh
# The SMK-37's flash guard (fm1_flash_policy.h with BOARD_SMK37): an app's windows, then the update loader's.
set -eu
cd "$(dirname "$0")/.."
D=$(mktemp -d)
trap 'rm -rf "$D"' EXIT HUP INT TERM
for d in "" -DLOADER; do
    "${CC:-cc}" -std=c11 -O1 -w -Ichip $d tests/flash_smk37_test.c -o "$D/t"
    "$D/t"
done
