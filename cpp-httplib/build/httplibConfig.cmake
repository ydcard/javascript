# Generates a macro to auto-configure everything

####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was httplibConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

####################################################################################

# Setting these here so they're accessible after install.
# Might be useful for some users to check which settings were used.
set(HTTPLIB_IS_USING_OPENSSL TRUE)
set(HTTPLIB_IS_USING_ZLIB TRUE)
set(HTTPLIB_IS_COMPILED OFF)

include(CMakeFindDependencyMacro)

# We add find_dependency calls here to not make the end-user have to call them.
find_dependency(Threads REQUIRED)
if(TRUE)
	# OpenSSL COMPONENTS were added in Cmake v3.11
	if(CMAKE_VERSION VERSION_LESS "3.11")
		find_dependency(OpenSSL 1.1.1 REQUIRED)
	else()
		# Once the COMPONENTS were added, they were made optional when not specified.
		# Since we use both, we need to search for both.
		find_dependency(OpenSSL 1.1.1 COMPONENTS Crypto SSL REQUIRED)
	endif()
endif()
if(TRUE)
	find_dependency(ZLIB REQUIRED)
endif()

# Lets the end-user find the header path if needed
# This is helpful if you're using Cmake's pre-compiled header feature
set_and_check(HTTPLIB_HEADER_PATH "${PACKAGE_PREFIX_DIR}/include/httplib.h")

# Brings in the target library
include("${CMAKE_CURRENT_LIST_DIR}/httplibTargets.cmake")
