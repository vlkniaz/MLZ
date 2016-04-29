project (IMLib)

cmake_minimum_required (VERSION 2.8.11)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

set(SOURCE IMTIFFImageFileFormat.cpp IMImageResizer.cpp IMImageFileFormat.cpp IMVideoServer.cpp IMVideoClient.cpp IMImageFile.cpp IMBMPImageFileFormat.cpp IMLibTypes.cpp MAMatrix.cpp PHCamera.cpp PHOrientation.cpp)

set(HEADER
	IMLib/IMLib.h
	IMLib/IMTIFFImageFileFormat.h
	IMLib/IMImageResizer.h
	IMLib/IMImageFileFormat.h
	IMLib/IMVideoServer.h
	IMLib/IMVideoClient.h
	IMLib/IMImageFile.h
	IMLib/IMBMPImageFileFormat.h
	IMLib/IMLibTypes.h
	IMLib/MAMatrix.h
	IMLib/PHCamera.h
	IMLib/PHOrientation.h
	)

# Create a library called "Hello" which includes the source file "hello.cxx".
# The extension is already found. Any number of sources could be listed here.
add_library (IMLib ${SOURCE} ${HEADER})

find_package (TIFF REQUIRED)
include_directories(${TIFF_INCLUDE_DIR})
set(LIBS ${LIBS} ${TIFF_LIBRARIES})

target_link_libraries(IMLib ${LIBS})

# Make sure the compiler can find include files for our Hello library
# when other libraries or executables link to Hello
target_include_directories (IMLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

MESSAGE("IMLib")

install (TARGETS IMLib DESTINATION lib)
install (FILES ${HEADER} DESTINATION include/IMLib)
install(FILES IMLibConfig.cmake DESTINATION lib/IMLib)

project (MLZ)

cmake_minimum_required (VERSION 2.8.11)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

add_executable (MLZ MLZ/main.cpp)

find_package (IMLIB REQUIRED)
target_link_libraries (MLZ LINK_PUBLIC ${IMLIB_LIBRARY})
include_directories(${IMLIB_INCLUDE_DIR})

add_executable (GraphBuilder MLZ/graphBuilder.cpp)
target_link_libraries (GraphBuilder LINK_PUBLIC ${IMLIB_LIBRARY})

# copy camera parameters file
file(COPY _impara01.txt DESTINATION Debug)