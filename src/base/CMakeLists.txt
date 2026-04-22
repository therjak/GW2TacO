add_library(taco_base STATIC)
source_group("Header Files" FILES
    assert.h
    color.h
    enum_helpers.h
    file_list.h
    hasher.h
    image_decompressor.h
    lock_free_queue.h
    logger.h
    ring_buffer.h
    socket.h
    stream_reader.h
    stream_writer.h
    string_format.h
    timer.h
)

target_sources(taco_base
    PRIVATE
        assert.h
        color.h
        enum_helpers.h
        file_list.cpp
        file_list.h
        hasher.cpp
        hasher.h
        image_decompressor.cpp
        image_decompressor.h
        lock_free_queue.h
        logger.cpp
        logger.h
        matrix.cpp
        ring_buffer.h
        socket.cpp
        socket.h
        spec_math.cpp
        stream_reader.cpp
        stream_reader.h
        stream_writer.cpp
        stream_writer.h
        string_format.cpp
        string_format.h
        timer.cpp
        timer.h
        vector.cpp
    PUBLIC
        FILE_SET modules TYPE CXX_MODULES
        FILES
            constants.ixx
            math.ixx
            matrix.ixx
            plane.ixx
            rectangle.ixx
            spec_math.ixx
            sphere.ixx
            vector.ixx
            time.ixx
)

target_include_directories(taco_base PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../..")
use_props(taco_base "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
target_compile_definitions(taco_base PUBLIC ${Common_Definitions})

add_executable(Base_Tests
    math_test.cpp
)

target_link_libraries(Base_Tests
    PRIVATE
        gtest_main
        taco_base
)
use_props(Base_Tests "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")

include(GoogleTest)
gtest_discover_tests(Base_Tests)
