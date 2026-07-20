# Third-Party Notices

## RapidOcrOnnx

Source: https://github.com/RapidAI/RapidOcrOnnx
Commit: abd498c13a6dbe5f3b3c0d421d72e01bb3e6ee6d (2025-03-25)
License: Apache-2.0 (see vendor/rapidocronnx/LICENSE-Apache-2.0.txt)

arboOCR vendors and adapts DBNet box-decoding logic (including the Clipper
polygon-offset library), CRNN CTC-decoding logic, and AngleNet
classification logic from RapidOcrOnnx. The `Detector`, `Recognizer`, and
`Classifier` classes are near-verbatim ports of RapidOcrOnnx's `DbNet`,
`CrnnNet`, and `AngleNet`.

## Clipper (via RapidOcrOnnx)

Source: bundled in RapidOcrOnnx, originally by Angus Johnson
License: Boost Software License 1.0
