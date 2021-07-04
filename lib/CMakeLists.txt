file(GLOB_RECURSE SRCFILES "${CMAKE_CURRENT_SOURCE_DIR}/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

set(LIB_FILE_NAME "FileTestSuite_parser")
string(TOUPPER "${LIB_FILE_NAME}" LIB_FILE_NAME_UPPER)

add_library(lib${LIB_FILE_NAME} SHARED "${SRCFILES}")
target_include_directories(lib${LIB_FILE_NAME} PUBLIC "${Include_dir}")

set_target_properties(lib${LIB_FILE_NAME} PROPERTIES
	PREFIX ""
	SOVERSION "${CPACK_PACKAGE_VERSION}"
	CXX_STANDARD 20
)

harden(lib${LIB_FILE_NAME})

cpack_add_component(lib${LIB_FILE_NAME}
	DISPLAY_NAME "library"
	DESCRIPTION "The library implementing fileTestSuite spec"
	REQUIRED
)

set(CMAKE_EXPORT_NAME "fileTestSuite_parser")
set(PKG_CONFIG_NAME "${CMAKE_EXPORT_NAME}")

install(TARGETS "lib${LIB_FILE_NAME}"
	LIBRARY
	COMPONENT "lib${LIB_FILE_NAME}"
)
set("CPACK_DEBIAN_LIB${LIB_FILE_NAME_UPPER}_PACKAGE_NAME" "${CPACK_DEBIAN_PACKAGE_NAME}_parser")
set("CPACK_RPM_LIB${LIB_FILE_NAME_UPPER}_PACKAGE_NAME" "${CPACK_RPM_PACKAGE_NAME}_parser")

list(APPEND CPACK_COMPONENTS_ALL "lib${LIB_FILE_NAME}")  # strangely, not populated automatically correctly


######## Headers and other dev files ########


cpack_add_component(lib${LIB_FILE_NAME}_dev
	DISPLAY_NAME "Development files"
	DESCRIPTION "Headers and other files needed for using lib${LIB_FILE_NAME} in own software"
	DEPENDS "lib${LIB_FILE_NAME}"
)

install(FILES "${HDRFILES_C}"
	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${Include_sub_dir_name}"
	COMPONENT "lib${LIB_FILE_NAME}_dev"
)

set(PKG_CONFIG_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/${PKG_CONFIG_NAME}.pc")
configure_file("${PackagingTemplatesDir}/pkgconfig.pc.in" "${PKG_CONFIG_FILE_NAME}" @ONLY)
install(FILES "${PKG_CONFIG_FILE_NAME}"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
	COMPONENT "lib${LIB_FILE_NAME}_dev"
)

set(CMAKE_CONFIG_FILE_BASENAME "${CMAKE_EXPORT_NAME}Config.cmake")
set(CMAKE_CONFIG_VERSION_FILE_BASENAME "${CMAKE_EXPORT_NAME}ConfigVersion.cmake")
set(CMAKE_CONFIG_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CONFIG_FILE_BASENAME}")
configure_package_config_file("${PackagingTemplatesDir}/CMakeConfig.cmake.in" "${CMAKE_CONFIG_FILE_NAME}"
	INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake"
)


set(CMAKE_CONFIG_VERSION_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CONFIG_VERSION_FILE_BASENAME}")
write_basic_package_version_file(
	"${CMAKE_CONFIG_VERSION_FILE_NAME}"
	VERSION "100500.100500.100500"  # any version of same bitness suits. CMake cannot compare to infinity, so use a large number we expect to be greater than any future version
	COMPATIBILITY AnyNewerVersion
)

install(FILES "${CMAKE_CONFIG_FILE_NAME}" "${CMAKE_CONFIG_VERSION_FILE_NAME}"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_EXPORT_NAME}"
	COMPONENT "lib${LIB_FILE_NAME}_dev"
)

set("CPACK_DEBIAN_LIB${LIB_FILE_NAME_UPPER}_DEV_PACKAGE_NAME" "${CPACK_DEBIAN_LIB${LIB_FILE_NAME_UPPER}_PACKAGE_NAME}-dev")
set("CPACK_DEBIAN_LIB${LIB_FILE_NAME_UPPER}_DEV_PACKAGE_DEPENDS" "${CPACK_DEBIAN_LIB${LIB_FILE_NAME_UPPER}_PACKAGE_NAME}")
set("CPACK_DEBIAN_LIB${LIB_FILE_NAME_UPPER}_DEV_PACKAGE_SUGGESTS" "cmake, pkg-config, pkg-conf")

set("CPACK_RPM_LIB${LIB_FILE_NAME_UPPER}_DEV_PACKAGE_NAME" "${CPACK_RPM_${LIB_FILE_NAME_UPPER}_PACKAGE_NAME}-devel")
set("CPACK_RPM_LIB${LIB_FILE_NAME_UPPER}_DEV_PACKAGE_DEPENDS" "${CPACK_RPM_${LIB_FILE_NAME_UPPER}_PACKAGE_NAME}")
set("CPACK_RPM_LIB${LIB_FILE_NAME_UPPER}_DEV_PACKAGE_SUGGESTS" "${CPACK_DEBIAN_${LIB_FILE_NAME_UPPER}_DEV_PACKAGE_SUGGESTS}")

list(APPEND CPACK_COMPONENTS_ALL "lib${LIB_FILE_NAME}_dev")  # strangely, not populated automatically correctly

pass_through_cpack_vars()
