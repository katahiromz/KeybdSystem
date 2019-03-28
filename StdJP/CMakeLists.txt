# StdJP.keybd
add_library(StdJP SHARED StdJP.cpp StdJP.def StdJP_res.rc)
target_include_directories(StdJP PRIVATE ..)
target_link_libraries(StdJP imm32)
set_target_properties(StdJP PROPERTIES OUTPUT_NAME "StdJP.keybd")
set_target_properties(StdJP PROPERTIES PREFIX "")
set_target_properties(StdJP PROPERTIES SUFFIX "")
target_compile_definitions(StdJP PRIVATE -DPLUGIN_BUILD -DUNICODE -D_UNICODE)
