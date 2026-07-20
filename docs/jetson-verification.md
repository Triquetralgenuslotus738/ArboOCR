# Jetson Verification (2026-07-20)

Verified on `jetson-nano-ssh` (aarch64, Ubuntu 24.04 / L4T, GNU 13.3.0,
onnxruntime 1.28.0 runtime from `ocr-benchmark/.venv`, TensorRT EP available).

## What was done

- Transferred the source tree, installed apt deps (already present:
  `libopencv-dev`, `libcurl4-openssl-dev`, `doctest-dev`, `libcxxopts-dev`,
  `cmake`, `build-essential`).
- Vendored onnxruntime **v1.27.1** aarch64 headers (v1.28.0 has no GitHub
  release; 1.27.1 headers are ABI-compatible with the 1.28.0 runtime `.so`).
- Configured + built the `jetson` preset with
  `-DARBOOCR_ORT_LIB_DIR=/home/nvidia/ocr-benchmark/.venv/lib/python3.12/site-packages/onnxruntime/capi`.
- Provisioned **tiny models only** (`PP-OCRv6_det_tiny` → `PP-OCRv6_det.onnx`,
  `PP-OCRv6_rec_tiny`) — deliberately small so TRT engine build stays quick.

## Results

Demo on the bundled Indonesian receipt (`arboocr_demo`, tiny models):

| Backend | Lines | Time (engine cached) |
|---|---|---|
| cpu | 31 | 751 ms |
| tensorrt | 31 | 364 ms |

Both backends produce identical text output. First TensorRT run compiles +
caches the engine (slow, minutes on a Nano); subsequent runs use the cache.

Unit tests: `17 passed | 0 failed | 1 skipped` (the skipped case is the
optional real-model inference test).

## Runtime note

The demo/tests need the onnxruntime runtime `.so` on the library path:
`LD_LIBRARY_PATH=/home/nvidia/ocr-benchmark/.venv/lib/python3.12/site-packages/onnxruntime/capi`.
The build already bakes this dir into the binaries' RPATH via
`ARBOOCR_ORT_LIB_DIR`, so `LD_LIBRARY_PATH` is only needed if that dir moves.
