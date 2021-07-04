cmake_minimum_required(VERSION 3.7.2)

set(CMAKE_USE_RELATIVE_PATHS TRUE)
project("libFileTestSuite")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake" "${CMAKE_SOURCE_DIR}/cmake/Hardening")

include(CPackComponent)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
include(Hardening)

if(${CMAKE_VERSION} VERSION_GREATER "3.12")
	set(CMAKE_CXX_STANDARD 20)
else()
	set(CMAKE_CXX_STANDARD 17)
endif()

if(${CMAKE_VERSION} VERSION_GREATER "3.22")
	set(CMAKE_C_STANDARD 23)
else()
	set(CMAKE_C_STANDARD 11)
endif()

macro(pass_through_cpack_vars)
	get_cmake_property(cpackVarsToPassthrough VARIABLES)
	foreach(varName ${cpackVarsToPassthrough})
		if(varName MATCHES "^CPACK_")
			set("${varName}" "${${varName}}" PARENT_SCOPE)
		endif()
	endforeach()
endmacro()


set(Include_sub_dir_name "fileTestSuite")
set(Include_dir "${CMAKE_CURRENT_SOURCE_DIR}/include")
file(GLOB_RECURSE HDRFILES_C "${Include_dir}/*.h")
file(GLOB_RECURSE HDRFILES_CPP "${Include_dir}/*.hpp")

set(LibSource_dir "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(BinSource_dir "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(testGen_dir "${CMAKE_CURRENT_SOURCE_DIR}/testGen")
set(PackagingTemplatesDir "${CMAKE_CURRENT_SOURCE_DIR}/packaging")

set(CPACK_PACKAGE_NAME "libFileTestSuite")
set(CPACK_PACKAGE_VENDOR "KOLANICH")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_DESCRIPTION "C companion library for fileTestSuite")
set(CPACK_DEBIAN_PACKAGE_NAME "${CPACK_PACKAGE_NAME}")
set(CPACK_RPM_PACKAGE_NAME "${CPACK_PACKAGE_NAME}")

set(mio_dir "${CMAKE_CURRENT_SOURCE_DIR}/third-party/mio")
add_subdirectory("${mio_dir}")

add_subdirectory("${LibSource_dir}")
add_subdirectory("${BinSource_dir}")
add_subdirectory("${testGen_dir}")


set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/fileTestSuite/fileTestSuite.c")
set(CPACK_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR}")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_MAINTAINER}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/UNLICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.md")
set(CPACK_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR}")
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_NSIS_COMPONENT_INSTALL ON)
#set(CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS ON)
set(CPACK_RPM_ENABLE_COMPONENT_DEPENDS ON)
set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")

include(CPack)
