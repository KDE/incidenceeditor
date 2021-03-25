/*
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "incidenceeditor-ng.h"

#include <KCalendarCore/FreeBusy>
#include <KContacts/Addressee>
namespace Ui
{
class EventOrTodoDesktop;
}

namespace KContacts
{
class Addressee;
class ContactGroup;
}

class KJob;

namespace IncidenceEditorNG
{
class AttendeeComboBoxDelegate;
class AttendeeLineEditDelegate;
class AttendeeTableModel;
class ConflictResolver;
class IncidenceDateTime;

class IncidenceAttendee : public IncidenceEditor
{
    Q_OBJECT
public:
    using IncidenceEditorNG::IncidenceEditor::load; // So we don't trigger -Woverloaded-virtual
    using IncidenceEditorNG::IncidenceEditor::save; // So we don't trigger -Woverloaded-virtual
    explicit IncidenceAttendee(QWidget *parent, IncidenceDateTime *dateTime, Ui::EventOrTodoDesktop *ui);
    ~IncidenceAttendee() override;

    void load(const KCalendarCore::Incidence::Ptr &incidence) override;
    void save(const KCalendarCore::Incidence::Ptr &incidence) override;
    Q_REQUIRED_RESULT bool isDirty() const override;
    void printDebugInfo() const override;

    AttendeeTableModel *dataModel() const;
    AttendeeComboBoxDelegate *stateDelegate() const;
    AttendeeComboBoxDelegate *roleDelegate() const;
    AttendeeComboBoxDelegate *responseDelegate() const;
    AttendeeLineEditDelegate *attendeeDelegate() const;

    Q_REQUIRED_RESULT int attendeeCount() const;

Q_SIGNALS:
    void attendeeCountChanged(int);

public Q_SLOTS:
    /// If the user is attendee of the loaded event, one of the following slots
    /// can be used to change the status.
    void acceptForMe();
    void declineForMe();

private Q_SLOTS:
    // cheks if row is a group,  that can/should be expanded
    void checkIfExpansionIsNeeded(const KCalendarCore::Attendee &attendee);

    // results of the group search job
    void groupSearchResult(KJob *job);
    void expandResult(KJob *job);
    void slotSelectAddresses();
    void slotSolveConflictPressed();
    void slotUpdateConflictLabel(int);
    void slotOrganizerChanged(const QString &organizer);
    void slotGroupSubstitutionPressed();

    // wrapper for the conflict resolver
    void slotEventDurationChanged();

    void filterLayoutChanged();
    void updateCount();

    void slotConflictResolverAttendeeAdded(const QModelIndex &index, int first, int last);
    void slotConflictResolverAttendeeRemoved(const QModelIndex &index, int first, int last);
    void slotConflictResolverAttendeeChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void slotConflictResolverLayoutChanged();
    void slotFreeBusyAdded(const QModelIndex &index, int first, int last);
    void slotFreeBusyChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void updateFBStatus();
    void updateFBStatus(const KCalendarCore::Attendee &attendee, const KCalendarCore::FreeBusy::Ptr &fb);

    void slotGroupSubstitutionAttendeeAdded(const QModelIndex &index, int first, int last);
    void slotGroupSubstitutionAttendeeRemoved(const QModelIndex &index, int first, int last);
    void slotGroupSubstitutionAttendeeChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void slotGroupSubstitutionLayoutChanged();

private:
    void updateGroupExpand();

    void insertAddresses(const KContacts::Addressee::List &list);

    void changeStatusForMe(KCalendarCore::Attendee::PartStat);

    /** Returns if I was the organizer of the loaded event */
    bool iAmOrganizer() const;

    /** Reads values from a KContacts::Addressee and inserts a new Attendee
     * item into the listview with those items. Used when adding attendees
     * from the addressbook and expanding distribution lists.
     * The optional Attendee parameter can be used to pass in default values
     * to be used by the new Attendee.
     * pos =-1 means insert attendee before empty line
     */
    void insertAttendeeFromAddressee(const KContacts::Addressee &a, int pos = -1);
    void fillOrganizerCombo();
    void setActions(KCalendarCore::Incidence::IncidenceType actions);

    int rowOfAttendee(const QString &uid) const;

    Ui::EventOrTodoDesktop *mUi = nullptr;
    QWidget *mParentWidget = nullptr;
    ConflictResolver *mConflictResolver = nullptr;

    IncidenceDateTime *mDateTime = nullptr;
    QString mOrganizer;

    /** used dataModel to rely on*/
    AttendeeTableModel *mDataModel = nullptr;
    AttendeeLineEditDelegate *mAttendeeDelegate = nullptr;
    AttendeeComboBoxDelegate *const mStateDelegate;
    AttendeeComboBoxDelegate *mRoleDelegate = nullptr;
    AttendeeComboBoxDelegate *mResponseDelegate = nullptr;

    // the QString is Attendee::uid here
    QMap<QString, KContacts::ContactGroup> mGroupList;
    QMap<KJob *, QString> mMightBeGroupJobs;
    QMap<KJob *, QString> mExpandGroupJobs;
};
}

