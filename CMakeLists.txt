CMAKE_MINIMUM_REQUIRED(VERSION 3.12)
SET(CMAKE_INSTALL_MESSAGE LAZY)

PROJECT(sensory)

# Setting this will use -O3 -g -flto and the -flto flag is slow.
SET(CMAKE_BUILD_TYPE Release)
# SET(CMAKE_BUILD_TYPE Debug)

# ----------------------------------------------------------
# Cogutil. Required prerequisite.
FIND_PACKAGE(CogUtil 2.0.3 CONFIG REQUIRED)
IF(COGUTIL_FOUND)
	SET(HAVE_COGUTIL 1)

	# Add the 'cmake' directory from cogutil to search path
	list(APPEND CMAKE_MODULE_PATH  ${COGUTIL_DATA_DIR}/cmake)

	# Needed to set OC_CMAKE_DIR
	if (NOT DEFINED ATOMSPACE_DATA_DIR)
		set (ATOMSPACE_DATA_DIR "${COGUTIL_DATA_DIR}")
	endif (NOT DEFINED ATOMSPACE_DATA_DIR)

	include(OpenCogGccOptions)
	include(OpenCogLibOptions)
	include(OpenCogInstallOptions)
	include(Summary)

ENDIF()

# AtomSpace
FIND_PACKAGE(AtomSpace 5.0.4 CONFIG REQUIRED)
IF(ATOMSPACE_FOUND)
	SET(HAVE_ATOMSPACE 1)
ENDIF()

# ----------------------------------------------------------
# Guile Python and Cython

include(OpenCogFindGuile)
include(OpenCogFindPython)

# Load cmake functions defined in cogutil repo that depend on Guile.
INCLUDE(OpenCogMacros)
INCLUDE(OpenCogGuile)
INCLUDE(OpenCogCython)

# ----------------------------------------------------------
ADD_SUBDIRECTORY (opencog)
ADD_SUBDIRECTORY (cmake)

# Unit tests
ADD_CUSTOM_TARGET(tests)
ADD_SUBDIRECTORY (tests EXCLUDE_FROM_ALL)
ADD_CUSTOM_TARGET(test
	DEPENDS tests
	WORKING_DIRECTORY tests
	COMMAND ${CMAKE_CTEST_COMMAND} $(ARGS)
	COMMENT "Running tests..."
)
ADD_CUSTOM_TARGET(check
	DEPENDS tests
	WORKING_DIRECTORY tests
	COMMAND ${CMAKE_CTEST_COMMAND} $(ARGS)
	COMMENT "Running tests..."
)

# ===================================================================
# Show a summary of what we found, what we will do.

SUMMARY_ADD("Python bindings" "Python (cython) bindings" HAVE_CYTHON)
SUMMARY_SHOW()
