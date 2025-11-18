add_executable(test_tokenizer tokenizer.test.cpp)
target_link_libraries(test_tokenizer PRIVATE format)
add_test(NAME test_tokenizer COMMAND test_tokenizer)

add_executable(test_unwrapped_line unwrapped_line.test.cpp)
target_link_libraries(test_unwrapped_line PRIVATE format)
add_test(NAME test_unwrapped_line COMMAND test_unwrapped_line)

add_executable(test_cst cst.test.cpp)
target_link_libraries(test_cst PRIVATE format)
add_test(NAME test_cst COMMAND test_cst)

add_executable(test_tokens tokens.test.cpp)
target_link_libraries(test_tokens PRIVATE format)
add_test(NAME test_tokens COMMAND test_tokens)

add_executable(test_cst_visitor cst_visitor.test.cpp)
target_link_libraries(test_cst_visitor PRIVATE format)
add_test(NAME test_cst_visitor COMMAND test_cst_visitor)
