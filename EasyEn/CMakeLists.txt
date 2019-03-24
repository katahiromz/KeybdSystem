# EasyEn.keybd
add_library(EasyEn SHARED EasyEn.cpp EasyEn.def EasyEn_res.rc)
target_include_directories(EasyEn PRIVATE ..)
target_link_libraries(EasyEn imm32)
set_target_properties(EasyEn PROPERTIES OUTPUT_NAME "EasyEn.keybd")
set_target_properties(EasyEn PROPERTIES PREFIX "")
set_target_properties(EasyEn PROPERTIES SUFFIX "")
target_compile_definitions(EasyEn PRIVATE -DPLUGIN_BUILD -DUNICODE -D_UNICODE)
