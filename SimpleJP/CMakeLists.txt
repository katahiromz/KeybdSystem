# SimpleJP.keybd
add_library(SimpleJP SHARED SimpleJP.cpp SimpleJP.def SimpleJP_res.rc)
target_include_directories(SimpleJP PRIVATE ..)
target_link_libraries(SimpleJP imm32)
set_target_properties(SimpleJP PROPERTIES OUTPUT_NAME "SimpleJP.keybd")
set_target_properties(SimpleJP PROPERTIES PREFIX "")
set_target_properties(SimpleJP PROPERTIES SUFFIX "")
target_compile_definitions(SimpleJP PRIVATE -DPLUGIN_BUILD -DUNICODE -D_UNICODE)
