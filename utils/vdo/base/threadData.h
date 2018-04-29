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
 * $Id: //eng/vdo-releases/magnesium-rhel7.5/src/c++/vdo/base/threadData.h#1 $
 */

#ifndef THREAD_DATA_H
#define THREAD_DATA_H

#include "completion.h"

typedef enum {
  NOT_ACCESSING_SUPER_BLOCK = 0,
  READING_SUPER_BLOCK,
  WRITING_SUPER_BLOCK,
} SuperBlockAccessState;

/**
 * Data associated with each base code thread.
 **/
struct threadData {
  /** The completion for entering read-only mode */
  VDOCompletion          completion;
  /** The thread this represents */
  ThreadID               threadID;
  /** The thread configuration for the VDO */
  const ThreadConfig    *threadConfig;
  /** The next physical zone to allocate from */
  ZoneCount              nextAllocationZone;
  /** The number of allocations done in the nextAllocationZone */
  BlockCount             allocationCount;
  /** Whether this thread is in read-only mode */
  bool                   isReadOnly;
  /** Whether this thread is entering read-only mode */
  bool                   isEnteringReadOnlyMode;
  /** Whether this thread may enter read-only mode */
  bool                   mayEnterReadOnlyMode;
  /** The error code for entering read-only mode */
  int                    readOnlyError;
  /** Whether this thread is accessing the super block */
  SuperBlockAccessState  superBlockAccessState;
  /** A completion to notify when this thread is not entering read-only mode */
  VDOCompletion         *superBlockIdleWaiter;
  /** A completion which is waiting to enter read-only mode */
  VDOCompletion         *readOnlyModeWaiter;
};

/**
 * Initialize the data for a thread.
 *
 * @param threadData    The data to initialize
 * @param threadID      The ID of the thread this data is for
 * @param isReadOnly    <code>true</code> if this thread should be in
 *                      read-only mode
 * @param threadConfig  The thread configuration of the VDO
 * @param layer         The physical layer of the VDO
 *
 * @return VDO_SUCCESS or an error
 **/
int initializeThreadData(ThreadData         *threadData,
                         ThreadID            threadID,
                         bool                isReadOnly,
                         const ThreadConfig *threadConfig,
                         PhysicalLayer      *layer)
  __attribute__((warn_unused_result));

/**
 * Clean up thread data resources.
 *
 * @param threadData  The thread data to uninitialize
 **/
void uninitializeThreadData(ThreadData *threadData);

/**
 * Get the next physical zone from which to allocate.
 *
 * @param vdo       The VDO
 * @param threadID  The ID of the thread which wants to allocate
 *
 * @return The physical zone from which to allocate
 **/
PhysicalZone *getNextAllocationZone(VDO *vdo, ThreadID threadID)
  __attribute__((warn_unused_result));

/**
 * Wait until no threads are entering read-only mode.
 *
 * @param vdo     The VDO to wait on
 * @param waiter  The completion to notify when no threads are entering
 *                read-only mode
 **/
void waitUntilNotEnteringReadOnlyMode(VDO *vdo, VDOCompletion *waiter);

/**
 * Put a VDO into read-only mode.
 *
 * @param vdo             The VDO to put into read-only mode
 * @param errorCode       The error which caused the VDO to enter read-only
 *                        mode
 * @param saveSuperBlock  <code>true</code> if the read-only state should be
 *                        persisted to disk immediately
 **/
void makeVDOReadOnly(VDO *vdo, int errorCode, bool saveSuperBlock);

#endif /* THREAD_DATA_H */
