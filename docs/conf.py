# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

import pathlib
import re
import sys

import os


pathobj_docs_dir = pathlib.Path(__file__).parent.absolute()
pathobj_rootdir = pathobj_docs_dir.parent.absolute()

# -- Project information -----------------------------------------------------

try:
    cmakelists_contents = pathobj_rootdir.joinpath("CMakeLists.txt").read_text()
    versiontext_match = re.search(r"CANOPEN VERSION ([\d.]*)", cmakelists_contents)
    version = versiontext_match.group(1)
except:
    version = "unknown version"

project = 'c-open'
copyright = '2023, RT-Labs AB'
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
    "sphinx.ext.autosectionlabel",
    "sphinx.ext.graphviz",
    "sphinxcontrib.kroki",
    "sphinxcontrib.programoutput",
    "sphinxcontrib.spelling",
]


# Spelling
spelling_word_list_filename = "spelling-wordlist.txt"

# Add any paths that contain templates here, relative to this directory.
templates_path = ["../../_templates"]

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']


# Breathe Configuration
breathe_default_project = "co"

# -- Options for HTML output -------------------------------------------------

html_context = {
   "default_mode": "light"
}

html_theme = "sphinx_book_theme"
html_theme_options = {
    "show_nav_level": 3,
    "home_page_in_toc": True,
    "use_repository_button": True,
    "use_fullscreen_button": False,
    "navbar_end": ["navbar-icon-links"],
    "use_download_button": False,
    "repository_url": "https://github.com/rtlabs-com/c-open",
}

html_last_updated_fmt = "%Y-%m-%d %H:%M"
html_static_path = ["static"]
html_logo = "static/i/c-open.svg"
html_show_sourcelink = False

if os.getenv("USE_EXTERNAL_CSS") is not None:
    html_css_files = [
        "https://rt-labs.com/content/themes/rtlabs2020/assets/css/style.css",
        "https://rt-labs.com/content/themes/rtlabs2020/assets/css/rt_custom_sphinx.css",
    ]
