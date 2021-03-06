cmake_minimum_required( VERSION 2.8.11 )
set (objectName sdrplay-radio)
set (CMAKE_CXX_FLAGS "${CMAKE_XCC_FLAGS} -Wall -std=c++11 -flto")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "/usr/local/bin" CACHE PATH "default install path" FORCE )
endif()
#set (CMAKE_INSTALL_PREFIX /usr/local/bin)

if(MINGW)
    add_definitions ( -municode)
endif()

########################################################################
# select the release build type by default to get optimization flags
########################################################################
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
   message(STATUS "Build type not specified: defaulting to release.")
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")

### make sure our local CMake Modules path comes first
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake/Modules)

######################################################################
#       R E A D T H I S
#####################################################################

if(DEFINED AIRSPY)
   set(AIRSPY true)
   set (objectName airspy-radio)
endif ()

if(DEFINED SDRPLAY)
   set(SDRPLAY true)
   set (objectName sdrplay-radio)
endif ()

########################################################################
	find_package (PkgConfig)

	find_package(FFTW3f)
	if (NOT FFTW3F_FOUND)
	    message(FATAL_ERROR "please install FFTW3")
	endif ()


	find_library (PTHREADS pthread)
	if (NOT(PTHREADS))
	   message (FATAL_ERROR "please install libpthread")
	else (NOT(PTHREADS))
	   set (extraLibs ${extraLibs} ${PTHREADS})
	endif (NOT(PTHREADS))

	find_package(LibSndFile)
	if (NOT LIBSNDFILE_FOUND)
	    message(FATAL_ERROR "please install libsndfile")
	endif ()
	list(APPEND extraLibs ${LIBSNDFILE_LIBRARY})

	find_package(LibSampleRate)
	if (NOT LIBSAMPLERATE_FOUND)
	    message(FATAL_ERROR "please install libsamplerate")
	endif ()
	list(APPEND extraLibs ${LIBSAMPLERATE_LIBRARY})


#######################################################################
#	The devices
#
	if (SDRPLAY)
	   find_path (SDRPLAYLIB_INCLUDE_DIR
                      NAMES mirsdrapi-rsp.h
                      PATHS
                      /usr/local/include/
	   )
           include_directories (${SDRPLAYLIB_INCLUDE_DIR})

	   include_directories (
	     ./devices/sdrplay-handler
	   )

	   set ($(objectName)_HDRS
	        ${${objectName}_HDRS}
	        ./devices/sdrplay-handler/sdrplay-handler.h
           )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./devices/sdrplay-handler/sdrplay-handler.cpp
	   )

	 add_definitions (-DHAVE_SDRPLAY)
	endif (SDRPLAY)

	if (AIRSPY)
	   find_path (AIRSPYLIB_INCLUDE_DIR
                      NAMES airspy.h
                      PATHS
                      /usr/local/include/
                      /usr/local/include/libairspy
           )
           include_directories (${AIRSPYLIB_INCLUDE_DIR})
           find_library (AIRSPYLIB airspy)
           if(NOT(AIRSPYLIB))
              message (FATAL_ERROR "please install library for airspy")
           else(NOT(AIRSPYLIB))
	      list (APPEND extraLibs ${AIRSPYLIB})
	   endif ()

	   include_directories (
	     ./devices/airspy-handler
	   )

	   set ($(objectName)_HDRS
	        ${${objectName}_HDRS}
	        ./devices/airspy-handler/airspy-handler.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./devices/airspy-handler/airspy-handler.cpp
	   )

	   add_definitions (-DHAVE_AIRSPY)
	endif (AIRSPY)

	if (RTLSDR)
           find_package(LibRTLSDR)
           if (NOT LIBRTLSDR_FOUND)
               message(FATAL_ERROR "please install librtlsdr")
           endif ()
	   ###include_directories (${RTLSDR_INCLUDE_DIR})

	   include_directories (
	        ../devices/rtlsdr-handler/
	   )

	   set (${objectName}_HDRS
	        ${${objectName}_HDRS}
	        ../devices/rtlsdr-handler/rtlsdr-handler.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ../devices/rtlsdr-handler/rtlsdr-handler.cpp
	   )

	   add_definitions (-DHAVE_RTLSDR)
	endif()
#
#	Here we really start

	include_directories (
	           ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	           .
	           ./
	           ./handlers
	           ./various
	           ./devices
	           /usr/include/
	)

	set (${objectName}_HDRS
	     ./radio-constants.h
	     ./radio-processor.h
	     ./handlers/radio-handler.h
	     ./handlers/fm-monohandler.h
	     ./handlers/fm-stereohandler.h
	     ./handlers/am-handler.h
	     ./handlers/raw-handler.h
	     ./handlers/fm-demodulator.h
	     ./various/newconverter.h
	     ./various/ringbuffer.h
	     ./various/pllC.h
	     ./various/Xtan2.h
	     ./various/fir-filters.h	
	     ./various/fft-filters.h
	     ./various/fft.h
	     ./devices/device-handler.h
	)

	set (${objectName}_SRCS
	     ${${objectName}_SRCS}
	     ./main.cpp
	     ./radio-processor.cpp
	     ./handlers/radio-handler.cpp
	     ./handlers/fm-monohandler.cpp
	     ./handlers/fm-stereohandler.cpp
	     ./handlers/am-handler.cpp
	     ./handlers/raw-handler.cpp
	     ./handlers/fm-demodulator.cpp
	     ./various/newconverter.cpp
	     ./various/pllC.cpp
	     ./various/Xtan2.cpp
	     ./various/fir-filters.cpp
	     ./various/fft-filters.cpp
	     ./various/fft.cpp
	     ./devices/device-handler.cpp
	)


	include_directories (
	          ${SDRPLAY_INCLUDES}
	          ${FFTW_INCLUDE_DIRS}
	)

#####################################################################

	add_executable (${objectName} 
	                ${${objectName}_SRCS}
	)

	target_link_libraries (${objectName}
	                       ${FFTW3F_LIBRARIES}
	                       ${extraLibs}
	                       ${CMAKE_DL_LIBS}
	)

	INSTALL (TARGETS ${objectName} DESTINATION ./)

########################################################################
# Create uninstall target
########################################################################

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
