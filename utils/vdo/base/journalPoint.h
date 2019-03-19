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
 * $Id: //eng/vdo-releases/magnesium/src/c++/vdo/base/journalPoint.h#1 $
 */

#ifndef JOURNAL_POINT_H
#define JOURNAL_POINT_H

#include "types.h"

typedef uint16_t JournalEntryCount;

/**
 * The absolute position of an entry in a recovery journal or slab journal.
 **/
typedef struct {
  SequenceNumber    sequenceNumber;
  JournalEntryCount entryCount;
} JournalPoint;

/**
 * A packed encoding of a JournalPoint.
 **/
typedef struct {
  /** 48 bits of sequence number (high-order bytes) | 16 bit entryCount */
  uint64_t encodedPoint;
} __attribute__((packed)) PackedJournalPoint;

/**
 * Move the given journal point forward by one entry.
 *
 * @param point            the journal point to adjust
 * @param entriesPerBlock  the number of entries in one full block
 **/
static inline void advanceJournalPoint(JournalPoint      *point,
                                       JournalEntryCount  entriesPerBlock)
{
  point->entryCount++;
  if (point->entryCount == entriesPerBlock) {
    point->sequenceNumber++;
    point->entryCount = 0;
  }
}

/**
 * Check whether a journal point is valid.
 *
 * @param point  the journal point
 *
 * @return <code>true</code> if the journal point is valid
 **/
static inline bool isValidJournalPoint(const JournalPoint *point)
{
  return ((point != NULL) && (point->sequenceNumber > 0));
}

/**
 * Check whether the first point precedes the second point.
 *
 * @param first   the first journal point
 * @param second  the second journal point

 *
 * @return <code>true</code> if the first point precedes the second point.
 **/
static inline bool beforeJournalPoint(const JournalPoint *first,
                                      const JournalPoint *second)
{
  return ((first->sequenceNumber < second->sequenceNumber)
          || ((first->sequenceNumber == second->sequenceNumber)
              && (first->entryCount < second->entryCount)));
}

/**
 * Check whether the first point is the same as the second point.
 *
 * @param first   the first journal point
 * @param second  the second journal point
 *
 * @return <code>true</code> if both points reference the same logical
 *         position of an entry the journal
 **/
static inline bool areEquivalentJournalPoints(const JournalPoint *first,
                                              const JournalPoint *second)
{
  return ((first->sequenceNumber == second->sequenceNumber)
          && (first->entryCount  == second->entryCount));
}

/**
 * Encode the journal location represented by a JournalPoint into a
 * PackedJournalPoint.
 *
 * @param unpacked  The unpacked input point
 * @param packed    The packed output point
 **/
static inline void packJournalPoint(const JournalPoint *unpacked,
                                    PackedJournalPoint *packed)
{
  packed->encodedPoint
    = ((unpacked->sequenceNumber << 16) | unpacked->entryCount);
}

/**
 * Decode the journal location represented by a PackedJournalPoint into a
 * JournalPoint.
 *
 * @param packed    The packed input point
 * @param unpacked  The unpacked output point
 **/
static inline void unpackJournalPoint(const PackedJournalPoint *packed,
                                      JournalPoint             *unpacked)
{
  unpacked->sequenceNumber = (packed->encodedPoint >> 16);
  unpacked->entryCount     = (packed->encodedPoint & 0xffff);
}

#endif // JOURNAL_POINT_H
