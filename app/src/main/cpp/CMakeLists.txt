# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include_directories("${CMAKE_SOURCE_DIR}/thirdParty/include/ffmpeg")
include_directories("${CMAKE_SOURCE_DIR}/thirdParty/include")
include_directories("${CMAKE_SOURCE_DIR}/thirdParty/include/openfec")
include_directories("${CMAKE_SOURCE_DIR}/thirdParty/include/openfec/lib_common")



set(my_nativelib_path ${CMAKE_SOURCE_DIR}/../jniLibs)
link_directories("${my_nativelib_path}/${ANDROID_ABI}")

add_library( # Sets the name of the library.
        native-lib
        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native-lib.cpp
        FRtpDecoder.cpp
        FRtpDecoder.h
        GLCoder.cpp
        GLCoder.h
        myrtspsenderv2.h
        myrtspsenderv2.cpp
        myvideodecoder.h
        myvideodecoder.cpp
        )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib
        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)


# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        native-lib
        android

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        ipprotocol jrtp openfec rtspclient rtsppusher
        avcodec
        avfilter
        avformat
        avutil
        postproc
        swresample
        swscale

        GLESv3
        EGL
        )
