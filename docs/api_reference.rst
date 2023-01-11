.. _api-reference:

API documentation
==================

Here is a brief overview given for some of the most important functions,
structs etc in the public API for the C-Open CANopen stack.

For detailed documentation, read the ``include/co_api.h`` header file.

Functions
----------

.. doxygenfunction:: co_error_set
.. doxygenfunction:: co_sync_timer
.. doxygenfunction:: co_error_get

Callbacks
----------

.. doxygentypedef:: co_access_fn

Structs
--------

.. doxygenstruct:: co_obj
   :members:
   :undoc-members:

.. doxygenstruct:: co_entry
   :members:
   :undoc-members:

.. doxygenstruct:: co_default
   :members:
   :undoc-members:

.. doxygenstruct:: co_cfg
   :members:
   :undoc-members:

Enums
------

.. doxygenenum:: co_sdo_abort
.. doxygenenum:: co_state
.. doxygenenum:: co_nmt_cmd
.. doxygenenum:: co_otype
.. doxygenenum:: co_dtype
.. doxygenenum:: od_event
.. doxygenenum:: co_store
.. doxygenenum:: co_mode
