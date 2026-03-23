"""Decode 8-bit RGBA PNG bytes to raw pixels (no third-party deps)."""

from __future__ import annotations

import struct
import zlib


def read_png_rgba(png: bytes) -> tuple[int, int, bytes]:
    """Return (width, height, rgba_bytes row-major)."""
    if len(png) < 24 or png[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError("not a PNG")

    pos = 8
    width = height = 0
    idat_parts: list[bytes] = []

    while pos < len(png):
        if pos + 12 > len(png):
            break
        length = struct.unpack(">I", png[pos : pos + 4])[0]
        ctype = png[pos + 4 : pos + 8]
        chunk = png[pos + 8 : pos + 8 + length]
        pos += 12 + length
        if ctype == b"IHDR":
            width, height = struct.unpack(">II", chunk[:8])
            bit_depth = chunk[8]
            color_type = chunk[9]
            if bit_depth != 8 or color_type != 6:
                raise ValueError(f"unsupported PNG mode depth={bit_depth} ct={color_type}")
        elif ctype == b"IDAT":
            idat_parts.append(chunk)
        elif ctype == b"IEND":
            break

    raw = zlib.decompress(b"".join(idat_parts))
    bpp = 4
    stride = width * bpp
    out = bytearray(height * stride)

    def paeth(a: int, b: int, c: int) -> int:
        p = a + b - c
        pa = abs(p - a)
        pb = abs(p - b)
        pc = abs(p - c)
        if pa <= pb and pa <= pc:
            return a
        if pb <= pc:
            return b
        return c

    i = 0
    prev = bytearray(stride)
    for _y in range(height):
        ft = raw[i]
        i += 1
        row = bytearray(raw[i : i + stride])
        i += stride
        if ft == 0:
            pass
        elif ft == 1:
            for x in range(stride):
                left = row[x - bpp] if x >= bpp else 0
                row[x] = (row[x] + left) & 0xFF
        elif ft == 2:
            for x in range(stride):
                row[x] = (row[x] + prev[x]) & 0xFF
        elif ft == 3:
            for x in range(stride):
                left = row[x - bpp] if x >= bpp else 0
                up = prev[x]
                row[x] = (row[x] + ((left + up) // 2)) & 0xFF
        elif ft == 4:
            for x in range(stride):
                left = row[x - bpp] if x >= bpp else 0
                up = prev[x]
                upleft = prev[x - bpp] if x >= bpp else 0
                row[x] = (row[x] + paeth(left, up, upleft)) & 0xFF
        else:
            raise ValueError(f"unknown PNG filter {ft}")

        out[_y * stride : (_y + 1) * stride] = row
        prev = row

    return width, height, bytes(out)


def region_max_channel_stats(
    rgba: bytes, width: int, height: int, x0: int, y0: int, x1: int, y1: int
) -> tuple[int, int, int]:
    """Return (min_max_channel, max_max_channel, mean_max_channel) per pixel in region."""
    bpp = 4
    stride = width * bpp
    vals: list[int] = []
    for y in range(max(0, y0), min(height, y1 + 1)):
        for x in range(max(0, x0), min(width, x1 + 1)):
            o = y * stride + x * bpp
            r, g, b = rgba[o], rgba[o + 1], rgba[o + 2]
            vals.append(max(r, g, b))
    if not vals:
        return 0, 0, 0
    return min(vals), max(vals), sum(vals) // len(vals)
