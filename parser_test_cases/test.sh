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

for test_case in parser_test_cases/*.lang; do

    no_extension=$(basename "$test_case" '.lang')
    expected="parser_test_cases/$no_extension.expected"
    got="parser_test_cases/$no_extension.got"
    heading="Test case $BLUE'$test_case'$CLEAR\n"

    ./compiler "$test_case" > "$got"

    if diff -q "$expected" "$got" 2>&1 >/dev/null; then
        printf "$GREEN$BOLD[ PASS ]$CLEAR $heading"
    else
        printf "$RED$BOLD[ FAIL ]$CLEAR $heading"
        diff --color=always "$expected" "$got" | indent
    fi

done
