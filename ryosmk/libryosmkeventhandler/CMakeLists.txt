SET(SOURCES
  ryos_dbus_server.c
  ryos_eventhandler.c
  ryos_eventhandler_channel.c
  ryos_ripple.c
  ryos_ripple_c.c
)

SET(LIBS
  libroccat
  libroccatryosmk
  libroccathelper
  ${GAMINGGEAR0_LIBRARY}
  ${DBUS-GLIB_LIBRARIES}
  ${GLIB2_LIBRARIES}
)

IF(LUA_FOUND)
  LIST(APPEND LIBS ${LUA_LIBRARY})
  LIST(APPEND SOURCES ryos_ripple_lua.c)
  INCLUDE_DIRECTORIES(${LUA_INCLUDE_DIR})
ENDIF()

ADD_LIBRARY(libryosmkeventhandler SHARED ${SOURCES})

ADD_DEPENDENCIES(libryosmkeventhandler
  generated_headers
  ryosmk_dbus_bindings
)

SET_TARGET_PROPERTIES(libryosmkeventhandler
  PROPERTIES
    OUTPUT_NAME ryosmkeventhandler
    COMPILE_FLAGS "-DG_LOG_DOMAIN=\\\"RyosMKEventhandler\\\" -fvisibility=hidden"
)

TARGET_LINK_LIBRARIES(libryosmkeventhandler ${LIBS})

INSTALL(TARGETS libryosmkeventhandler DESTINATION ${EVENTHANDLER_PLUGIN_DIR})
