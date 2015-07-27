CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/foreign/modules")
PROJECT(HPACK-HEADERS)

SET(HPACKHEADERS_VERSION_MAJOR 0)
SET(HPACKHEADERS_VERSION_MINOR 1)
SET(HPACKHEADERS_VERSION_PATCH 0)
SET(HPACKHEADERS_VERSION "${HPACKHEADERS_VERSION_MAJOR}.${HPACKHEADERS_VERSION_MINOR}.${HPACKHEADERS_VERSION_PATCH}")

IF(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
    SET(HPACK_IS_MODULE FALSE)
ELSE()
    SET(HPACK_IS_MODULE TRUE)
ENDIF()
# Evaluate ABI version number.
MATH(EXPR HPACKHEADERS_ABI_VERSION_SUFFIX "${HPACKHEADERS_VERSION_MAJOR} * 10000 + ${HPACKHEADERS_VERSION_MINOR} * 100 + ${HPACKHEADERS_VERSION_PATCH} * 1")

SET(HPACKHEADERS_ABI_VERSION v${HPACKHEADERS_ABI_VERSION_SUFFIX})

CONFIGURE_FILE(
    ${PROJECT_SOURCE_DIR}/config.hpp.in
    ${PROJECT_SOURCE_DIR}/include/hpack-headers/config.hpp
)

if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
endif()

IF(HPACK_IS_MODULE)
    OPTION(HPACK_ALLOW_TESTS "Build Tests" OFF)
ELSE()
    OPTION(HPACK_ALLOW_TESTS "Build Tests" ON)
ENDIF()

# Import our CMake modules
INCLUDE(foreign/modules/AddCXXCompilerFlag.cmake)

FIND_PACKAGE(Boost 1.46.0
    REQUIRED
)

FIND_PACKAGE(Threads)

INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
)

INCLUDE_DIRECTORIES(BEFORE
    ${PROJECT_SOURCE_DIR}/include
)

LINK_DIRECTORIES(
    ${Boost_LIBRARY_DIRS}
)

ADD_LIBRARY(hpack-headers SHARED
    src/header.cpp
)

ADD_LIBRARY(hpack-headers-static STATIC
    src/header.cpp
)

TARGET_LINK_LIBRARIES(hpack-headers ${CMAKE_THREAD_LIBS_INIT})
TARGET_LINK_LIBRARIES(hpack-headers-static ${CMAKE_THREAD_LIBS_INIT})

# Try and enable C++11.
add_cxx_compiler_flag(-std=c++11)
IF(NOT HAVE_CXX_FLAG_STD_CXX11)
  add_cxx_compiler_flag(-std=c++0x)
ENDIF()

# Turn compiler warnings.
add_cxx_compiler_flag(-Wall)
add_cxx_compiler_flag(-Wextra)
add_cxx_compiler_flag(-Werror RELEASE)
add_cxx_compiler_flag(-pedantic)
add_cxx_compiler_flag(-pedantic-errors)
add_cxx_compiler_flag(-fpic)

IF(HPACK_ALLOW_TESTS)
    ENABLE_TESTING()
    ADD_SUBDIRECTORY(tests)
    ADD_TEST(unit_test COMMAND ${CMAKE_BINARY_DIR}/tests/hpack-headers-tests )
ENDIF()

IF(!HPACK_IS_MODULE)
    INSTALL(
        TARGETS
            hpack-headers
        LIBRARY DESTINATION lib
    )

    INSTALL(
        DIRECTORY
            include/
        DESTINATION include
        COMPONENT development
    )
ENDIF()
