"""
Flux UI Test Client

Binary IPC client for the Flux test server (TCP or Unix domain socket).
"""

import json
import os
import struct
import socket
import subprocess
import signal
import tempfile
import time
import uuid
from typing import Optional


FLUX_TEST_MAGIC = 0x58554C46  # 'FLUX' on wire (little-endian)


class Op:
    GetUi = 1
    GetScreenshot = 2
    Click = 3
    Type = 4
    Key = 5
    Scroll = 6
    Hover = 7
    Drag = 8


def _recv_exact(sock: socket.socket, n: int) -> bytes:
    buf = b""
    while len(buf) < n:
        chunk = sock.recv(n - len(buf))
        if not chunk:
            raise ConnectionError("connection closed")
        buf += chunk
    return buf


class FluxTestClient:
    """Client for a single Flux test server (TCP or Unix socket)."""

    def __init__(self, port: int = 8435, unix_socket: str | None = None):
        self.port = port
        self.unix_socket = unix_socket

    def _connect(self) -> socket.socket:
        if self.unix_socket:
            s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            s.connect(self.unix_socket)
            return s
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect(("127.0.0.1", self.port))
        return s

    def _rpc(self, opcode: int, body: bytes = b"") -> tuple[int, int, bytes]:
        """Returns (status, payload_type, body). status 0 = ok."""
        sock = self._connect()
        try:
            req = struct.pack("<IHHI", FLUX_TEST_MAGIC, 1, opcode, len(body)) + body
            sock.sendall(req)
            hdr = _recv_exact(sock, 8)
            status = hdr[0]
            payload_type = hdr[1]
            (body_len,) = struct.unpack("<I", hdr[4:8])
            payload = _recv_exact(sock, body_len) if body_len else b""
            return status, payload_type, payload
        finally:
            sock.close()

    def _rpc_json_post(self, opcode: int, body: dict) -> dict:
        status, ptype, payload = self._rpc(opcode, json.dumps(body).encode("utf-8"))
        if status != 0:
            try:
                err = json.loads(payload.decode("utf-8"))
            except (json.JSONDecodeError, UnicodeDecodeError):
                err = {"error": payload.decode("utf-8", errors="replace")}
            raise RuntimeError(f"RPC error: {err}")
        if ptype != 0:
            raise RuntimeError("unexpected non-JSON payload")
        return json.loads(payload.decode("utf-8"))

    # --- Core API ---

    def get_ui(self) -> dict:
        for attempt in range(3):
            try:
                status, ptype, payload = self._rpc(Op.GetUi, b"")
                if status != 0:
                    try:
                        err = json.loads(payload.decode("utf-8"))
                    except (json.JSONDecodeError, UnicodeDecodeError):
                        err = {"error": payload.decode("utf-8", errors="replace")}
                    raise RuntimeError(f"RPC error: {err}")
                if ptype != 0:
                    raise RuntimeError("unexpected non-JSON payload")
                return json.loads(payload.decode("utf-8"))
            except (ConnectionError, OSError):
                if attempt == 2:
                    raise
                time.sleep(0.5)

    def get_screenshot(self) -> bytes:
        status, ptype, payload = self._rpc(Op.GetScreenshot, b"")
        if status != 0:
            try:
                err = json.loads(payload.decode("utf-8"))
            except (json.JSONDecodeError, UnicodeDecodeError):
                err = {"error": payload.decode("utf-8", errors="replace")}
            raise RuntimeError(f"RPC error: {err}")
        if ptype != 1:
            raise RuntimeError("expected PNG payload")
        return payload

    def click(self, x: float, y: float) -> dict:
        return self._rpc_json_post(Op.Click, {"x": x, "y": y})

    def type_text(self, text: str) -> dict:
        return self._rpc_json_post(Op.Type, {"text": text})

    def press_key(self, key: str) -> dict:
        return self._rpc_json_post(Op.Key, {"key": key})

    def scroll(self, x: float, y: float, delta_x: float = 0, delta_y: float = 0) -> dict:
        return self._rpc_json_post(Op.Scroll, {"x": x, "y": y, "deltaX": delta_x, "deltaY": delta_y})

    def hover(self, x: float, y: float) -> dict:
        return self._rpc_json_post(Op.Hover, {"x": x, "y": y})

    def drag(self, start_x: float, start_y: float, end_x: float, end_y: float, steps: int = 10) -> dict:
        return self._rpc_json_post(Op.Drag, {
            "startX": start_x, "startY": start_y,
            "endX": end_x, "endY": end_y,
            "steps": steps
        })

    def wait_ready(self, timeout: float = 15.0, interval: float = 0.3):
        """Block until the test server responds with a non-empty UI tree."""
        deadline = time.time() + timeout
        while time.time() < deadline:
            try:
                tree = self.get_ui()
                if isinstance(tree, dict) and tree.get("children"):
                    return
            except (OSError, ConnectionError, RuntimeError, ValueError):
                pass
            time.sleep(interval)
        loc = self.unix_socket or f"port {self.port}"
        raise TimeoutError(f"Test server ({loc}) did not become ready within {timeout}s")


