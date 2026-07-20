# arboOCR

Standalone C++ OCR library — text **detection + orientation + recognition**
using PP-OCRv6 ONNX models via ONNXRuntime (CPU / CUDA / TensorRT). Extracted
from a benchmark harness into a clean, reusable library you can drop into your
own project.

Ported from [RapidOcrOnnx](https://github.com/RapidAI/RapidOcrOnnx)
(Apache-2.0) — see `THIRD_PARTY_NOTICES.md`.

## Quickstart

```cpp
#include <arboOCR/engine.hpp>
#include <iostream>

int main() {
    arbo::ocr::EngineConfig cfg;
    cfg.modelsDir = "models";
    cfg.useTensorrt = true; // auto-falls back to CUDA, then CPU

    arbo::ocr::Engine engine(cfg);
    auto page = engine.recognize("page.jpg");
    for (auto& line : page.lines)
        std::cout << line.text << " (" << line.score << ")\n";
}
```

Or try the bundled CLI without writing any C++:

```bash
./arboocr_demo --image page.jpg --models-dir models --model-type tiny
```

## Build

### Windows (vcpkg)

```powershell
$env:VCPKG_ROOT = "C:\vcpkg"
cmake --preset windows-x64
cmake --build build/windows-x64 --config Release
```

### Linux x64 (vcpkg)

```bash
export VCPKG_ROOT=/path/to/vcpkg
cmake --preset linux-x64
cmake --build build/linux-x64
```

### Jetson / aarch64 (system deps + CUDA/TensorRT onnxruntime)

vcpkg is impractical on a Jetson. Use apt packages + a fully self-contained
vendored onnxruntime (headers **and** the CUDA/TensorRT-enabled runtime
`.so`, both under `vendor/onnxruntime/`) — arboOCR doesn't depend on any
other project's Python venv once this is done:

```bash
sudo apt install -y libopencv-dev libcurl4-openssl-dev doctest-dev cxxopts-dev cmake build-essential

# 1. onnxruntime C++ headers (the pip wheel ships none). Use the latest
# release tarball whose headers are ABI-compatible with your runtime — e.g.
# v1.27.1 headers work against a 1.28.x runtime .so (the C API is stable):
mkdir -p vendor/onnxruntime && cd vendor/onnxruntime
curl -sL -o ort.tgz https://github.com/microsoft/onnxruntime/releases/download/v1.27.1/onnxruntime-linux-aarch64-1.27.1.tgz
tar xzf ort.tgz && rm ort.tgz && mv onnxruntime-linux-aarch64-1.27.1 dist

# 2. onnxruntime runtime .so with CUDA/TensorRT support. The official aarch64
# release tarball above is CPU-only — the pip wheel (`pip install onnxruntime`,
# or JetPack's preinstalled one) ships the accelerated build instead. Copy it
# in (adjust SRC to wherever onnxruntime is installed on your machine):
SRC=/path/to/your/onnxruntime/capi   # e.g. a venv's site-packages/onnxruntime/capi
mkdir -p lib
cp "$SRC"/libonnxruntime.so.* "$SRC"/libonnxruntime_providers_*.so lib/
ln -sf $(basename "$SRC"/libonnxruntime.so.*.*.*) lib/libonnxruntime.so.1
ln -sf $(basename "$SRC"/libonnxruntime.so.*.*.*) lib/libonnxruntime.so
cd ../..

cmake --preset jetson   # ARBOOCR_ORT_LIB_DIR defaults to vendor/onnxruntime/lib
cmake --build build/jetson -j$(nproc)
```

`Engine` auto-detects TensorRT then CUDA then CPU via
`Ort::GetAvailableProviders()`; `engine.backend()` reports what was selected.

## Models

arboOCR needs PP-OCRv6 ONNX files in `modelsDir`, named:

```
models/
├── PP-OCRv6_det.onnx              text detection
├── PP-OCRv6_cls.onnx              angle classification (only if useAngleCls)
├── PP-OCRv6_rec_tiny.onnx         text recognition (tiny | small | medium)
└── PP-OCRv6_rec_tiny_dict.txt     char dict (only if not embedded in ONNX metadata)
```

Two ways to provide them:

1. **Copy from an existing install** (e.g. a Python `rapidocr` package's
   `models/` dir) into `modelsDir`, renaming to the layout above.
2. **Download programmatically** with a base URL you control:

   ```cpp
   arbo::ocr::downloadOcrModels(
       "https://your-host.example/models/PP-OCRv6/", // baseUrl (you supply)
       "PP-OCRv6", "tiny", "models");
   ```

   arboOCR ships **no** default download URL — PP-OCR model hosting URLs are
   not stable, so the caller decides the source.

## API

- `arbo::ocr::Engine` — facade: construct with `EngineConfig`, call
  `recognize(path)` → `PagePrediction { image, lines[], elapsedMs }`.
- `arbo::ocr::Detector` / `Classifier` / `Recognizer` — the individual
  pipeline stages, exposed for custom pipelines.
- `arbo::ocr::downloadFile` / `downloadOcrModels` — optional model fetch
  helpers (libcurl).

## License

The ported OCR logic derives from RapidOcrOnnx (Apache-2.0) and bundles
Clipper (Boost Software License 1.0). See `THIRD_PARTY_NOTICES.md`.
