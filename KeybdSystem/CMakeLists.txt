# executable
add_executable(KeybdSystem WIN32
    KeybdSystem.cpp
    PluginFramework.cpp
    KeybdSystem_res.rc)
target_include_directories(KeybdSystem PRIVATE ..)
target_link_libraries(KeybdSystem comctl32 shlwapi imm32)
target_compile_definitions(KeybdSystem PRIVATE -DUNICODE -D_UNICODE)