# --- UI tree helpers ---

def find_all(tree: dict, predicate) -> list[dict]:
    """Return every node in the tree matching *predicate(node)*."""
    results = []
    if predicate(tree):
        results.append(tree)
    for child in tree.get("children", []):
        results.extend(find_all(child, predicate))
    return results


def find_first(tree: dict, predicate) -> Optional[dict]:
    """Return the first node matching *predicate*, depth-first."""
    if predicate(tree):
        return tree
    for child in tree.get("children", []):
        result = find_first(child, predicate)
        if result is not None:
            return result
    return None


def find_by_type(tree: dict, type_name: str) -> list[dict]:
    return find_all(tree, lambda n: n.get("type", "").endswith(type_name))


def find_by_text(tree: dict, text: str) -> list[dict]:
    return find_all(tree, lambda n: n.get("text") == text)


def find_by_text_containing(tree: dict, substring: str) -> list[dict]:
    return find_all(tree, lambda n: substring in (n.get("text") or ""))


def find_by_focus_key(tree: dict, key: str) -> Optional[dict]:
    return find_first(tree, lambda n: n.get("focusKey") == key)


def get_bounds(node: dict) -> dict:
    """Return {x, y, w, h} from a node's bounds."""
    return node.get("bounds", {})


def center_of(node: dict) -> tuple[float, float]:
    """Return (cx, cy) for a node."""
    b = get_bounds(node)
    return b.get("x", 0) + b.get("w", 0) / 2, b.get("y", 0) + b.get("h", 0) / 2


def get_text_value(tree: dict, prefix: str) -> Optional[str]:
    """Find a text node whose text starts with *prefix* and return the part after the prefix."""
    nodes = find_all(tree, lambda n: (n.get("text") or "").startswith(prefix))
    if not nodes:
        return None
    return nodes[0]["text"][len(prefix):]


# --- Process management ---

class FluxAppProcess:
    """Launches a Flux test app in --test-mode with a Unix domain socket (default) or TCP port."""

    def __init__(self, executable: str, port: int = 8435, extra_args: list[str] | None = None,
                 unix_socket: str | None = None):
        self.executable = executable
        self.port = port
        self.extra_args = extra_args or []
        self.unix_socket = unix_socket
        self.process: Optional[subprocess.Popen] = None

    def start(self):
        if self.unix_socket is None:
            self.unix_socket = os.path.join(
                tempfile.gettempdir(),
                f"flux-test-{os.getpid()}-{uuid.uuid4().hex}.sock",
            )
        if os.path.exists(self.unix_socket):
            try:
                os.unlink(self.unix_socket)
            except OSError:
                pass

        cmd = [
            self.executable, "--test-mode", "--test-socket", self.unix_socket,
        ] + self.extra_args

        backend = os.environ.get("FLUX_TEST_BACKEND")
        if backend:
            cmd.extend(["--backend", backend])

        self.process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )

    def stop(self):
        if self.process and self.process.poll() is None:
            self.process.send_signal(signal.SIGTERM)
            try:
                self.process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.process.wait(timeout=2)
        if self.unix_socket and os.path.exists(self.unix_socket):
            try:
                os.unlink(self.unix_socket)
            except OSError:
                pass

    def is_alive(self) -> bool:
        return self.process is not None and self.process.poll() is None

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, *_):
        self.stop()


def find_free_port() -> int:
    """Find and return a free TCP port (for TCP-based tests)."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(("", 0))
        return s.getsockname()[1]
