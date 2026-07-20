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

vcpkg is impractical on a Jetson. Use apt packages + a vendored onnxruntime
header set, and point the build at a CUDA/TensorRT-enabled `libonnxruntime.so`
(e.g. the one shipped in a `rapidocr`/onnxruntime Python install):

```bash
sudo apt install -y libopencv-dev libcurl4-openssl-dev doctest-dev cxxopts-dev cmake build-essential

# onnxruntime C++ headers (the pip wheel ships none) — match your runtime version:
mkdir -p vendor/onnxruntime && cd vendor/onnxruntime
curl -sL -o ort.tgz https://github.com/microsoft/onnxruntime/releases/download/v1.28.0/onnxruntime-linux-aarch64-1.28.0.tgz
tar xzf ort.tgz && rm ort.tgz && mv onnxruntime-linux-aarch64-1.28.0 dist
cd ../..

cmake --preset jetson \
  -DARBOOCR_ORT_LIB_DIR=/path/to/onnxruntime/lib/dir
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
