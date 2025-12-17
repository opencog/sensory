CMAKE_MINIMUM_REQUIRED(VERSION 3.12)
SET(CMAKE_INSTALL_MESSAGE LAZY)

PROJECT(sensory VERSION 0.5.1)

# Something about path broken normalization
IF(CMAKE_VERSION VERSION_GREATER 3.31)
   CMAKE_POLICY(SET CMP0177 NEW)
ENDIF(CMAKE_VERSION VERSION_GREATER 3.31)

# Setting this will use -O3 -g -flto and the -flto flag is slow.
SET(CMAKE_BUILD_TYPE Release)
# SET(CMAKE_BUILD_TYPE Debug)

# ----------------------------------------------------------
# Cogutil. Required prerequisite.
FIND_PACKAGE(CogUtil 2.2.1 CONFIG REQUIRED)
IF (NOT COGUTIL_FOUND)
	MESSAGE(FATAL_ERROR "CogUtil missing: it is needed!")
ENDIF()

include(OpenCogGccOptions)
include(OpenCogLibOptions)
include(OpenCogInstallOptions)
include(Summary)

# AtomSpace
FIND_PACKAGE(AtomSpace 5.2.0 CONFIG REQUIRED)
IF(NOT ATOMSPACE_FOUND)
	MESSAGE(FATAL_ERROR "AtomSpace missing: it is needed!")
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
