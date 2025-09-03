ADD_SUBDIRECTORY (types)

add_guile_extension(SCM_V0_CONFIG sensory-v0 "opencog-ext-path-sensory-v0")

add_guile_module(FILES
   sensory-v0.scm
   ${CMAKE_CURRENT_BINARY_DIR}/types/sensory_v0_types.scm
   MODULE_DESTINATION "${GUILE_SITE_DIR}/opencog/sensory-v0"
   DEPENDS sensory_v0_atom_types
)
