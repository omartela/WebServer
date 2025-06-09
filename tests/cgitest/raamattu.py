import socket
import time
import textwrap

# Load Genesis text (truncated example – in real use, load from a file or compress)
GENESIS_TEXT = """1:1 In the beginning God created the heaven and the earth.

1:2 And the earth was without form, and void; and darkness was upon
the face of the deep. And the Spirit of God moved upon the face of the
waters.

1:3 And God said, Let there be light: and there was light.

1:4 And God saw the light, that it was good: and God divided the light
from the darkness.

1:5 And God called the light Day, and the darkness he called Night.
And the evening and the morning were the first day.

1:6 And God said, Let there be a firmament in the midst of the waters,
and let it divide the waters from the waters.

1:7 And God made the firmament, and divided the waters which were
under the firmament from the waters which were above the firmament:
and it was so.

1:8 And God called the firmament Heaven. And the evening and the
morning were the second day.

1:9 And God said, Let the waters under the heaven be gathered together
unto one place, and let the dry land appear: and it was so.

1:10 And God called the dry land Earth; and the gathering together of
the waters called he Seas: and God saw that it was good.

1:11 And God said, Let the earth bring forth grass, the herb yielding
seed, and the fruit tree yielding fruit after his kind, whose seed is
in itself, upon the earth: and it was so.

1:12 And the earth brought forth grass, and herb yielding seed after
his kind, and the tree yielding fruit, whose seed was in itself, after
his kind: and God saw that it was good.

1:13 And the evening and the morning were the third day.

1:14 And God said, Let there be lights in the firmament of the heaven
to divide the day from the night; and let them be for signs, and for
seasons, and for days, and years: 1:15 And let them be for lights in
the firmament of the heaven to give light upon the earth: and it was
so."""

def send_chunked_genesis(text, host='127.0.0.2', port=8004, path='/directory/test.py'):
    # Connect to server
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))

    # Send HTTP headers with Transfer-Encoding: chunked
    headers = (
        f"POST {path} HTTP/1.1\r\n"
        f"Host: localhost\r\n"
        f"Content-Type: text/plain\r\n"
        f"Transfer-Encoding: chunked\r\n"
        f"\r\n"
    )
    sock.send(headers.encode())

    # Send the body in chunks (e.g. 200 bytes per chunk)
    chunks = textwrap.wrap(text, 200)  # You can adjust chunk size here

    for chunk in chunks:
        data = chunk.encode()
        length = f"{len(data):X}\r\n".encode()  # Hex length + CRLF
        sock.send(length)
        sock.send(data + b"\r\n")
        time.sleep(0.2)  # Optional: simulate streaming delay

    # Final chunk
    sock.send(b"0\r\n\r\n")

    # Receive and print server response
    response = sock.recv(4096).decode()
    print("📥 Server response:\n", response)

    sock.close()

if __name__ == "__main__":
    send_chunked_genesis(GENESIS_TEXT)

