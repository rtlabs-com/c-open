# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))


# -- Project information -----------------------------------------------------

project = 'c-open'
copyright = '2022, RT-Labs AB'
author = 'RT-Labs AB'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    "breathe",
    "recommonmark",
    "sphinx_copybutton",
    "sphinx_jinja",
    "sphinx_rtd_theme",
    "sphinx.ext.autosectionlabel",
    "sphinx.ext.graphviz",
    "sphinxcontrib.kroki",
    "sphinxcontrib.programoutput",
    "sphinxcontrib.spelling",
]

# Spelling
spelling_word_list_filename = "spelling-wordlist.txt"

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']


# Breathe Configuration
breathe_default_project = "co"

# -- Options for HTML output -------------------------------------------------

# TODO Fine-tune the CSS to adjust logo and API documentation appearance
html_theme = "press"
html_last_updated_fmt = "%Y-%m-%d %H:%M"
html_static_path = ["static"]
html_logo = "illustrations/c-open.png"
html_show_sourcelink = False
html_copy_sources = False
html_show_sphinx = False

html_css_files = [
    "../../css/custom_rtd.css",
    "css/fix_table_width.css",
    "css/change_header_size.css",
]
