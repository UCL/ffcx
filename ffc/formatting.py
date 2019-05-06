# -*- coding: utf-8 -*-
# Copyright (C) 2009-2018 Anders Logg and Garth N. Wells
#
# This file is part of FFC (https://www.fenicsproject.org)
#
# SPDX-License-Identifier:    LGPL-3.0-or-later
"""Compiler stage 5: Code formatting

This module implements the formatting of UFC code from a given
dictionary of generated C++ code for the body of each UFC function.

It relies on templates for UFC code available as part of the module
ufc_utils.

"""

import logging
import os
import pprint
import textwrap
from collections import namedtuple

from ffc import __version__ as FFC_VERSION
from ffc.codegeneration import __version__ as UFC_VERSION
from ffc.parameters import compilation_relevant_parameters

logger = logging.getLogger(__name__)

FORMAT_TEMPLATE = {
    "ufc comment":
    """\
// This code conforms with the UFC specification version {ufc_version}
// and was automatically generated by FFC version {ffc_version}.
""",
    "dolfin comment":
    """\
// This code conforms with the UFC specification version {ufc_version}
// and was automatically generated by FFC version {ffc_version}.
//
""",
    "header_h":
    """
#pragma once

""",
    "header_c":
    """
""",
}

c_extern_pre = """
#ifdef __cplusplus
extern "C" {
#endif
"""

c_extern_post = """
#ifdef __cplusplus
}
#endif
"""


def format_code(code: namedtuple, wrapper_code, prefix, parameters):
    """Format given code in UFC format. Returns two strings with header and source file contents."""

    logger.debug("Compiler stage 5: Formatting code")

    # Generate code for comment at top of file
    code_h_pre = _generate_comment(parameters) + "\n"
    code_c_pre = _generate_comment(parameters) + "\n"

    # Generate code for header
    code_h_pre += FORMAT_TEMPLATE["header_h"]
    code_c_pre += FORMAT_TEMPLATE["header_c"]

    # Define ufc_scalar before including ufc.h
    scalar_type = _define_scalar(parameters)
    code_h_pre += scalar_type
    code_c_pre += scalar_type

    # Generate includes and add to preamble
    includes_h, includes_c = _generate_includes(parameters)
    code_h_pre += includes_h
    code_c_pre += includes_c

    # Enclose header with 'extern "C"'
    code_h_pre += c_extern_pre
    code_h_post = c_extern_post

    code_h = ""
    code_c = ""

    for parts_code in code:
        code_h += "".join([c[0] for c in parts_code])
        code_c += "".join([c[1] for c in parts_code])

    # Add wrappers
    if wrapper_code:
        code_h += wrapper_code[0]
        code_c += wrapper_code[1]

    # Add headers to body
    code_h = code_h_pre + code_h + code_h_post
    code_c = code_c_pre + code_c

    return code_h, code_c


def write_code(code_h, code_c, prefix, parameters):
    # Write file(s)
    _write_file(code_h, prefix, ".h", parameters)
    if code_c:
        _write_file(code_c, prefix, ".c", parameters)


def _write_file(output, prefix, postfix, parameters):
    """Write generated code to file."""
    filename = os.path.join(parameters["output_dir"], prefix + postfix)
    with open(filename, "w") as hfile:
        hfile.write(output)
    logger.info("Output written to " + filename + ".")


def _generate_comment(parameters):
    """Generate code for comment on top of file."""

    # Drop irrelevant parameters
    parameters = compilation_relevant_parameters(parameters)

    # Generate top level comment
    comment = FORMAT_TEMPLATE["ufc comment"].format(ffc_version=FFC_VERSION, ufc_version=UFC_VERSION)

    # Add parameter information
    comment += "//\n"
    comment += "// This code was generated with the following parameters:\n"
    comment += "//\n"
    comment += textwrap.indent(pprint.pformat(parameters), "//  ")
    comment += "\n"

    return comment


def _generate_includes(parameters):

    default_h_includes = [
        "#include <ufc.h>",
    ]

    default_c_includes = [
        "#include <math.h>",  # This should really be set by the backend
        "#include <stdalign.h>",  # This should really be set by the backend
        "#include <stdbool.h>",  # This should really be set by the backend
        "#include <stdlib.h>",  # This should really be set by the backend
        "#include <string.h>",  # This should really be set by the backend
        "#include <ufc.h>",
    ]

    s_h = set(default_h_includes)
    s_c = set(default_c_includes)

    includes_h = "\n".join(sorted(s_h)) + "\n" if s_h else ""
    includes_c = "\n".join(sorted(s_c)) + "\n" if s_c else ""

    return includes_h, includes_c


def _define_scalar(parameters):
    # Define the ufc_scalar type before including  the ufc header
    # By default use double scalars
    scalar_type = parameters.get("scalar_type")
    if "complex" in scalar_type:
        base_type = scalar_type.replace("complex", "")
        scalar = """
#if defined(__cplusplus)
 #include <complex>
 typedef std::complex<{0}> ufc_scalar_t;
#else
 #include <complex.h>
 typedef {0} _Complex ufc_scalar_t;
#endif

""".format(base_type)
    else:
        scalar = "typedef " + scalar_type + " ufc_scalar_t;" + "\n"

    return scalar
