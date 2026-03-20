#!/usr/bin/env python3
"""
Emit FluxEmbeddedShaders.hpp / .cpp from compiled .metal and .spv files.
Usage:
  embed_shaders.py <output_base_path> <msl_path> <spv_path> <stem> ...
  stem: C identifier suffix, e.g. sdf_quad_vert_glsl for sdf_quad.vert.glsl
"""

from __future__ import annotations

import pathlib
import sys


def emit_byte_array(name: str, data: bytes) -> str:
    lines: list[str] = [f"static const unsigned char {name}[] = {{"]
    row: list[str] = []
    for b in data:
        row.append(f"0x{b:02x}")
        if len(row) == 14:
            lines.append("  " + ", ".join(row) + ",")
            row = []
    if row:
        lines.append("  " + ", ".join(row) + ",")
    lines.append("};")
    return "\n".join(lines)


def main() -> int:
    argv = sys.argv[1:]
    if len(argv) < 1 or len(argv) % 3 != 1:
        print(
            "usage: embed_shaders.py <output_base> <msl> <spv> <stem> ...",
            file=sys.stderr,
        )
        return 2

    out_base = pathlib.Path(argv[0])
    triples: list[tuple[pathlib.Path, pathlib.Path, str]] = []
    rest = argv[1:]
    for i in range(0, len(rest), 3):
        triples.append((pathlib.Path(rest[i]), pathlib.Path(rest[i + 1]), rest[i + 2]))

    hpp = out_base.with_suffix(".hpp")
    cpp = out_base.with_suffix(".cpp")

    hpp_lines: list[str] = [
        "#pragma once",
        "",
        "#include <cstddef>",
        "#include <cstdint>",
        "#include <span>",
        "#include <string_view>",
        "",
        "namespace flux::gpu::embedded {",
        "",
    ]

    cpp_lines: list[str] = [
        '#include "FluxEmbeddedShaders.hpp"',
        "",
        "namespace {",
        "",
    ]

    for msl_path, spv_path, stem in triples:
        msl_data = msl_path.read_bytes()
        spv_data = spv_path.read_bytes()
        msl_arr = f"k_msl_{stem}"
        spv_arr = f"k_spv_{stem}"
        cpp_lines.append(emit_byte_array(msl_arr, msl_data))
        cpp_lines.append("")
        cpp_lines.append(emit_byte_array(spv_arr, spv_data))
        cpp_lines.append("")

    cpp_lines.append("} // anonymous namespace")
    cpp_lines.append("")
    cpp_lines.append("namespace flux::gpu::embedded {")
    cpp_lines.append("")

    for _msl_path, _spv_path, stem in triples:
        msl_arr = f"k_msl_{stem}"
        spv_arr = f"k_spv_{stem}"
        hpp_lines.append(f"[[nodiscard]] std::string_view msl_{stem}();")
        hpp_lines.append(f"[[nodiscard]] std::span<const std::uint8_t> spv_{stem}();")
        hpp_lines.append("")
        cpp_lines.append(f"std::string_view msl_{stem}() {{")
        cpp_lines.append(
            f"  return {{ reinterpret_cast<const char*>({msl_arr}), sizeof({msl_arr}) }};"
        )
        cpp_lines.append("}")
        cpp_lines.append("")
        cpp_lines.append(f"std::span<const std::uint8_t> spv_{stem}() {{")
        cpp_lines.append(f"  return {{ {spv_arr}, sizeof({spv_arr}) }};")
        cpp_lines.append("}")
        cpp_lines.append("")

    hpp_lines.append("} // namespace flux::gpu::embedded")
    hpp_lines.append("")

    cpp_lines.append("} // namespace flux::gpu::embedded")
    cpp_lines.append("")

    hpp.write_text("\n".join(hpp_lines), encoding="utf-8")
    cpp.write_text("\n".join(cpp_lines), encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
