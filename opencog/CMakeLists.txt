
# This declaration must happen before subdirectory traversal.
DECLARE_GUILE_CONFIG_TARGET(SCM_V0_CONFIG "opencog sensory-v0-config" "SENSORY_TEST")
DECLARE_GUILE_CONFIG_TARGET(SCM_CONFIG "opencog sensory-config" "SENSORY_TEST")

ADD_SUBDIRECTORY (atoms)
ADD_SUBDIRECTORY (sensory-v0)
ADD_SUBDIRECTORY (sensory)

# Python bindings
IF (HAVE_CYTHON)
   ADD_SUBDIRECTORY (cython-v0)
ENDIF (HAVE_CYTHON)

# -----------------------------
# Testing framework boilerplate.

WRITE_GUILE_CONFIG(${GUILE_BIN_DIR}/opencog/sensory-v0-config.scm SCM_V0_CONFIG TRUE)
WRITE_GUILE_CONFIG(${GUILE_BIN_DIR}/opencog/sensory-v0-config-installable.scm SCM_V0_CONFIG FALSE)
INSTALL(FILES ${GUILE_BIN_DIR}/opencog/sensory-v0-config-installable.scm
DESTINATION ${GUILE_SITE_DIR}/opencog RENAME sensory-v0-config.scm)

# -----------------------------
# Testing framework boilerplate.

WRITE_GUILE_CONFIG(${GUILE_BIN_DIR}/opencog/sensory-config.scm SCM_CONFIG TRUE)
WRITE_GUILE_CONFIG(${GUILE_BIN_DIR}/opencog/sensory-config-installable.scm SCM_CONFIG FALSE)
INSTALL(FILES ${GUILE_BIN_DIR}/opencog/sensory-config-installable.scm
DESTINATION ${GUILE_SITE_DIR}/opencog RENAME sensory-config.scm)

# -----------------------------
