
# This declaration must happen before subdirectory traversal.
DECLARE_GUILE_CONFIG_TARGET(SCM_CONFIG "opencog sensory-v0-config" "SENSORY_TEST")

ADD_SUBDIRECTORY (atoms-v0)
ADD_SUBDIRECTORY (sensory-v0)

# Python bindings
IF (HAVE_CYTHON)
   ADD_SUBDIRECTORY (cython-v0)
ENDIF (HAVE_CYTHON)

# -----------------------------
# Testing framework boilerplate.

WRITE_GUILE_CONFIG(${GUILE_BIN_DIR}/opencog/sensory-v0-config.scm SCM_CONFIG TRUE)
WRITE_GUILE_CONFIG(${GUILE_BIN_DIR}/opencog/sensory-v0-config-installable.scm SCM_CONFIG FALSE)
INSTALL(FILES ${GUILE_BIN_DIR}/opencog/sensory-v0-config-installable.scm
DESTINATION ${GUILE_SITE_DIR}/opencog RENAME sensory-v0-config.scm)
