# CMake generated Testfile for 
# Source directory: /Users/linyiyan/Project_Sentinel/test
# Build directory: /Users/linyiyan/Project_Sentinel/build_test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(SentinelCoreTest "/Users/linyiyan/Project_Sentinel/build_test/run_tests")
set_tests_properties(SentinelCoreTest PROPERTIES  _BACKTRACE_TRIPLES "/Users/linyiyan/Project_Sentinel/test/CMakeLists.txt;26;add_test;/Users/linyiyan/Project_Sentinel/test/CMakeLists.txt;0;")
add_test(RingBufferTest "/Users/linyiyan/Project_Sentinel/build_test/test_ring_buffer")
set_tests_properties(RingBufferTest PROPERTIES  _BACKTRACE_TRIPLES "/Users/linyiyan/Project_Sentinel/test/CMakeLists.txt;41;add_test;/Users/linyiyan/Project_Sentinel/test/CMakeLists.txt;0;")
add_test(HalUartTest "/Users/linyiyan/Project_Sentinel/build_test/test_hal_uart")
set_tests_properties(HalUartTest PROPERTIES  _BACKTRACE_TRIPLES "/Users/linyiyan/Project_Sentinel/test/CMakeLists.txt;62;add_test;/Users/linyiyan/Project_Sentinel/test/CMakeLists.txt;0;")
