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

.. doxygenstruct:: co_obj_t
   :members:
   :undoc-members:

.. doxygenstruct:: co_entry_t
   :members:
   :undoc-members:

.. doxygenstruct:: co_default_t
   :members:
   :undoc-members:

.. doxygenstruct:: co_cfg_t
   :members:
   :undoc-members:

Enums
------

.. doxygenenum:: co_sdo_abort_t
.. doxygenenum:: co_state_t
.. doxygenenum:: co_nmt_cmd_t
.. doxygenenum:: co_otype_t
.. doxygenenum:: co_dtype_t
.. doxygenenum:: od_event_t
.. doxygenenum:: co_store_t
.. doxygenenum:: co_mode_t
