#!/bin/bash

SERVER="http://127.0.0.2:8004"
CGI_PATH="/cgi/test.py"

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

function print_header() {
    echo -e "\n===== $1 ====="
}

function run_test() {
    local name="$1"
    local url="$2"
    local method="$3"
    local data="$4"
    local expected="$5"

    print_header "$name"

    if [ "$method" == "POST" ]; then
        response=$(curl -s -i -X POST -H "Content-Type: application/x-www-form-urlencoded" --data "$data" "$url")
    else
        response=$(curl -s -i "$url")
    fi

    echo "EXPECTED:"
    echo "-------------------------"
    echo "$expected"
    echo
    echo "ACTUAL:"
    echo "-------------------------"
    echo "$response"
    echo

    echo "$response" > actual.txt
    echo "$expected" > expected.txt
    echo "$response" | tr -d '\r' > actual.txt
    echo "$expected" | tr -d '\r' > expected.txt
    if diff -u expected.txt actual.txt > /dev/null; then
        echo "✅ Test Passed"
    else
        echo "❌ Test Failed"
        diff -u expected.txt actual.txt
    fi
}

function test_get_no_query() {
    expected=$'HTTP/1.1 200 OK\nContent-Length: 7\nContent-Type: text/plain\nMethod: GET\nQuery: \r\n\r\nBody:'
    run_test "GET - No Query" "$SERVER$CGI_PATH" "GET" "" "$expected"
}

function test_get_with_query() {
    expected=$'HTTP/1.1 200 OK\nContent-Length: 7\nContent-Type: text/plain\nMethod: GET\nQuery: name=webserv&age=42\r\n\r\nBody:'
    run_test "GET - With Query" "$SERVER$CGI_PATH?name=webserv&age=42" "GET" "" "$expected"
}

function test_post_urlencoded() {
    expected=$'HTTP/1.1 200 OK\nContent-Length: 33\nContent-Type: application/x-www-form-urlencoded\nMethod: POST\nQuery: \r\n\r\nBody:\nusername=test&password=123'
    run_test "POST - Form-Encoded Body" "$SERVER$CGI_PATH" "POST" "username=test&password=123" "$expected"
}

function test_post_large_body() {
    expected="Method: POST"

    # Generate large binary payload and encode as base64
    bigdata=$(head -c 100000 < /dev/urandom | base64)

    # Save to a temp file
    tmpfile=$(mktemp)
    echo "data=$bigdata" > "$tmpfile"

    # Use curl to POST the file contents
    response=$(curl -s -i -X POST \
        -H "Content-Type: application/x-www-form-urlencoded" \
        --data-binary @"$tmpfile" \
        "$SERVER$CGI_PATH")

    echo "EXPECTED:"
    echo "-------------------------"
    echo "$expected"
    echo
    echo "ACTUAL:"
    echo "-------------------------"
    echo "$response"
    echo

    echo "$response" > actual.txt
    echo "$expected" > expected.txt
    if diff -u expected.txt actual.txt > /dev/null; then
        echo "✅ Test Passed"
    else
        echo "❌ Test Failed"
        diff -u expected.txt actual.txt
    fi

    # Cleanup
    rm "$tmpfile"
}

function test_path_traversal() {
    expected="403 Forbidden"
    run_test "Security - Path Traversal" "$SERVER/cgi-bin/../../etc/passwd" "GET" "" "$expected"
}

function test_not_found_script() {
    expected="404 Not Found"
    run_test "404 - Script Not Found" "$SERVER/cgi-bin/doesnotexist.py" "GET" "" "$expected"
}

function test_timeout_script() {
    expected="504 Gateway Timeout"
    run_test "Timeout - Sleep Script" "$SERVER/cgi-bin/sleep5.py" "GET" "" "$expected"
}

function test_malformed_output() {
    expected="502 Bad Gateway"
    run_test "Malformed Output" "$SERVER/cgi-bin/bad_output.py" "GET" "" "$expected"
}

function test_concurrent_requests() {
    print_header "Concurrent - 10 Requests"

    for i in {1..10}; do
        curl -s "$SERVER$CGI_PATH?i=$i" > /dev/null &
    done
    wait

    echo -e "${GREEN}✅ All concurrent requests completed${NC}"
}

# Run all tests
# test_get_no_query
# test_get_with_query
# test_post_urlencoded
test_post_large_body
# test_path_traversal
# test_not_found_script
# test_timeout_script
# test_malformed_output
# test_concurrent_requests

