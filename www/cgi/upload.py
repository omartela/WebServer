#!/usr/bin/env python3
import sys
import cgitb
import json

cgitb.enable()

def main():
    try:
        raw_input = sys.stdin.read()
        json_data = json.loads(raw_input)

        with open("/tmp/db.json", 'a') as db:
            json.dump(json_data, db)
            db.write("\n")

        # Compose response body as a single string
        response_body = json.dumps({
            "status": "success",
            "message": "Raw JSON saved"
        })

        # Write full CGI response
        sys.stdout.write("Content-Type: application/json\r\n")
        sys.stdout.write(f"Content-Length: {len(response_body)}\r\n")
        sys.stdout.write("\r\n")
        sys.stdout.write(response_body)
        sys.stdout.flush()

    except Exception as e:
        error_body = json.dumps({
            "status": "error",
            "message": str(e)
        })
        sys.stdout.write("Content-Type: application/json\r\n")
        sys.stdout.write(f"Content-Length: {len(error_body)}\r\n")
        sys.stdout.write("\r\n")
        sys.stdout.write(error_body)
        sys.stdout.flush()

if __name__ == "__main__":
    main()