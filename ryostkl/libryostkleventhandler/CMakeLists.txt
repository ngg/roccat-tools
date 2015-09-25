SET(SOURCES
  ryostkl_dbus_server.c
  ryostkl_eventhandler.c
  ryostkl_eventhandler_channel.c
  ryostkl_ripple.c
  ryostkl_ripple_c.c
)

SET(LIBS
  libroccat
  libroccatryostkl
  libroccatryosmk
  libroccathelper
  ${GAMINGGEAR0_LIBRARY}
  ${DBUS-GLIB_LIBRARIES}
  ${GLIB2_LIBRARIES}
)

IF(LUA_FOUND)
  LIST(APPEND LIBS ${LUA_LIBRARY})
  LIST(APPEND SOURCES ryostkl_ripple_lua.c)
  INCLUDE_DIRECTORIES(${LUA_INCLUDE_DIR})
ENDIF()

ADD_LIBRARY(libryostkleventhandler SHARED ${SOURCES})

ADD_DEPENDENCIES(libryostkleventhandler
  generated_headers
  ryostkl_dbus_bindings
)

SET_TARGET_PROPERTIES(libryostkleventhandler
  PROPERTIES
    OUTPUT_NAME ryostkleventhandler
    COMPILE_FLAGS "-DG_LOG_DOMAIN=\\\"RyosTKLEventhandler\\\" -fvisibility=hidden"
)

TARGET_LINK_LIBRARIES(libryostkleventhandler ${LIBS})

INSTALL(TARGETS libryostkleventhandler DESTINATION ${EVENTHANDLER_PLUGIN_DIR})
