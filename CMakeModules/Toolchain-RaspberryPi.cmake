SET( CMAKE_SYSTEM_NAME Linux )
SET( CMAKE_SYSTEM_VERSION 1 )

SET( CMAKE_C_COMPILER arm-linux-gnueabihf-gcc )
SET( CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++ )
SET( CMAKE_FIND_ROOT_PATH /home/drich/rpi/tools/arm-bcm2708/arm-bcm2708hardfp-linux-gnueabi/arm-bcm2708hardfp-linux-gnueabi/sysroot/usr )

# Fix..
SET( FREETYPE_INCLUDE_DIRS "${CMAKE_FIND_ROOT_PATH}/include/freetype2" )

SET( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
SET( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
SET( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
