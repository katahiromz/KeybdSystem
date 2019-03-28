# EasyJP.keybd
add_library(EasyJP SHARED EasyJP.cpp EasyJP.def EasyJP_res.rc)
target_include_directories(EasyJP PRIVATE ..)
target_link_libraries(EasyJP imm32)
set_target_properties(EasyJP PROPERTIES OUTPUT_NAME "EasyJP.keybd")
set_target_properties(EasyJP PROPERTIES PREFIX "")
set_target_properties(EasyJP PROPERTIES SUFFIX "")
target_compile_definitions(EasyJP PRIVATE -DPLUGIN_BUILD -DUNICODE -D_UNICODE)
