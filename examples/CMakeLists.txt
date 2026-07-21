# Buildable, runnable usage examples — separate from cli/arboocr_demo.cpp
# (which is a full-featured CLI tool) and from the code snippets in
# README.md (which aren't compiled, just illustrative). These are.

add_executable(arboocr_example_basic basic_recognize.cpp)
target_link_libraries(arboocr_example_basic PRIVATE arboOCR)

add_executable(arboocr_example_custom_pipeline custom_pipeline.cpp)
target_link_libraries(arboocr_example_custom_pipeline PRIVATE arboOCR)

if(ARBOOCR_USE_SYSTEM_DEPS)
    foreach(tgt arboocr_example_basic arboocr_example_custom_pipeline)
        set_target_properties(${tgt} PROPERTIES
            INSTALL_RPATH "${ARBOOCR_ORT_LIB_DIR}" BUILD_WITH_INSTALL_RPATH TRUE)
    endforeach()
endif()
