set (SRC dyna-joiner-main.cpp)

add_executable(dyna-joiner ${SRC})
target_link_libraries(dyna-joiner da)

if (MSVC)
  set_target_properties(dyna-joiner PROPERTIES FOLDER "${DYNA_TOOLS_FOLDER}")
endif ()

if (Dyna_install)
  install (TARGETS dyna-joiner)
endif ()
