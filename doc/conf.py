# Copyright (c) 2026 Vilhelm Engström
#
# SPDX-License-Identifier: Apache-2.0

from typing import List

import sphinx
import sphinx.cmd.build

# -- Command line arguments --------------------------------------------------
parser = sphinx.cmd.build.get_parser()
args = parser.parse_args()

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = "SCC"
copyright = "2026, Vilhelm Engström"
author = "Vilhelm Engström"

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = ["sphinxcontrib.tikz"]

templates_path = ["_templates"]
exclude_patterns: List[str] = []

# -- Tikz --------------------------------------------------------------------

tikz_proc_suite = "GhostScript"
tikz_transparent = False
tikz_resolution = 1024

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = "sphinx_rtd_theme"
html_static_path = ["_static"]
html_logo = "_static/images/logo.svg"
html_theme_options = {"logo_only": True, "display_version": False}
html_context = {
    "reference_links": {
        "API": "/doxygen",
    },
}
