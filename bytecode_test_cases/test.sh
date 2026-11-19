#!/bin/sh

# (invoke this script from the repo's root)

RED="\e[31m"
GREEN="\e[32m"
BLUE="\e[34m"
BOLD="\e[1m"
CLEAR="\e[0m"

indent() {
    sed 's/^/ | /'
}

did_any_fail=''

for test_case_full_name in bytecode_test_cases/*.lang; do

    heading="Correctly generate bytecode for $BLUE'$test_case_full_name'$CLEAR\n"
    test_case_basename=$(basename "$test_case_full_name" '.lang')
    got="bytecode_test_cases/$test_case_basename.got"

    expected="bytecode_test_cases/$test_case_basename.expected"
    if [ ! -f "$expected" ]; then
        expected="$test_case_full_name"
    fi

    ./compiler --bytecodify-and-exit "$test_case_full_name" > "$got"

    if diff -q "$expected" "$got" >/dev/null 2>&1; then
        printf "$GREEN$BOLD[ PASS ]$CLEAR $heading"
    else
        printf "$RED$BOLD[ FAIL ]$CLEAR $heading"
        diff --color=always "$expected" "$got" | indent
        did_any_fail=yes
    fi

done

if [ -n "$did_any_fail" ]; then
    exit 1
fi
