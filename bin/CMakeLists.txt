file(GLOB_RECURSE SRCFILES "${CMAKE_CURRENT_SOURCE_DIR}/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(ftsBin2Json "${SRCFILES}")
target_include_directories(ftsBin2Json PRIVATE "${Include_dir}")
target_link_libraries(ftsBin2Json PRIVATE libFileTestSuite_parser)
harden(ftsBin2Json)

cpack_add_component(ftsBin2Json
	DISPLAY_NAME "binary meta to JSON convertor"
	DESCRIPTION "A simple app converting binary metadata into JSON one."
	DEPENDS "libFileTestSuite_parser" "libFileTestSuite_parser_dev"
)

install(TARGETS "ftsBin2Json"
	RUNTIME
	DESTINATION "${CMAKE_INSTALL_BINDIR}"
	COMPONENT "ftsBin2Json"
)

set("CPACK_DEBIAN_FTSBIN2JSON_PACKAGE_NAME" "ftsBin2Json")
set("CPACK_RPM_FTSBIN2JSON_PACKAGE_NAME" "ftsBin2Json")


set("CPACK_DEBIAN_FTSBIN2JSON_PACKAGE_DEPENDS" "${CPACK_DEBIAN_LIBFILETESTSUITE_PARSER_PACKAGE_NAME}")
set("CPACK_RPM_FTSBIN2JSON_PACKAGE_DEPENDS" "${CPACK_RPM_LIBFILETESTSUITE_PARSER_PACKAGE_NAME}")


list(APPEND CPACK_COMPONENTS_ALL "ftsBin2Json")  # strangely, not populated automatically correctly

pass_through_cpack_vars()
