# Jetson Verification (2026-07-20)

Verified on `jetson-nano-ssh` (aarch64, Ubuntu 24.04 / L4T, GNU 13.3.0,
TensorRT EP available).

## What was done

- Transferred the source tree, installed apt deps (already present:
  `libopencv-dev`, `libcurl4-openssl-dev`, `doctest-dev`, `libcxxopts-dev`,
  `cmake`, `build-essential`).
- Vendored onnxruntime **v1.27.1** aarch64 headers (v1.28.0 has no GitHub
  release; 1.27.1 headers are ABI-compatible with the 1.28.0 runtime `.so`).
- First pass: linked the runtime `.so` straight from an existing
  `ocr-benchmark` Python venv (`-DARBOOCR_ORT_LIB_DIR=.../onnxruntime/capi`)
  to prove the build works at all. Confirmed working, then **superseded** —
  see below.
- **Made fully standalone**: copied `libonnxruntime.so.1.28.0` +
  `libonnxruntime_providers_{shared,cuda,tensorrt}.so` (~155MB) from that
  venv into `vendor/onnxruntime/lib/` inside arboOCR itself, recreated the
  `.so`/`.so.1` symlinks, reconfigured with the (now-default)
  `ARBOOCR_ORT_LIB_DIR=vendor/onnxruntime/lib`, rebuilt. `ldd` confirms the
  binary now resolves onnxruntime from inside the arboOCR tree, not the venv.
  arboOCR no longer depends on `ocr-benchmark` being installed at all.
- Provisioned **tiny models only** (`PP-OCRv6_det_tiny` → `PP-OCRv6_det.onnx`,
  `PP-OCRv6_rec_tiny`) — deliberately small so TRT engine build stays quick.

## Results (against the vendored, standalone runtime)

Demo on the bundled Indonesian receipt (`arboocr_demo`, tiny models):

| Backend | Lines | Time (engine cached) |
|---|---|---|
| cpu | 31 | 734 ms |
| tensorrt | 31 | 460 ms |

Both backends produce identical text output. First TensorRT run compiles +
caches the engine (slow, minutes on a Nano); subsequent runs use the cache.

Unit tests: `17 passed | 0 failed | 1 skipped` (the skipped case is the
optional real-model inference test).

## Runtime note

The build bakes `ARBOOCR_ORT_LIB_DIR` (default: `vendor/onnxruntime/lib`)
into the binaries' RPATH, so no `LD_LIBRARY_PATH` is needed at all — verified
by running `arboocr_demo` with `LD_LIBRARY_PATH` explicitly unset.
`vendor/onnxruntime/` is gitignored (headers + runtime `.so` are both
user-provisioned, not committed) — see README's Jetson build section for
the vendoring steps.
