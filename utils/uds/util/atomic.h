/*
 * Copyright (c) 2018 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA. 
 *
 * $Id: //eng/uds-releases/flanders-rhel7.5/src/uds/util/atomic.h#1 $
 */

#ifndef ATOMIC_H
#define ATOMIC_H

#include "compiler.h"
#include "typeDefs.h"

#define ATOMIC_INITIALIZER(value) { (value) }

typedef struct {
  volatile uint32_t value;
} __attribute__((aligned(4))) Atomic32;

typedef struct {
  volatile uint64_t value;
} __attribute__((aligned(8))) Atomic64;

typedef struct {
  Atomic32 value;
} __attribute__((aligned(4))) AtomicBool;

/**
 * Memory load operations that precede this fence will be prevented from
 * changing order with any that follow this fence, by either the compiler or
 * the CPU. This can be used to ensure that the load operations accessing
 * the fields of a structure are not re-ordered so they actually take effect
 * before a pointer to the structure is resolved.
 **/
static INLINE void loadFence(void)
{
  __asm__ __volatile__("lfence" : : : "memory");
}

/**
 * Memory store operations that precede this fence will be prevented from
 * changing order with any that follow this fence, by either the compiler or
 * the CPU. This can be used to ensure that the store operations initializing
 * the fields of a structure are not re-ordered so they actually take effect
 * after a pointer to the structure is published.
 **/
static INLINE void storeFence(void)
{
  __asm__ __volatile__("sfence" : : : "memory");
}

/**
 * Generate a full memory fence for the compiler and CPU. Load and store
 * operations issued before the fence will not be re-ordered with operations
 * issued after the fence.
 **/
static INLINE void memoryFence(void)
{
  /*
   * X86 full fence. Supposedly __sync_synchronize() will do this, but
   * either the GCC documentation is a lie or GCC is broken.
   *
   * XXX http://blogs.sun.com/dave/entry/atomic_fetch_and_add_vs says
   * atomicAdd of zero may be a better way to spell this on current CPUs.
   */
  __asm__ __volatile__("mfence" : : : "memory");
}

/**
 * Stop GCC from moving memory operations across a point in the
 * instruction stream.  This is needed in association with the __sync
 * builtins, because (at least, as of GCC versions 4.6 and earlier on
 * x86_64) the __sync operations don't actually act as the barriers
 * the compiler documentation says they should.
 **/
static INLINE void gccFence(void)
{
  /*
   * asm volatile cannot be removed, and the memory clobber tells the
   * compiler not to move memory accesses past the asm.  We don't
   * actually need any instructions issued on x86_64, as synchronizing
   * instructions are ordered with respect to both loads and stores,
   * with some irrelevant-to-us exceptions.
   */
  __asm__ __volatile__("" : : : "memory");
}

/**
 * Access the value of a 32-bit atomic variable, ensuring that the load is not
 * re-ordered by the compiler or CPU with any subsequent load operations.
 *
 * @param atom  a pointer to the atomic variable to access
 *
 * @return the value that was in the atom at the moment it was accessed
 **/
static INLINE uint32_t atomicLoad32(const Atomic32 *atom)
{
  uint32_t value = atom->value;
  loadFence();
  return value;
}

/**
 * Access the value of a 64-bit atomic variable, ensuring that the memory load
 * is not re-ordered by the compiler or CPU with any subsequent load
 * operations.
 *
 * @param atom  a pointer to the atomic variable to access
 *
 * @return the value that was in the atom at the moment it was accessed
 **/
static INLINE uint64_t atomicLoad64(const Atomic64 *atom)
{
  uint64_t value = atom->value;
  loadFence();
  return value;
}

/**
 * Access the value of a boolean atomic variable, ensuring that the load is not
 * re-ordered by the compiler or CPU with any subsequent load operations.
 *
 * @param atom  a pointer to the atomic variable to access
 *
 * @return the value that was in the atom at the moment it was accessed
 **/
