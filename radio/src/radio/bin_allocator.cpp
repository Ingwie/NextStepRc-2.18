#include <stdlib.h>
#include <string.h>
#include "opentx.h"
#include "bin_allocator.h"


BinAllocator_slots1 slots1;
BinAllocator_slots2 slots2;

#if defined(DEBUG)
int SimulateMallocFailure = 0;    //set this to simulate allocation failure
#endif 

bool bin_free(void * ptr)
{
  //return TRUE if ours
  return slots1.free(ptr) || slots2.free(ptr);
}

void * bin_malloc(size_t size) {
  //try to allocate from our space
  void * res = slots1.malloc(size);
  return res ? res : slots2.malloc(size);
}

void * bin_realloc(void * ptr, size_t size)
{
  if (ptr == 0) {
    //no previous data, try our malloc
    return bin_malloc(size);
  }
  else {
    if (! (slots1.is_member(ptr) || slots2.is_member(ptr)) ) {
      // not our data, leave it to libc realloc
      return 0;
    }

    //we have existing data
    // if it fits in current slot, return it
    // TODO if new size is smaller, try to relocate in smaller slot
    if ( slots1.can_fit(ptr, size) ) {
      // TRACE("OUR realloc %p[%lu] fits in slot1", ptr, size);
      return ptr;
    }
    if ( slots2.can_fit(ptr, size) ) {
      // TRACE("OUR realloc %p[%lu] fits in slot2", ptr, size);
      return ptr;
    }

    //we need a bigger slot
    void * res = bin_malloc(size);
    if (res == 0) {
      // we don't have the space, use libc malloc
      // TRACE("bin_malloc [%lu] FAILURE", size);
      res = malloc(size);
      if (res == 0) {
        TRACE("libc malloc [%lu] FAILURE", size);  
        return 0;
      }
    }
    //copy data
    memcpy(res, ptr, slots1.size(ptr) + slots2.size(ptr));
    bin_free(ptr);
    return res;
  }
}


void *bin_l_alloc (void *ud, void *ptr, size_t osize, size_t nsize)
{
  (void)ud; (void)osize;  /* not used */
  if (nsize == 0) {
    if (ptr) {   // avoid a bunch of NULL pointer free calls
      if (!bin_free(ptr)) {
        // not our range, use libc allocator
        // TRACE("libc free %p", ptr);
        free(ptr);
      }
    }
    return NULL;
  }
  else {
#if defined(DEBUG)
    if (SimulateMallocFailure < 0 ) {
      //delayed failure
      if (++SimulateMallocFailure == 0) {
        SimulateMallocFailure = 1;
      }
    }
    if ( SimulateMallocFailure > 0) {
      // simulate one malloc failure
      TRACE("bin_l_alloc(): simulating malloc failure at %p[%lu]", ptr, nsize);
      return 0;
    }
#endif // #if defined(DEBUG)
    // try our allocator, if it fails use libc allocator
    void * res = bin_realloc(ptr, nsize);
    if (res && ptr) {
      // TRACE("OUR realloc %p[%lu] -> %p[%lu]", ptr, osize, res, nsize); 
    }
    if (res == 0) {
      res = realloc(ptr, nsize);
      // TRACE("libc realloc %p[%lu] -> %p[%lu]", ptr, osize, res, nsize);
      // if (res == 0 ){
      //   TRACE("realloc FAILURE %lu", nsize);
      //   dumpFreeMemory();
      // }
    }
    return res;
  }
}
