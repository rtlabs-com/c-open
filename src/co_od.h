/*********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * www.rt-labs.com
 * Copyright 2017 rt-labs AB, Sweden.
 *
 * This software is dual-licensed under GPLv3 and a commercial
 * license. See the file LICENSE.md distributed with this software for
 * full license information.
 ********************************************************************/

/**
 * @file
 * @brief Object dictionary interface
 */

#ifndef CO_OD_H
#define CO_OD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "co_api.h"
#include "co_main.h"
#include "co_sdo.h"

/**
 * Traverse and call function on all subindexes in an object.
 *
 * This function traverses all subindexes until \a fn returns a value
 * other than 0.
 *
 * This function can e.g. be used to search for an entry.
 *
 * @param net           network handle
 * @param obj           object descriptor
 * @param fn            function to call
 * @param arg           opaque argument for function
 * @param max_subindex  max subindex for traversal
 *
 * @return entry descriptor for subindex where \a fn != 0, or NULL
 */
static inline const co_entry_t * co_obj_traverse (
   co_net_t * net,
   const co_obj_t * obj,
   int (*fn) (
      co_net_t * net,
      const co_obj_t * obj,
      const co_entry_t * entry,
      uintptr_t arg),
   uintptr_t arg,
   uint8_t max_subindex)
{
   const co_entry_t * entry = obj->entries;
   uint8_t subindex;

   do
   {
      subindex = entry->subindex;
      if (fn (net, obj, entry, arg) != 0)
         return entry;
      entry++;
   } while (subindex < max_subindex);

   return NULL;
}

/**
 * Traverse and call function with state on all subindexes in an
 * object.
 *
 * This function traverses all subindexes and calls \a fn supplying
 * the current state.
 *
 * This function can e.g. be used to compute a property of the object,
 * such as the total bit-length of all subindexes.
 *
 * @param net           network handle
 * @param obj           object descriptor
 * @param fn            function to call
 * @param arg           opaque argument for function
 * @param state         initial state
 * @param max_subindex  max subindex for traversal
 *
 * @return final state
 */
static inline int co_obj_reduce (
   co_net_t * net,
   const co_obj_t * obj,
   int (*fn) (co_net_t * net, const co_entry_t * entry, uintptr_t arg, int state),
   uintptr_t arg,
   int state,
   uint8_t max_subindex)
{
   const co_entry_t * entry = obj->entries;
   uint8_t subindex;

   do
   {
      subindex = entry->subindex;
      state    = fn (net, entry, arg, state);
      entry++;
   } while (subindex < max_subindex);

   return state;
}

/**
 * Set dictionary default values
 *
 * This function sets the default value for dictionary entries that
 * have default values. Only indices that are within the given minimum
 * and maximum values are considered.
 *
 * @param net           network handle
 * @param min           minimum index
 * @param max           maximum index
 */
void co_od_set_defaults (co_net_t * net, uint16_t min, uint16_t max);

/**
 * Zero dictionary values
 *
 * This function sets dictionary entries to zero. Only indices that
 * are within the given minimum and maximum values are considered.
 *
 * @param net           network handle
 * @param min           minimum index
 * @param max           maximum index
 */
void co_od_zero (co_net_t * net, uint16_t min, uint16_t max);

/**
 * Load dictionary from store
 *
 * This function loads dictionary values from a store.
 *
 * @param net           network handle
 * @param store         store identifier
 *
 * @return sdo abort code
 */
uint32_t co_od_load (co_net_t * net, co_store_t store);

/**
 * Reset dictionary
 *
 * This function resets dictionary values related to the given
 * store. Only indices that are within the minimum and maximum values
 * are considered. The following operations will be performed:
 *
 *   -# Zero values
 *   -# Set default values
 *   -# Load stored values
 *
 * @param net           network handle
 * @param store         store identifier
 * @param min           minimum index
 * @param max           maximum index
 */
void co_od_reset (co_net_t * net, co_store_t store, uint16_t min, uint16_t max);

/**
 * Save dictionary in store
 *
 * This function saves dictionary values in a store. Only indices that
 * are within the minimum and maximum values are considered.
 *
 * @param net           network handle
 * @param store         store identifier
 * @param min           minimum index
 * @param max           maximum index
 *
 * @return sdo abort code
 */
uint32_t co_od_store (co_net_t * net, co_store_t store, uint16_t min, uint16_t max);

/**
 * Find object in dictionary
 *
 * This function finds the object with the given index.
 *
 * @param net           network handle
 * @param index         index to find
 *
 * @return object descriptor, or NULL if not found
 */
const co_obj_t * co_obj_find (co_net_t * net, uint16_t index);

/**
 * Find entry in object
 *
 * This function finds the entry descriptor with the given subindex.
 *
 * @param net           network handle
 * @param obj           object descriptor
 * @param subindex      subindex to find
 *
 * @return entry descriptor, or NULL if not found
 */
const co_entry_t * co_entry_find (
   co_net_t * net,
   const co_obj_t * obj,
   uint8_t subindex);

/**
 * Get subindex pointer
 *
 * This function returns a pointer to the subindex value. Note that
 * accessing the value in this manner is not thread-safe.
 *
 * @param net           network handle
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param ptr           result on success
 *
 * @return 0 or CO_SDO_ABORT_GENERAL if object has no storage
 */
uint32_t co_od_get_ptr (
   co_net_t * net,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint8_t ** ptr);

/**
 * Get subindex value
 *
 * This function returns the subindex value. The value is read
 * atomically. This function is thread-safe.
 *
 * @param net           network handle
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value on success
 *
 * @return 0 or sdo abort code
 */
uint32_t co_od_get_value (
   co_net_t * net,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint64_t * value);

/**
 * Set subindex value
 *
 * This function sets the subindex value. The value is written
 * atomically. This function is thread-safe.
 *
 * @param net           network handle
 * @param obj           object descriptor
 * @param entry         entry descriptor
 * @param subindex      subindex
 * @param value         value to set
 *
 * @return 0 or sdo abort code
 */
uint32_t co_od_set_value (
   co_net_t * net,
   const co_obj_t * obj,
   const co_entry_t * entry,
   uint8_t subindex,
   uint64_t value);

#ifdef __cplusplus
}
#endif

#endif /* CO_OD_H */
