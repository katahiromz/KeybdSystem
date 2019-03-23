# SimpleNumPad.keybd
add_library(SimpleNumPad SHARED SimpleNumPad.cpp SimpleNumPad.def SimpleNumPad_res.rc)
target_include_directories(SimpleNumPad PRIVATE ..)
target_link_libraries(SimpleNumPad imm32)
set_target_properties(SimpleNumPad PROPERTIES OUTPUT_NAME "SimpleNumPad.keybd")
set_target_properties(SimpleNumPad PROPERTIES PREFIX "")
set_target_properties(SimpleNumPad PROPERTIES SUFFIX "")
target_compile_definitions(SimpleNumPad PRIVATE -DPLUGIN_BUILD -DUNICODE -D_UNICODE)
