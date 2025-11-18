add_library(format tokenizer.cpp unwrapped_line.cpp cst.cpp tokens.cpp)
set_target_properties(format PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(format INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)