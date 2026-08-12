#!/usr/bin/env bash
set -u

cd "$(dirname "$0")/.."

BINARY="./compilerfrontend"
if [ ! -x "$BINARY" ] && [ -x "./compilerfrontend.exe" ]; then
    BINARY="./compilerfrontend.exe"
fi

if [ ! -x "$BINARY" ]; then
    echo "compilerfrontend binary not found — run 'make' first."
    exit 1
fi

SAMPLES_DIR="tests/samples"
pass=0
fail=0

for input in "$SAMPLES_DIR"/*.txt; do
    name=$(basename "$input" .txt)
    expected="$SAMPLES_DIR/$name.expected"

    if [ ! -f "$expected" ]; then
        echo "SKIP $name (no .expected file)"
        continue
    fi

    case "$name" in
        *eval*) extra_args="--eval" ;;
        *) extra_args="" ;;
    esac

    actual=$("$BINARY" "$input" $extra_args 2>&1)
    exit_code=$?
    expected_content=$(cat "$expected")

    case "$name" in
        valid_*|eval_*) expected_exit=0 ;;
        broken_*) expected_exit=1 ;;
        *) expected_exit=0 ;;
    esac

    if [ "$actual" == "$expected_content" ] && [ "$exit_code" -eq "$expected_exit" ]; then
        echo "PASS $name"
        pass=$((pass + 1))
    else
        echo "FAIL $name"
        if [ "$exit_code" -ne "$expected_exit" ]; then
            echo "  exit code: expected $expected_exit, got $exit_code"
        fi
        if [ "$actual" != "$expected_content" ]; then
            echo "  --- expected ---"
            echo "$expected_content" | sed 's/^/  /'
            echo "  --- actual ---"
            echo "$actual" | sed 's/^/  /'
        fi
        fail=$((fail + 1))
    fi
done

echo ""
echo "$pass passed, $fail failed"

if [ "$fail" -ne 0 ]; then
    exit 1
fi
exit 0
