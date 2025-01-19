
# This declaration must happen before subdirectory traversal.
DECLARE_GUILE_CONFIG_TARGET(SCM_CONFIG "opencog sensory-config" "SENSORY_TEST")

ADD_SUBDIRECTORY (atoms)
ADD_SUBDIRECTORY (sensory)

# Python bindings
IF (HAVE_CYTHON)
   ADD_SUBDIRECTORY (cython)
ENDIF (HAVE_CYTHON)

# -----------------------------
# Testing framework boilerplate.

WRITE_GUILE_CONFIG(${GUILE_BIN_DIR}/opencog/sensory-config.scm SCM_CONFIG TRUE)
WRITE_GUILE_CONFIG(${GUILE_BIN_DIR}/opencog/sensory-config-installable.scm SCM_CONFIG FALSE)
INSTALL(FILES ${GUILE_BIN_DIR}/opencog/sensory-config-installable.scm
DESTINATION ${GUILE_SITE_DIR}/opencog RENAME sensory-config.scm)
