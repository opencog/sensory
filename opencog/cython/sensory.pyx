#
# This file reads files that are generated by the OPENCOG_ADD_ATOM_TYPES
# macro so they can be imported using:
#
# from sensory import *
#
# This imports all the python wrappers for atom creation.
#
import warnings
# from cython.operator cimport dereference as deref

from opencog.atomspace import types
from opencog.atomspace import regenerate_types
# from opencog.utilities import add_node, add_link

# from opencog.atomspace cimport cValuePtr, cHandle, cAtomSpace
# from opencog.atomspace cimport Atom
# from opencog.atomspace cimport create_python_value_from_c_value

# The list of Atom Types that python knows about has to be rebuilt,
# before much else can be done.
regenerate_types()

include "opencog/atoms/sensory-types/sensory_types.pyx"
