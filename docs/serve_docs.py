import os
import http.server
import subprocess
import socketserver
from pathlib import Path

SCIPT_PARENT_PATH = Path(__file__).resolve().parent
DOCS_SOURCE_PATH = SCIPT_PARENT_PATH / "source"
DOCS_BUILD_PATH = DOCS_SOURCE_PATH / "build" / "html"
PORT = 8000

def build_docs():
    print("Building Docs")
    result = subprocess.run(
        ["sphinx-build", "-b", "html", str(DOCS_SOURCE_PATH), str(DOCS_BUILD_PATH)],
        cwd=SCIPT_PARENT_PATH,
    )
    if result.returncode != 0:
        print("Sphinx build failed")
        exit(1)

    print("Built Docs")

class CORSRequestHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        # Keeps local testing compatible with WebAssembly SharedArrayBuffer
        # self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        # self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        # Prevent browser caching so you always see your latest changes
        self.send_header("Cache-Control", "no-cache, no-store, must-revalidate")
        super().end_headers()

def serve_local():
    print("---Starting Server---")
    print(f"Staring server at: http://localhost:{PORT}")

    os.chdir(DOCS_BUILD_PATH)
    with socketserver.TCPServer(("", PORT), CORSRequestHandler) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("Server shutting down.")

if __name__ == "__main__":
    build_docs()
    serve_local()

