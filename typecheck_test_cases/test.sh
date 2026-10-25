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

for test_case_full_name in typecheck_test_cases/*.lang; do

    heading="$BLUE'$test_case_full_name'$CLEAR\n"
    test_case_basename=$(basename "$test_case_full_name" '.lang')
    got="typecheck_test_cases/$test_case_basename.got"

    # When a '.expected' file is absent, treat the test case as expected to fail
    expected_file="typecheck_test_cases/$test_case_basename.expected"
    expected_exit_code=0
    if [ ! -f "$expected_file" ]; then
        expected_exit_code=67
    fi

    ./compiler --typecheck-and-exit "$test_case_full_name" > "$got" 2>&1 
    exit_code=$?

    if [ "$exit_code" -eq 0 ] && [ "$expected_exit_code" -ne 0 ]; then
        printf "$RED$BOLD[ FAIL ]$CLEAR Typecheck accepted an invalid program $heading"
        did_any_fail=yes
    elif [ "$exit_code" -ne 0 ] && [ "$expected_exit_code" -eq 0 ]; then
        printf "$RED$BOLD[ FAIL ]$CLEAR Typecheck rejected a valid program $heading"
        did_any_fail=yes
    elif [ "$exit_code" -ne 0 ] && [ "$expected_exit_code" -ne 0 ]; then
        printf "$GREEN$BOLD[ PASS ]$CLEAR Typechecker fails an invalid program $heading"
    elif diff -q "$expected_file" "$got" >/dev/null 2>&1; then
        printf "$GREEN$BOLD[ PASS ]$CLEAR Output matches expected $heading"
    else
        printf "$RED$BOLD[ FAIL ]$CLEAR Output doesn't match expected $heading"
        diff --color=always "$expected_file" "$got" | indent
        did_any_fail=yes
    fi

done

if [ -n "$did_any_fail" ]; then
    exit 1
fi
