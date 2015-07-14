SET(PROJECT hpack-headers-tests)
PROJECT(${PROJECT})

# Unit tests
IF(HPACK_ALLOW_TESTS)
    INCLUDE(DownloadGoogleTesting)
    download_google_testing()

    INCLUDE_DIRECTORIES(
        SYSTEM ${GTEST_INCLUDE_DIR}
        SYSTEM ${GMOCK_INCLUDE_DIR})

    LINK_DIRECTORIES(${GTEST_BINARY_DIR} ${GMOCK_BINARY_DIR})

    INCLUDE_DIRECTORIES(
        ${CMAKE_CURRENT_SOURCE_DIR}/../include)

    ADD_EXECUTABLE(hpack-headers-tests
        ${CMAKE_CURRENT_SOURCE_DIR}/unit/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/unit/header.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/unit/header_table.cpp)

    ADD_DEPENDENCIES(hpack-headers-tests googlemock)

    TARGET_LINK_LIBRARIES(hpack-headers-tests
        hpack-headers
        gmock
        gtest)

    SET_TARGET_PROPERTIES(hpack-headers-tests PROPERTIES
    COMPILE_FLAGS "-std=c++0x -W -Wall -Werror -pedantic")
ENDIF()
