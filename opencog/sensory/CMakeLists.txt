ADD_SUBDIRECTORY (types)

add_guile_extension(SCM_CONFIG sensory "opencog-ext-path-sensory")

add_guile_module(FILES
   sensory.scm
   MODULE_DESTINATION "${GUILE_SITE_DIR}/opencog/sensory"
   DEPENDS sensory_atom_types
)
