enable_language(CXX)

message(STATUS "Compiling cpp tests...")

add_executable(test_cpp_modular test_cpp.cpp)
target_link_libraries(test_cpp_modular PRIVATE controlly)

add_test(NAME CppLinkage_Modular COMMAND test_cpp_modular)

add_executable(test_cpp_single test_cpp.cpp)
target_link_libraries(test_cpp_single PRIVATE controlly)
target_compile_definitions(test_cpp_single PRIVATE HEADERLIB_TEST)
target_include_directories(test_cpp_single PRIVATE ${PROJECT_SOURCE_DIR}/dist)

add_test(NAME CppLinkage_Single COMMAND test_cpp_single)
