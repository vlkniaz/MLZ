project (IMLibProject)

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

set(SOURCE 
	IMTIFFImageFileFormat.cpp
	IMImageResizer.cpp
	IMImageConverter.cpp
	IMImageFileFormat.cpp
	IMVideoServer.cpp
	IMVideoClient.cpp
	IMImageFile.cpp
	IMBMPImageFileFormat.cpp
	IMLibTypes.cpp 
	MAMatrix.cpp
	PHCamera.cpp 
	PHOrientation.cpp
	PHQuickHull.cpp
	PHMesh.cpp
	IMMotionAnalyser.cpp
	deps/tiny_obj/tiny_obj_loader.cc
	)

set(HEADER
	IMLib/IMLib.h
	IMLib/IMTIFFImageFileFormat.h
	IMLib/IMImageResizer.h
	IMLib/IMImageConverter.h
	IMLib/IMImageFileFormat.h
	IMLib/IMVideoServer.h
	IMLib/IMVideoClient.h
	IMLib/IMImageFile.h
	IMLib/IMBMPImageFileFormat.h
	IMLib/IMLibTypes.h
	IMLib/MAMatrix.h
	IMLib/PHCamera.h
	IMLib/PHOrientation.h
	IMLib/PHQuickHull.h
	IMLib/PHMesh.h
	IMLib/IMMotionAnalyser.h
	deps/tiny_obj/tiny_obj_loader.h
	)

# Create a library called "Hello" which includes the source file "hello.cxx".
# The extension is already found. Any number of sources could be listed here.
add_library (IMLib ${SOURCE} ${HEADER})

find_package (TIFF REQUIRED)
include_directories(${TIFF_INCLUDE_DIR})

find_library(FFTW_LIBRARY NAMES fftw3 fftw)
set(FFTW_LIBRARIES "${FFTW_LIBRARY}")

set(LIBS ${LIBS} ${TIFF_LIBRARIES} ${FFTW_LIBRARIES})

target_link_libraries(IMLib ${LIBS})

# Make sure the compiler can find include files for our Hello library
# when other libraries or executables link to Hello
target_include_directories (IMLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

MESSAGE("IMLib")

install (TARGETS IMLib 
		ARCHIVE DESTINATION lib)
		
install (FILES ${HEADER} DESTINATION include/IMLib)
install(FILES IMLibConfig.cmake DESTINATION lib/IMLib)

# project MLZ

add_executable (MLZ EXCLUDE_FROM_ALL MLZ/main.cpp)

#find_package (IMLIB REQUIRED)
target_link_libraries (MLZ LINK_PUBLIC ${IMLIB_LIBRARY})
include_directories(${IMLIB_INCLUDE_DIR})

add_executable (GraphBuilder EXCLUDE_FROM_ALL MLZ/graphBuilder.cpp)
target_link_libraries (GraphBuilder LINK_PUBLIC ${IMLIB_LIBRARY})

# copy camera parameters file
file(COPY _impara01.txt DESTINATION Debug)

# project ZFlow

add_executable (ZFlow EXCLUDE_FROM_ALL ZFlow/main.cpp)

#find_package (IMLIB REQUIRED)
target_link_libraries (ZFlow LINK_PUBLIC ${IMLIB_LIBRARY} ${LIBS})
include_directories(${IMLIB_INCLUDE_DIR})

add_dependencies(ZFlow IMLib)

# project Dithering

add_executable (Dithering EXCLUDE_FROM_ALL Dithering/main.cpp)

#find_package (IMLIB REQUIRED)
target_link_libraries (Dithering LINK_PUBLIC IMLib ${LIBS})
target_include_directories(Dithering PUBLIC .)

add_dependencies(Dithering IMLib)

# project QuickHull

set(QUICKHULL_SOURCE 
	IMLibTypes.cpp 
	MAMatrix.cpp
	PHCamera.cpp 
	PHOrientation.cpp
	PHQuickHull.cpp
	PHConvexHull.cpp
	PHMesh.cpp
	deps/tiny_obj/tiny_obj_loader.cc
	)

set(QUICKHULL_HEADER 
	IMLib/IMLibTypes.h 
	IMLib/MAMatrix.h
	IMLib/PHCamera.h
	IMLib/PHOrientation.h
	IMLib/PHQuickHull.h
	IMLib/PHConvexHull.h
	IMLib/PHMesh.h
	deps/tiny_obj/tiny_obj_loader.h
	)

add_executable (QuickHull EXCLUDE_FROM_ALL Tests/QuickHull/main.cpp ${QUICKHULL_SOURCE} ${QUICKHULL_HEADER})

#find_package (IMLIB REQUIRED)
#target_link_libraries (QuickHull LINK_PUBLIC IMLib ${LIBS})
target_include_directories(QuickHull PUBLIC .)
include_directories(deps/tiny_obj/)

#add_dependencies(QuickHull IMLib)