static INLINE bool atomicLoadBool(const AtomicBool *atom)
{
  return (atomicLoad32(&atom->value) > 0);
}

/**
 * Set the value of a 32-bit atomic variable, ensuring that the memory store
 * operation is not re-ordered by the compiler or CPU with any preceding store
 * operations.
 *
 * @param atom      a pointer to the atomic variable to modify
 * @param newValue  the value to assign to the atomic variable
 **/
static INLINE void atomicStore32(Atomic32 *atom, uint32_t newValue)
{
  storeFence();
  atom->value = newValue;
}

/**
 * Set the value of a 64-bit atomic variable, ensuring that the memory store
 * operation is not re-ordered by the compiler or CPU with any preceding store
 * operations.
 *
 * @param atom      a pointer to the atomic variable to modify
 * @param newValue  the value to assign to the atomic variable
 **/
static INLINE void atomicStore64(Atomic64 *atom, uint64_t newValue)
{
  storeFence();
  atom->value = newValue;
}

/**
 * Set the value of a boolean atomic variable, ensuring that the memory store
 * operation is not re-ordered by the compiler or CPU with any preceding store
 * operations.
 *
 * @param atom      a pointer to the atomic variable to modify
 * @param newValue  the value to assign to the atomic variable
 **/
static INLINE void atomicStoreBool(AtomicBool *atom, bool newValue)
{
  atomicStore32(&atom->value, (newValue ? 1 : 0));
}

/**
 * Add a 32-bit signed delta to a 32-bit atomic variable.
 *
 * @param atom   a pointer to the atomic variable
 * @param delta  the value to be added (or subtracted) from the variable
 *
 * @return       the new value of the atom after the add operation
 **/
static INLINE uint32_t atomicAdd32(Atomic32 *atom, int32_t delta)
{
  gccFence();
  uint32_t result = __sync_add_and_fetch(&atom->value, delta);
  gccFence();
  return result;
}

/**
 * Add a 64-bit signed delta to a 64-bit atomic variable.
 *
 * @param atom   a pointer to the atomic variable
 * @param delta  the value to be added (or subtracted) from the variable
 *
 * @return       the new value of the atom after the add operation
 **/
static INLINE uint64_t atomicAdd64(Atomic64 *atom, int64_t delta)
{
  gccFence();
  uint64_t result = __sync_add_and_fetch(&atom->value, delta);
  gccFence();
  return result;
}

/**
 * Atomic 32-bit compare-and-swap. If the atom is identical to a required
 * value, atomically replace it with the new value and return true, otherwise
 * do nothing and return false.
 *
 * @param atom           a pointer to the atomic variable
 * @param requiredValue  the value that must be present to perform the swap
 * @param newValue       the value to be swapped for the required value
 *
 * @return               true if the atom was changed, false otherwise
 **/
static INLINE bool compareAndSwap32(Atomic32 *atom,
                                    uint32_t  requiredValue,
                                    uint32_t  newValue)
{
  gccFence();
  bool result
    = __sync_bool_compare_and_swap(&atom->value, requiredValue, newValue);
  gccFence();
  return result;
}

/**
 * Atomic 64-bit compare-and-swap. If the atom is identical to a required
 * value, atomically replace it with the new value and return true, otherwise
 * do nothing and return false.
 *
 * @param atom           a pointer to the atomic variable
 * @param requiredValue  the value that must be present to perform the swap
 * @param newValue       the value to be swapped for the required value
 *
 * @return               true if the atom was changed, false otherwise
 **/
static INLINE bool compareAndSwap64(Atomic64 *atom,
                                    uint64_t  requiredValue,
                                    uint64_t  newValue)
{
  gccFence();
  bool result
    = __sync_bool_compare_and_swap(&atom->value, requiredValue, newValue);
  gccFence();
  return result;
}

