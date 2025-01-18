
export(EXPORT SensoryTargets
	FILE "${CMAKE_CURRENT_BINARY_DIR}/Sensory/SensoryTargets.cmake"
)

set(ConfigPackageLocation lib/cmake/Sensory)

install(EXPORT SensoryTargets
	FILE SensoryTargets.cmake
	DESTINATION ${ConfigPackageLocation}
)

include(CMakePackageConfigHelpers)

configure_package_config_file(SensoryConfig.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/SensoryConfig.cmake
	INSTALL_DESTINATION lib/Sensory/cmake
	PATH_VARS CMAKE_INSTALL_PREFIX
)

write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/SensoryConfigVersion.cmake"
	VERSION ${SEMANTIC_VERSION}
	COMPATIBILITY SameMajorVersion
)

INSTALL(FILES
	${CMAKE_CURRENT_BINARY_DIR}/SensoryConfigVersion.cmake
	${CMAKE_CURRENT_BINARY_DIR}/SensoryConfig.cmake
	DESTINATION ${ConfigPackageLocation}
)

# -----------------------------------------------------------
