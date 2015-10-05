CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(Services
        "application"
        "base"
        "content"
        "location"
        "media"
        "messaging"
        "network"
        "social"
        "telephony"
        "system"
   )

# project
SET(project_prefix "capi")
SET(prefix "/usr")
SET(version "0.0.1")
SET(maintainer "Seungkeun Lee <sngn.lee@samsung.com>, Kangho Hur<kagho.hur@samsung.com>")
SET(description "A Camera library in Tizen Native API")
SET(service "media")
SET(submodule "recorder")

# for package file
SET(dependents "dlog mm-camcorder capi-media-camera capi-media-audio-io")
SET(pc_dependents "capi-base-common capi-media-camera capi-media-audio-io")

SET(fw_name "${project_prefix}-${service}-${submodule}")

PROJECT(${fw_name})

SET(CMAKE_INSTALL_PREFIX ${prefix})
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(VERSION ${version})

SET(INC_DIR include)
INCLUDE_DIRECTORIES(${INC_DIR})

INCLUDE(FindPkgConfig)
pkg_check_modules(${fw_name} REQUIRED ${dependents})
FOREACH(flag ${${fw_name}_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -fPIC -Wall -Wextra -Wno-array-bounds -Wno-empty-body -Wno-ignored-qualifiers -Wno-unused-parameter -Wshadow -Wwrite-strings -Wswitch-default -Wno-unused-but-set-parameter -Wno-unused-but-set-variable")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g")

IF("${ARCH}" STREQUAL "arm")
    ADD_DEFINITIONS("-DTARGET")
ENDIF("${ARCH}" STREQUAL "arm")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DTIZEN_DEBUG")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -Wl,--rpath=${LIB_INSTALL_DIR}")

aux_source_directory(src SOURCES)
ADD_LIBRARY(${fw_name} SHARED ${SOURCES})

TARGET_LINK_LIBRARIES(${fw_name} ${${fw_name}_LDFLAGS})

SET_TARGET_PROPERTIES(${fw_name}
     PROPERTIES
     VERSION ${FULLVER}
     SOVERSION ${MAJORVER}
     CLEAN_DIRECT_OUTPUT 1
)

INSTALL(TARGETS ${fw_name} DESTINATION ${LIB_INSTALL_DIR})
INSTALL(
        DIRECTORY ${INC_DIR}/ DESTINATION include/${service}
        FILES_MATCHING
        PATTERN "*_private.h" EXCLUDE
        PATTERN "${INC_DIR}/*.h"
        )

SET(PC_NAME ${fw_name})
SET(PC_REQUIRED ${pc_dependents})
SET(PC_LDFLAGS -l${fw_name})

CONFIGURE_FILE(
    ${fw_name}.pc.in
    ${CMAKE_CURRENT_SOURCE_DIR}/${fw_name}.pc
    @ONLY
)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${fw_name}.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)

#ADD_SUBDIRECTORY(test)

IF(UNIX)

ADD_CUSTOM_TARGET (distclean @echo cleaning for source distribution)
ADD_CUSTOM_COMMAND(
        DEPENDS clean
        COMMENT "distribution clean"
        COMMAND find
        ARGS    .
        -not -name config.cmake -and \(
        -name tester.c -or
        -name Testing -or
        -name CMakeFiles -or
        -name cmake.depends -or
        -name cmake.check_depends -or
        -name CMakeCache.txt -or
        -name cmake.check_cache -or
        -name *.cmake -or
        -name Makefile -or
        -name core -or
        -name core.* -or
        -name gmon.out -or
        -name install_manifest.txt -or
        -name *.pc -or
        -name *~ \)
        | grep -v TC | xargs rm -rf
        TARGET  distclean
        VERBATIM
)

ENDIF(UNIX)