/**
 * Atomic boolean compare-and-swap. If the atom is identical to a required
 * value, atomically replace it with the new value and return true, otherwise
 * do nothing and return false.
 *
 * @param atom           a pointer to the atomic variable
 * @param requiredValue  the value that must be present to perform the swap
 * @param newValue       the value to be swapped for the required value
 *
 * @return               true if the atom was changed, false otherwise
 **/
static INLINE bool compareAndSwapBool(AtomicBool *atom,
                                      bool        requiredValue,
                                      bool        newValue)
{
  return compareAndSwap32(&atom->value, (requiredValue ? 1 : 0),
                          (newValue ? 1 : 0));
}

/**
 * Access the value of a 32-bit atomic variable using relaxed memory order,
 * without any compiler or CPU fences.
 *
 * @param atom  a pointer to the atomic variable to access
 *
 * @return the value that was in the atom at the moment it was accessed
 **/
static INLINE uint32_t relaxedLoad32(const Atomic32 *atom)
{
  return atom->value;
}

/**
 * Access the value of a 64-bit atomic variable using relaxed memory order,
 * without any compiler or CPU fences.
 *
 * @param atom  a pointer to the atomic variable to access
 *
 * @return the value that was in the atom at the moment it was accessed
 **/
static INLINE uint64_t relaxedLoad64(const Atomic64 *atom)
{
  return atom->value;
}

/**
 * Access the value of a boolean atomic variable using relaxed memory order,
 * without any compiler or CPU fences.
 *
 * @param atom  a pointer to the atomic variable to access
 *
 * @return the value that was in the atom at the moment it was accessed
 **/
static INLINE bool relaxedLoadBool(const AtomicBool *atom)
{
  return (relaxedLoad32(&atom->value) > 0);
}

/**
 * Set the value of a 32-bit atomic variable using relaxed memory order,
 * without any compiler or CPU fences.
 *
 * @param atom      a pointer to the atomic variable to modify
 * @param newValue  the value to assign to the atomic variable
 **/
static INLINE void relaxedStore32(Atomic32 *atom, uint32_t newValue)
{
  atom->value = newValue;
}

/**
 * Set the value of a 64-bit atomic variable using relaxed memory order,
 * without any compiler or CPU fences.
 *
 * @param atom      a pointer to the atomic variable to modify
 * @param newValue  the value to assign to the atomic variable
 **/
static INLINE void relaxedStore64(Atomic64 *atom, uint64_t newValue)
{
  atom->value = newValue;
}

/**
 * Set the value of a boolean atomic variable using relaxed memory order,
 * without any compiler or CPU fences.
 *
 * @param atom      a pointer to the atomic variable to modify
 * @param newValue  the value to assign to the atomic variable
 **/
static INLINE void relaxedStoreBool(AtomicBool *atom, bool newValue)
{
  relaxedStore32(&atom->value, (newValue ? 1 : 0));
}

/**
 * Non-atomically add a 32-bit signed delta to a 32-bit atomic variable,
 * without any compiler or CPU fences.
 *
 * @param atom   a pointer to the atomic variable
 * @param delta  the value to be added (or subtracted) from the variable
 *
 * @return       the new value of the atom after the add operation
 **/
static INLINE uint32_t relaxedAdd32(Atomic32 *atom, int32_t delta)
{
  uint32_t newValue = (relaxedLoad32(atom) + delta);
  relaxedStore32(atom, newValue);
  return newValue;
}

/**
 * Non-atomically add a 64-bit signed delta to a 64-bit atomic variable,
 * without any compiler or CPU fences.
 *
 * @param atom   a pointer to the atomic variable
 * @param delta  the value to be added (or subtracted) from the variable
 *
 * @return       the new value of the atom after the add operation
 **/
static INLINE uint64_t relaxedAdd64(Atomic64 *atom, int64_t delta)
{
  uint64_t newValue = (relaxedLoad64(atom) + delta);
  relaxedStore64(atom, newValue);
  return newValue;
}

#endif /* ATOMIC_H */
