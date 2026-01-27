
import os
import sys
import urllib.parse

MAX_BODY = 5_000_000  # 5MB

def parse_qs():
    return urllib.parse.parse_qs(os.environ.get("QUERY_STRING", ""), keep_blank_values=True)

def safe_name(name: str) -> str:
    name = os.path.basename(name)
    keep = []
    for c in name:
        if c.isalnum() or c in ("-", "_", ".", "@"):
            keep.append(c)
    out = "".join(keep)[:120]
    return out if out else "upload.bin"

def read_body():
    cl = os.environ.get("CONTENT_LENGTH", "")
    if cl.isdigit():
        n = int(cl)
        if n > MAX_BODY:
            return None, "413 Payload Too Large"
        return sys.stdin.buffer.read(n), None
    data = sys.stdin.buffer.read(MAX_BODY + 1)
    if len(data) > MAX_BODY:
        return None, "413 Payload Too Large"
    return data, None

def respond(text: str, status=None):
    hdrs = []
    if status:
        hdrs.append(f"Status: {status}")
    hdrs.append("Content-Type: text/plain; charset=utf-8")
    hdrs.append("X-CGI: upload.py")
    hdrs.append("")
    sys.stdout.write("\r\n".join(hdrs) + "\r\n")
    sys.stdout.write(text)
    sys.stdout.flush()

def main():
    method = os.environ.get("REQUEST_METHOD", "GET").upper()
    qs = parse_qs()

    if method == "GET":
        respond("Use POST raw body:\n"
                "curl -i -X POST --data-binary @file.bin "
                "\"http://HOST/cgi-bin/upload.py?name=file.bin\"\n")
        return

    if method != "POST":
        respond("Method Not Allowed\n", status="405 Method Not Allowed")
        return

    data, err = read_body()
    if err:
        respond(err + "\n", status=err)
        return

    name = safe_name(qs.get("name", ["upload.bin"])[0])
    try:
        with open(name, "wb") as f:
            f.write(data)
        respond(f"OK saved {name} ({len(data)} bytes) in cwd={os.getcwd()}\n")
    except Exception as e:
        respond(f"500 Save failed: {e}\n", status="500 Internal Server Error")

if __name__ == "__main__":
    main()
