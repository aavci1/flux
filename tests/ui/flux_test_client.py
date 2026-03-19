"""
Flux UI Test Client

HTTP client wrapper for the Flux test server. Provides methods to interact
with a running Flux app via the test server API, and helpers for inspecting
the serialized UI tree.
"""

import json
import time
import subprocess
import signal
import os
import socket
from urllib.request import urlopen, Request
from urllib.error import URLError
from typing import Optional


class FluxTestClient:
    """HTTP client for a single Flux test server instance."""

    def __init__(self, port: int = 8435, base_url: str | None = None):
        self.port = port
        self.base_url = base_url or f"http://localhost:{port}"

    def _get(self, path: str) -> dict | list | str:
        url = f"{self.base_url}{path}"
        for attempt in range(3):
            try:
                with urlopen(url, timeout=10) as resp:
                    data = resp.read().decode("utf-8")
                return json.loads(data)
            except (ConnectionResetError, ConnectionRefusedError, URLError, OSError):
                if attempt == 2:
                    raise
                time.sleep(0.5)

    def _post(self, path: str, body: dict) -> dict:
        url = f"{self.base_url}{path}"
        payload = json.dumps(body).encode("utf-8")
        req = Request(url, data=payload, headers={"Content-Type": "application/json"})
        with urlopen(req, timeout=10) as resp:
            data = resp.read().decode("utf-8")
        return json.loads(data)

    # --- Core API ---

    def get_ui(self) -> dict:
        return self._get("/ui")

    def get_screenshot(self) -> bytes:
        url = f"{self.base_url}/screenshot"
        with urlopen(url, timeout=5) as resp:
            return resp.read()

    def click(self, x: float, y: float) -> dict:
        return self._post("/click", {"x": x, "y": y})

    def type_text(self, text: str) -> dict:
        return self._post("/type", {"text": text})

    def press_key(self, key: str) -> dict:
        return self._post("/key", {"key": key})

    def scroll(self, x: float, y: float, delta_x: float = 0, delta_y: float = 0) -> dict:
        return self._post("/scroll", {"x": x, "y": y, "deltaX": delta_x, "deltaY": delta_y})

    def hover(self, x: float, y: float) -> dict:
        return self._post("/hover", {"x": x, "y": y})

    def drag(self, start_x: float, start_y: float, end_x: float, end_y: float, steps: int = 10) -> dict:
        return self._post("/drag", {
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
            except (URLError, ConnectionError, OSError, ValueError):
                pass
            time.sleep(interval)
        raise TimeoutError(f"Test server on port {self.port} did not become ready within {timeout}s")


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
    """Launches a Flux test app as a subprocess and manages its lifecycle."""

    def __init__(self, executable: str, port: int = 8435, extra_args: list[str] | None = None):
        self.executable = executable
        self.port = port
        self.extra_args = extra_args or []
        self.process: Optional[subprocess.Popen] = None

    def start(self):
        cmd = [self.executable, "--test-mode", "--test-port", str(self.port)] + self.extra_args
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

    def is_alive(self) -> bool:
        return self.process is not None and self.process.poll() is None

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, *_):
        self.stop()


def find_free_port() -> int:
    """Find and return a free TCP port."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(("", 0))
        return s.getsockname()[1]
