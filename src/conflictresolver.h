/*
  SPDX-FileCopyrightText: 2000, 2001, 2004 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  SPDX-FileCopyrightText: 2010 Andras Mantia <andras@kdab.com>
  SPDX-FileCopyrightText: 2010 Casey Link <casey@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "incidenceeditor_export.h"
#include <CalendarSupport/FreeBusyItem>

#include <QBitArray>
#include <QSet>
#include <QTimer>

namespace CalendarSupport
{
class FreeBusyItemModel;
}

namespace IncidenceEditorNG
{
/**
 * Takes a list of attendees and event info (e.g., min time start, max time end)
 * fetches their freebusy information, then identifies conflicts and periods of non-conflict.
 *
 * It exposes these periods so another class can display them to the user and allow
 * them to choose a correct time.
 * @author Casey Link
 */
class INCIDENCEEDITOR_EXPORT ConflictResolver : public QObject
{
    Q_OBJECT
public:
    /**
     * @param parentWidget is passed to Akonadi when fetching free/busy data.
     */
    explicit ConflictResolver(QWidget *parentWidget, QObject *parent = nullptr);

    /**
     *  Add an attendee
     * The attendees free busy info will be fetched
     * and integrated into the resolver.
     */
    void insertAttendee(const KCalendarCore::Attendee &attendee);

    void insertAttendee(const CalendarSupport::FreeBusyItem::Ptr &freebusy);
    /**
     * Removes an attendee
     * The attendee will no longer be considered when
     * resolving conflicts
     */

    void removeAttendee(const KCalendarCore::Attendee &attendee);

    /**
     * Clear all attendees
     */
    void clearAttendees();

    /**
     * Returns whether the resolver contains the attendee
     */
    Q_REQUIRED_RESULT bool containsAttendee(const KCalendarCore::Attendee &attendee);

    /**
     * Constrain the free time slot search to the weekdays
     * identified by their KCalendarSystem integer representation
     * Default is Monday - Friday
     * @param weekdays a 7 bit array indicating the allowed days (bit 0=Monday, value 1=allowed).
     * @see KCalendarSystem
     */
    void setAllowedWeekdays(const QBitArray &weekdays);

    /**
     * Constrain the free time slot search to the set participant roles.
     * Mandatory roles are considered the minimum required to attend
     * the meeting, so only those attendees with the mandatory roles will
     * be considered  in the search.
     * Default is all roles are mandatory.
     * @param roles the set of mandatory participant roles
     */
    void setMandatoryRoles(const QSet<KCalendarCore::Attendee::Role> &roles);

    /**
     * Returns a list of date time ranges that conform to the
     * search constraints.
     * @see setMandatoryRoles
     * @see setAllowedWeekdays
     */
    KCalendarCore::Period::List availableSlots() const;

    /**
      Finds a free slot in the future which has at least the same size as
      the initial slot.
    */
    Q_REQUIRED_RESULT bool findFreeSlot(const KCalendarCore::Period &dateTimeRange);

    CalendarSupport::FreeBusyItemModel *model() const;

Q_SIGNALS:
    /**
     * Emitted when the user changes the start and end dateTimes
     * for the incidence.
     */
    void dateTimesChanged(const QDateTime &newStart, const QDateTime &newEnd);

    /**
     * Emitted when there are conflicts
     * @param number the number of conflicts
     */
    void conflictsDetected(int number);

    /**
     * Emitted when the resolver locates new free slots.
     */
    void freeSlotsAvailable(const KCalendarCore::Period::List &);

public Q_SLOTS:
    /**
     * Set the timeframe constraints
     *
     * These control the timeframe for which conflicts are to be resolved.
     */
    void setEarliestDate(const QDate &newDate);
    void setEarliestTime(const QTime &newTime);
    void setLatestDate(const QDate &newDate);
    void setLatestTime(const QTime &newTime);

    void setEarliestDateTime(const QDateTime &newDateTime);
    void setLatestDateTime(const QDateTime &newDateTime);

    void freebusyDataChanged();

    void findAllFreeSlots();

    void setResolution(int seconds);

private:
    /**
      Checks whether the slot specified by (tryFrom, tryTo) matches the
      search constraints. If yes, return true. The return value is the
      number of conflicts that were detected, and (tryFrom, tryTo) contain the next free slot for
      that participant. In other words, the returned slot does not have to
      be free for everybody else.
    */
    int tryDate(QDateTime &tryFrom, QDateTime &tryTo);

    /**
      Checks whether the slot specified by (tryFrom, tryTo) is available
      for the participant with specified fb. If yes, return true. If
      not, return false and change (tryFrom, tryTo) to contain the next
      possible slot for this participant (not necessarily a slot that is
      available for all participants).
    */
    bool tryDate(const KCalendarCore::FreeBusy::Ptr &fb, QDateTime &tryFrom, QDateTime &tryTo);

    /**
     * Checks whether the supplied attendee passes the
     * current mandatory role constraint.
     * @return true if the attendee is of one of the mandatory roles, false if not
     */
    bool matchesRoleConstraint(const KCalendarCore::Attendee &attendee);

    void calculateConflicts();

    KCalendarCore::Period mTimeframeConstraint; //!< the datetime range for outside of which
    // free slots won't be searched.
    KCalendarCore::Period::List mAvailableSlots;

    QTimer mCalculateTimer; //!< A timer is used control the calculation of conflicts
    // to prevent the process from being repeated many times
    // after a series of quick parameter changes.

    CalendarSupport::FreeBusyItemModel *mFBModel = nullptr;
    QWidget *mParentWidget = nullptr;

    QSet<KCalendarCore::Attendee::Role> mMandatoryRoles;
    QBitArray mWeekdays; //!< a 7 bit array indicating the allowed days
    //(bit 0 = Monday, value 1 = allowed).

    int mSlotResolutionSeconds;
};
}

