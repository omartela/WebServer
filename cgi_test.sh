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
    expected=$'HTTP/1.1 200 OK\nContent-Length: 6\nContent-Type: text/plain\nMethod: GET\nQuery: \r\n\r\nBody:'
    run_test "GET - No Query" "$SERVER$CGI_PATH" "GET" "" "$expected"
}

function test_get_with_query() {
    expected=$'HTTP/1.1 200 OK\nContent-Length: 6\nContent-Type: text/plain\nMethod: GET\nQuery: name=webserv&age=42\r\n\r\nBody:'
    run_test "GET - With Query" "$SERVER$CGI_PATH?name=webserv&age=42" "GET" "" "$expected"
}

function test_post_urlencoded() {
    expected=$'HTTP/1.1 200 OK\nContent-Length: 33\nContent-Type: application/x-www-form-urlencoded\nMethod: POST\nQuery: \r\n\r\nBody:\nusername=test&password=123'
    run_test "POST - Form-Encoded Body" "$SERVER$CGI_PATH" "POST" "username=test&password=123" "$expected"
}

function test_post_large_body() {
    expected=$'HTTP/1.1 200 OK\r\nContent-Length: 30\r\nContent-Type: application/octet-stream\r\n\r\nFile(s) uploaded successfully'
    expected=$'HTTP/1.1 200 OK\r\nContent-Length: 30\r\nContent-Type: application/octet-stream\r\n\r\nFile(s) uploaded successfully'

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
    expected=$'HTTP/1.1 403 Forbidden\nContent-Length: 139\nContent-Type: text/html; charset=UTF-8\r\n\r\n<html><head><title>403 Forbidden</title></head><body><h1>403 Forbidden</h1><p>The server encountered an error: Forbidden.</p></body></html>'
    run_test "Security - Path Traversal" "$SERVER/cgi/%2E%2E/%2E%2E/etc/passwd" "GET" "" "$expected"
}

function test_not_found_script() {
    expected=$'HTTP/1.1 404 Invalid file\nContent-Length: 142\nContent-Type: text/html; charset=UTF-8\r\n\r\n<html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1><p>The server encountered an error: Invalid file.</p></body></html>'
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
    mkdir -p tmp_cgi_test

    # Send 10 concurrent requests and capture output
    for i in {1..10}; do
        curl -s -i "$SERVER$CGI_PATH?i=$i" > "tmp_cgi_test/output_$i.txt" &
    done
    wait

    # Check responses
    all_ok=true
    for i in {1..10}; do
        file="tmp_cgi_test/output_$i.txt"
        if [ ! -f "$file" ]; then
            echo "❌ Missing response for request i=$i"
            all_ok=false
            continue
        fi

        # Check if response contains expected identifier
        if ! grep -q "i=$i" "$file"; then
            echo "❌ Incorrect or missing data in response $file"
            all_ok=false
        fi
    done

    if $all_ok; then
        echo -e "${GREEN}✅ All 10 concurrent responses received and valid${NC}"
    else
        echo -e "${RED}❌ Some responses were missing or invalid${NC}"
    fi

    # Cleanup
    # rm -r tmp_cgi_test
}

# Run all tests
#test_get_no_query
#test_get_with_query
#test_post_urlencoded
#test_post_large_body
#test_path_traversal
# test_not_found_script
# test_timeout_script
# test_malformed_output
test_concurrent_requests
