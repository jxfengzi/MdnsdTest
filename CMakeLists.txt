CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(MdnsdTest C)

#---------------------------------------------------------------------------------------
#
# Cross Compile
#
#---------------------------------------------------------------------------------------
IF(WIN32)
ELSE(WIN32)

    IF(APPLE)
        # Apple LLVM Compiler
        # SET(CMAKE_CXX_COMPILER /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++)

        # LLVM gcc Compiler
        SET(CMAKE_CXX_COMPILER /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++)
        SET(CMAKE_CXX_FLAGS -m64)

        #SET(CMAKE_CXX_COMPILER g++)
        #SET(CMAKE_CXX_FLAGS -m64)
    ELSE(APPLE)

        #SET(CMAKE_C_COMPILE /scratch/nathan/arm-lite/install/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi-gcc)
        #SET(CMAKE_CXX_COMPILE /scratch/nathan/arm-lite/install/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi-g++)
        #SET(CMAKE_RANLIB /scratch/nathan/arm-lite/install/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi-ranlib)
        #SET(CMAKE_AR /scratch/nathan/arm-lite/install/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi-ar)

        SET(CMAKE_C_COMPILE /opt/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-gcc)
        SET(CMAKE_CXX_COMPILE /opt/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-g++)
        SET(CMAKE_RANLIB /opt/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-ranlib)
        SET(CMAKE_AR /opt/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-ar)
    ENDIF(APPLE)

ENDIF(WIN32)

#---------------------------------------------------------------------------------------
#
# cxx flags
#
#---------------------------------------------------------------------------------------

#---------------------------------------------------------------------------------------
# WIN32
#---------------------------------------------------------------------------------------
IF(WIN32)
    #ADD_DEFINITIONS(-D__USE_LIB_AO__)

	ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
	
	# unicode for windows
    ADD_DEFINITIONS(-DUNICODE -D_UNICODE)
   
    # bonjour for windows
    ADD_DEFINITIONS(-D__APPLE_BONJOUR__)
ELSE(WIN32)
    #---------------------------------------------------------------------------------------
    # APPLE MACOSX
    #---------------------------------------------------------------------------------------
    IF(APPLE)
        MESSAGE(STATUS "build airjoy for osx")
	    ADD_DEFINITIONS(-D__MAC_OSX__)
	    ADD_DEFINITIONS(-fPIC)
    #---------------------------------------------------------------------------------------
    # Linux
    #---------------------------------------------------------------------------------------
    ELSE(APPLE)
        #ADD_DEFINITIONS(-D__USE_LIB_AO__)
        ADD_DEFINITIONS(-D__APPLE_BONJOUR__)
	    ADD_DEFINITIONS(-fPIC)
    ENDIF(APPLE)
ENDIF(WIN32)

#---------------------------------------------------------------------------------------
#
# include dir
#
#---------------------------------------------------------------------------------------
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/mdnsresponder/mDNSShared)

#--------------------------------------------------------------------------------------
#
# 3rdpart library dir
#
#--------------------------------------------------------------------------------------
IF (WIN32)
  SET(LIB_DIR ${PROJECT_SOURCE_DIR}/src/3rdpartLib/Win32)
ELSE (WIN32)
  IF(APPLE)
    SET(LIB_DIR /usr/lib/system/)
  ELSE(APPLE)
    #SET(LIB_DIR ${PROJECT_SOURCE_DIR}/src/3rdpartLib/Linux64)
    SET(LIB_DIR ${PROJECT_SOURCE_DIR}/src/3rdpartLib/Linux.arm)
  ENDIF(APPLE)
ENDIF (WIN32)

LINK_DIRECTORIES(${LIB_DIR})
MESSAGE(STATUS "3rdpart Library Dir : " ${LIB_DIR})

#--------------------------------------------------------------------------------------
#
# lib
#
#--------------------------------------------------------------------------------------
IF (WIN32)
  SET(OS_LIB Iphlpapi.lib winmm.lib)
  SET(DNSSD_LIB dnssd.lib) 
ELSE (WIN32)
  SET(OS_LIB pthread dl)

  IF(APPLE)
    LINK_DIRECTORIES("/opt/local/lib")
  ELSE(APPLE)
    SET(DNSSD_LIB dns_sd)
  ENDIF(APPLE)
ENDIF (WIN32)

MESSAGE(STATUS "os_lib : ${OS_LIB}")

#-------------------------------------------------------------------------------------
# set output dir
#-------------------------------------------------------------------------------------
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/output)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/output)
MESSAGE(STATUS "executable output:  ${EXECUTABLE_OUTPUT_PATH}")
MESSAGE(STATUS "library output: ${LIBRARY_OUTPUT_PATH}")

#-------------------------------------------------------------------------------------
#
# project
#
#-------------------------------------------------------------------------------------
ADD_SUBDIRECTORY(src)
