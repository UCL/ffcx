# -*- coding: utf-8 -*-
"""Code generation format strings for UFC (Unified Form-assembly Code)

Five format strings are defined for each of the following UFC classes:

    cell_integral
    exterior_facet_integral
    interior_facet_integral
    custom_integral

    finite_element
    dofmap
    coordinate_mapping
    form

The strings are named:

    '<classname>_header'
    '<classname>_implementation'
    '<classname>_combined'

The header and implementation contain the definition and declaration of
the class respectively, and are meant to be placed in .h and .cpp files,
while the combined version is for an implementation within a single .h
header. Some additional factory functions exported as extern "C" to
allow construction of compiled objects through ctypes without dealing
with C++ ABI and name mangling issues.

Each string has at least the following format variables: 'classname',
'members', 'constructor', 'destructor', plus one for each interface
function with name equal to the function name.

For more information about UFC and the FEniCS Project, visit

    http://www.fenicsproject.org
    https://bitbucket.org/fenics-project/ffc

"""

__author__ = "Martin Sandve Alnæs, Anders Logg, Kent-Andre Mardal, Ola Skavhaug, and Hans Petter Langtangen"
__date__ = "2017-05-09"
__version__ = "2018.1.0.dev0"
__license__ = "This code is released into the public domain"

import os
from hashlib import sha1

from ffc.backends.ufc.finite_element import *
from ffc.backends.ufc.dofmap import *
from ffc.backends.ufc.coordinate_mapping import *
from ffc.backends.ufc.integrals import *
from ffc.backends.ufc.form import *


# Get abspath on import, it can in some cases be
# a relative path w.r.t. curdir on startup
_include_path = os.path.dirname(os.path.abspath(__file__))


def get_include_path():
    "Return location of UFC header files"
    return _include_path

# NOTE: seems this had no effect since dijitso doesn't use -fvisibility
#
# # Platform specific snippets for controlling visilibity of exported
# # symbols in generated shared libraries
# visibility_snippet = """
# // Based on https://gcc.gnu.org/wiki/Visibility
# #if defined _WIN32 || defined __CYGWIN__
#     #ifdef __GNUC__
#         #define DLL_EXPORT __attribute__ ((dllexport))
#     #else
#         #define DLL_EXPORT __declspec(dllexport)
#     #endif
# #else
#     #define DLL_EXPORT __attribute__ ((visibility ("default")))
# #endif
# """


# Generic factory function signature
factory_decl = """
extern "C" %(basename)s * create_%(publicname)s();
"""


# Generic factory function implementation. Set basename to the base class,
# and note that publicname and privatename does not need to match, allowing
# multiple factory functions to return the same object.
factory_impl = """
extern "C" %(basename)s * create_%(publicname)s()
{
  return new %(privatename)s();
}
"""
# factory_impl = """
# extern "C" DLL_EXPORT %(basename)s * create_%(publicname)s()
# {
#   return new %(privatename)s();
# }
# """


def all_ufc_classnames():
    "Build list of all classnames."
    jitable_classnames = []
    classnames = jitable_classnames
    return classnames


def _build_templates():
    "Build collection of all templates to store in the templates dict."
    templates = {}
    classnames = all_ufc_classnames()

    for classname in classnames:
        # Expect all classes to have header, implementation, and combined versions
        header = globals()[classname + "_header"]
        implementation = globals()[classname + "_implementation"]
        combined = globals()[classname + "_combined"]

        # Construct jit header with class and factory function signature
        _fac_decl = factory_decl % {
            "basename": "ufc::" + classname,
            "publicname": "%(classname)s",
            "privatename": "%(classname)s",
        }
        header += _fac_decl
        combined += _fac_decl

        # Construct jit implementation template with class declaration,
        # factory function implementation, and class definition
        _fac_impl = factory_impl % {
            "basename": "ufc::" + classname,
            "publicname": "%(classname)s",
            "privatename": "%(classname)s",
        }
        implementation += _fac_impl
        combined += _fac_impl

        # Store all in templates dict
        templates[classname + "_header"] = header
        templates[classname + "_implementation"] = implementation
        templates[classname + "_combined"] = combined

    return templates


def _compute_ufc_templates_signature(templates):
    # Compute signature of jit templates
    h = sha1()
    for k in sorted(templates):
        h.update(k.encode("utf-8"))
        h.update(templates[k].encode("utf-8"))
    return h.hexdigest()


def _compute_ufc_signature():
    # Compute signature of ufc header files
    h = sha1()
    for fn in ("ufc.h", "ufc_geometry.h"):
        with open(os.path.join(get_include_path(), fn)) as f:
            h.update(f.read().encode("utf-8"))
    return h.hexdigest()


# Build these on import
templates = _build_templates()
_ufc_signature = _compute_ufc_signature()
_ufc_templates_signature = _compute_ufc_templates_signature(templates)


def get_ufc_signature():
    """Return SHA-1 hash of the contents of ufc.h and ufc_geometry.h.

    In this implementation, the value is computed on import.
    """
    return _ufc_signature


def get_ufc_templates_signature():
    """Return SHA-1 hash of the ufc code templates.

    In this implementation, the value is computed on import.
    """
    return _ufc_templates_signature


def get_ufc_cxx_flags():
    """Return C++ flags for compiling UFC C++11 code.

    Return type is a list of strings.

    Used internally in some tests.
    """
    return ["-std=c++14"]


# ufc_signature() already introduced to FFC standard in 1.7.0dev,
# called by the dolfin cmake build system to compare against
# future imported ffc versions for compatibility.
ufc_signature = get_ufc_signature
