// tests/test_recognizer.cpp
#include <doctest/doctest.h>
#include "arboOCR/recognizer.hpp"

using namespace arbo::ocr;

TEST_CASE("loadKeysFromFile loads keys and prepends blank + appends space") {
    Recognizer net;
    net.loadKeysFromFile("tests/fixtures/sample_keys.txt");
    CHECK(net.keyCount() == 7); // 5 chars + blank prefix + space suffix
}

TEST_CASE("loadKeysFromFile on a missing file leaves keys empty, does not throw") {
    Recognizer net;
    net.loadKeysFromFile("tests/fixtures/does_not_exist.txt");
    CHECK(net.keyCount() == 0);
}

TEST_CASE("loadKeysFromModelMetadata without a loaded model returns false") {
    Recognizer net; // never loadModel()
    CHECK(net.loadKeysFromModelMetadata() == false);
    CHECK(net.keyCount() == 0);
}

TEST_CASE("CTC greedy decode collapses repeats and skips blank index 0") {
    Recognizer net;
    net.loadKeysFromFile("tests/fixtures/sample_keys.txt");
    // keys = ["#", "a", "b", "c", "d", "e", " "]  (indices 0..6)
    std::vector<float> output(6 * 7, 0.0f);
    auto setArgmax = [&](int t, int idx) { output[t * 7 + idx] = 1.0f; };
    setArgmax(0, 1); // a
    setArgmax(1, 1); // a (repeat, collapsed)
    setArgmax(2, 2); // b
    setArgmax(3, 0); // blank
    setArgmax(4, 3); // c
    setArgmax(5, 3); // c (repeat, collapsed)
    auto line = net.decodeForTest(output, 6, 7);
    CHECK(line.text == "abc");
}
