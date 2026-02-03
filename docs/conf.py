# -- Path setup --------------------------------------------------------------
import breathe


# -- Project information -----------------------------------------------------

project = 'scc'
copyright = '2025, Vilhelm Engström'
author = 'Vilhelm Engström'

release = '0.1'
version = '0.0.1'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'sphinx.ext.duration',
    'sphinx.ext.doctest',
    'sphinx.ext.autodoc',
    'sphinx.ext.autosummary',
    'sphinx.ext.intersphinx',
    'breathe',
]

intersphinx_mapping = {
    'python': ('https://docs.python.org/3/', None),
    'sphinx': ('https://www.sphinx-doc.org/en/master/', None),
}
intersphinx_disabled_domains = ['std']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'

html_theme_options = {
    'collapse_navigation': False,
    'navigation_depth': 5,
}

# -- Options for EPUB output
epub_show_urls = 'footnote'

# -- Options for breathe -------------------------------------------------

breathe_default_project = 'scc'
breathe_projects = { breathe_default_project: '../build/docs/doxygen/xml' }
