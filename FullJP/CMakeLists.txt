# FullJP.keybd
add_library(FullJP SHARED FullJP.cpp FullJP.def FullJP_res.rc)
target_include_directories(FullJP PRIVATE ..)
target_link_libraries(FullJP imm32)
set_target_properties(FullJP PROPERTIES OUTPUT_NAME "FullJP.keybd")
set_target_properties(FullJP PROPERTIES PREFIX "")
set_target_properties(FullJP PROPERTIES SUFFIX "")
target_compile_definitions(FullJP PRIVATE -DPLUGIN_BUILD -DUNICODE -D_UNICODE)
