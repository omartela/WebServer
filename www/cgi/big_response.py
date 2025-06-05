#!/usr/bin/env python3
import sys

def main():
    # Write required HTTP headers
    sys.stdout.write("Content-Type: text/plain\r\n")
    sys.stdout.write("\r\n")  # End of headers

    # Generate a large response (e.g., 10 MB of 'A')
    chunk = "1" * 10  # 1 KB
    for _ in range(1):  # 10 MB total
        sys.stdout.write(chunk)

if __name__ == "__main__":
    main()
