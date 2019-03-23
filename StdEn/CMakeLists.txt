# StdEn.keybd
add_library(StdEn SHARED StdEn.cpp StdEn.def StdEn_res.rc)
target_include_directories(StdEn PRIVATE ..)
target_link_libraries(StdEn imm32)
set_target_properties(StdEn PROPERTIES OUTPUT_NAME "StdEn.keybd")
set_target_properties(StdEn PROPERTIES PREFIX "")
set_target_properties(StdEn PROPERTIES SUFFIX "")
target_compile_definitions(StdEn PRIVATE -DPLUGIN_BUILD -DUNICODE -D_UNICODE)
