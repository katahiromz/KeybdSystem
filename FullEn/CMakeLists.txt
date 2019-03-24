# FullEn.keybd
add_library(FullEn SHARED FullEn.cpp FullEn.def FullEn_res.rc)
target_include_directories(FullEn PRIVATE ..)
target_link_libraries(FullEn imm32)
set_target_properties(FullEn PROPERTIES OUTPUT_NAME "FullEn.keybd")
set_target_properties(FullEn PROPERTIES PREFIX "")
set_target_properties(FullEn PROPERTIES SUFFIX "")
target_compile_definitions(FullEn PRIVATE -DPLUGIN_BUILD -DUNICODE -D_UNICODE)
