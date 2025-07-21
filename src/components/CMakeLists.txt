target_sources(app PRIVATE button_module.c event_module.c)

target_include_directories(app PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)