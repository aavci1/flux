#!/usr/bin/env bash
# Deprecated: Flux no longer uses git submodules.
# NanoVG, NanoSVG, and stb are downloaded automatically by CMake (FetchContent)
# on first configure—same pattern as SDL3 when not installed system-wide.
#
# To bump pinned versions, edit CMakeLists.txt cache defaults:
#   FLUX_NANOVG_GIT_TAG, FLUX_NANOSVG_GIT_TAG, FLUX_STB_GIT_TAG
#
echo "Note: submodules were removed; CMake fetches deps. Nothing to do."
exit 0
