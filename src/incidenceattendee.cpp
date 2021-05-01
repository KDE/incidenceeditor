/*
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  Based on old attendeeeditor.cpp:
  SPDX-FileCopyrightText: 2000, 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2007 Volker Krause <vkrause@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "incidenceattendee.h"
#include "attendeecomboboxdelegate.h"
#include "attendeeeditor.h"
#include "attendeelineeditdelegate.h"
#include "attendeetablemodel.h"
#include "conflictresolver.h"
#include "editorconfig.h"
#include "incidencedatetime.h"
#include "schedulingdialog.h"
#include "ui_dialogdesktop.h"
#include <CalendarSupport/FreeBusyItemModel>

#include <Akonadi/Contact/AbstractEmailAddressSelectionDialog>
#include <Akonadi/Contact/ContactGroupExpandJob>
#include <Akonadi/Contact/ContactGroupSearchJob>
#include <Akonadi/Contact/EmailAddressSelectionDialog>

#include <KCalUtils/Stringify>
#include <KEmailAddress>
#include <KPluginFactory>
#include <KPluginLoader>

#include "incidenceeditor_debug.h"
#include <KLocalizedString>
#include <KMessageBox>
#include <QPointer>
#include <QTreeView>

using namespace IncidenceEditorNG;

IncidenceAttendee::IncidenceAttendee(QWidget *parent, IncidenceDateTime *dateTime, Ui::EventOrTodoDesktop *ui)
    : mUi(ui)
    , mParentWidget(parent)
    , mDateTime(dateTime)
    , mStateDelegate(new AttendeeComboBoxDelegate(this))
    , mRoleDelegate(new AttendeeComboBoxDelegate(this))
    , mResponseDelegate(new AttendeeComboBoxDelegate(this))
{
    mDataModel = new AttendeeTableModel(this);
    mDataModel->setKeepEmpty(true);
    mDataModel->setRemoveEmptyLines(true);
    mRoleDelegate->addItem(QIcon::fromTheme(QStringLiteral(":/meeting-participant.png")),
                           KCalUtils::Stringify::attendeeRole(KCalendarCore::Attendee::ReqParticipant));
    mRoleDelegate->addItem(QIcon::fromTheme(QStringLiteral(":/meeting-participant-optional.png")),
                           KCalUtils::Stringify::attendeeRole(KCalendarCore::Attendee::OptParticipant));
    mRoleDelegate->addItem(QIcon::fromTheme(QStringLiteral(":/meeting-observer.png")),
                           KCalUtils::Stringify::attendeeRole(KCalendarCore::Attendee::NonParticipant));
    mRoleDelegate->addItem(QIcon::fromTheme(QStringLiteral(":/meeting-chair.png")), KCalUtils::Stringify::attendeeRole(KCalendarCore::Attendee::Chair));

    mResponseDelegate->addItem(QIcon::fromTheme(QStringLiteral("meeting-participant-request-response")), i18nc("@item:inlistbox", "Request Response"));
    mResponseDelegate->addItem(QIcon::fromTheme(QStringLiteral("meeting-participant-no-response")), i18nc("@item:inlistbox", "Request No Response"));

    mStateDelegate->setWhatsThis(i18nc("@info:whatsthis", "Edits the current attendance status of the attendee."));

    mRoleDelegate->setWhatsThis(i18nc("@info:whatsthis", "Edits the role of the attendee."));

    mResponseDelegate->setToolTip(i18nc("@info:tooltip", "Request a response from the attendee"));
    mResponseDelegate->setWhatsThis(i18nc("@info:whatsthis",
                                          "Edits whether to send an email to the "
                                          "attendee to request a response concerning "
                                          "attendance."));

    setObjectName(QStringLiteral("IncidenceAttendee"));

    auto filterProxyModel = new AttendeeFilterProxyModel(this);
    filterProxyModel->setDynamicSortFilter(true);
    filterProxyModel->setSourceModel(mDataModel);

    connect(mUi->mGroupSubstitution, &QPushButton::clicked, this, &IncidenceAttendee::slotGroupSubstitutionPressed);

    mUi->mAttendeeTable->setModel(filterProxyModel);

    mAttendeeDelegate = new AttendeeLineEditDelegate(this);

    mUi->mAttendeeTable->setItemDelegateForColumn(AttendeeTableModel::Role, roleDelegate());
    mUi->mAttendeeTable->setItemDelegateForColumn(AttendeeTableModel::FullName, attendeeDelegate());
    mUi->mAttendeeTable->setItemDelegateForColumn(AttendeeTableModel::Status, stateDelegate());
    mUi->mAttendeeTable->setItemDelegateForColumn(AttendeeTableModel::Response, responseDelegate());

    mUi->mOrganizerStack->setCurrentIndex(0);

    fillOrganizerCombo();
    mUi->mSolveButton->setEnabled(false);
    mUi->mOrganizerLabel->setVisible(false);

    mConflictResolver = new ConflictResolver(parent, parent);
    mConflictResolver->setEarliestDate(mDateTime->startDate());
    mConflictResolver->setEarliestTime(mDateTime->startTime());
    mConflictResolver->setLatestDate(mDateTime->endDate());
    mConflictResolver->setLatestTime(mDateTime->endTime());

    connect(mUi->mSelectButton, &QPushButton::clicked, this, &IncidenceAttendee::slotSelectAddresses);
    connect(mUi->mSolveButton, &QPushButton::clicked, this, &IncidenceAttendee::slotSolveConflictPressed);
    /* Added as part of kolab/issue2297, which is currently under review
    connect(mUi->mOrganizerCombo, qOverload<const QString &>(&QComboBox::activated),
            this, &IncidenceAttendee::slotOrganizerChanged);
    */
    connect(mUi->mOrganizerCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &IncidenceAttendee::checkDirtyStatus);

    connect(mDateTime, &IncidenceDateTime::startDateChanged, this, &IncidenceAttendee::slotEventDurationChanged);
    connect(mDateTime, &IncidenceDateTime::endDateChanged, this, &IncidenceAttendee::slotEventDurationChanged);
    connect(mDateTime, &IncidenceDateTime::startTimeChanged, this, &IncidenceAttendee::slotEventDurationChanged);
    connect(mDateTime, &IncidenceDateTime::endTimeChanged, this, &IncidenceAttendee::slotEventDurationChanged);

    connect(mConflictResolver, &ConflictResolver::conflictsDetected, this, &IncidenceAttendee::slotUpdateConflictLabel);

    connect(mConflictResolver->model(), &QAbstractItemModel::rowsInserted, this, &IncidenceAttendee::slotFreeBusyAdded);
    connect(mConflictResolver->model(), &QAbstractItemModel::layoutChanged, this, qOverload<>(&IncidenceAttendee::updateFBStatus));
    connect(mConflictResolver->model(), &QAbstractItemModel::dataChanged, this, &IncidenceAttendee::slotFreeBusyChanged);

    slotUpdateConflictLabel(0); // initialize label

    // conflict resolver (should show also resources)
    connect(mDataModel, &AttendeeTableModel::layoutChanged, this, &IncidenceAttendee::slotConflictResolverLayoutChanged);
    connect(mDataModel, &AttendeeTableModel::rowsAboutToBeRemoved, this, &IncidenceAttendee::slotConflictResolverAttendeeRemoved);
    connect(mDataModel, &AttendeeTableModel::rowsInserted, this, &IncidenceAttendee::slotConflictResolverAttendeeAdded);
    connect(mDataModel, &AttendeeTableModel::dataChanged, this, &IncidenceAttendee::slotConflictResolverAttendeeChanged);

    // Group substitution
    connect(filterProxyModel, &AttendeeFilterProxyModel::layoutChanged, this, &IncidenceAttendee::slotGroupSubstitutionLayoutChanged);
    connect(filterProxyModel, &AttendeeFilterProxyModel::rowsAboutToBeRemoved, this, &IncidenceAttendee::slotGroupSubstitutionAttendeeRemoved);
    connect(filterProxyModel, &AttendeeFilterProxyModel::rowsInserted, this, &IncidenceAttendee::slotGroupSubstitutionAttendeeAdded);
    connect(filterProxyModel, &AttendeeFilterProxyModel::dataChanged, this, &IncidenceAttendee::slotGroupSubstitutionAttendeeChanged);

    connect(filterProxyModel, &AttendeeFilterProxyModel::rowsInserted, this, &IncidenceAttendee::updateCount);
    connect(filterProxyModel, &AttendeeFilterProxyModel::rowsRemoved, this, &IncidenceAttendee::updateCount);
    // only update when FullName is changed
    connect(filterProxyModel, &AttendeeFilterProxyModel::dataChanged, this, &IncidenceAttendee::updateCount);
    connect(filterProxyModel, &AttendeeFilterProxyModel::layoutChanged, this, &IncidenceAttendee::updateCount);
    connect(filterProxyModel, &AttendeeFilterProxyModel::layoutChanged, this, &IncidenceAttendee::filterLayoutChanged);
}

IncidenceAttendee::~IncidenceAttendee()
{
}

void IncidenceAttendee::load(const KCalendarCore::Incidence::Ptr &incidence)
{
    mLoadedIncidence = incidence;

    if (iAmOrganizer() || incidence->organizer().isEmpty()) {
        mUi->mOrganizerStack->setCurrentIndex(0);

        int found = -1;
        const QString fullOrganizer = incidence->organizer().fullName();
        const QString organizerEmail = incidence->organizer().email();
        for (int i = 0; i < mUi->mOrganizerCombo->count(); ++i) {
            KCalendarCore::Person organizerCandidate = KCalendarCore::Person::fromFullName(mUi->mOrganizerCombo->itemText(i));
            if (organizerCandidate.email() == organizerEmail) {
                found = i;
                mUi->mOrganizerCombo->setCurrentIndex(i);
                break;
            }
        }
        if (found < 0 && !fullOrganizer.isEmpty()) {
            mUi->mOrganizerCombo->insertItem(0, fullOrganizer);
            mUi->mOrganizerCombo->setCurrentIndex(0);
        }

        mUi->mOrganizerLabel->setVisible(false);
    } else { // someone else is the organizer
        mUi->mOrganizerStack->setCurrentIndex(1);
        mUi->mOrganizerLabel->setText(incidence->organizer().fullName());
        mUi->mOrganizerLabel->setVisible(true);
    }

    KCalendarCore::Attendee::List attendees;
    const KCalendarCore::Attendee::List incidenceAttendees = incidence->attendees();
    attendees.reserve(incidenceAttendees.count());
    for (const KCalendarCore::Attendee &a : incidenceAttendees) {
        attendees << KCalendarCore::Attendee(a);
    }

    mDataModel->setAttendees(attendees);
    slotUpdateConflictLabel(0);

    setActions(incidence->type());

    mWasDirty = false;
}

void IncidenceAttendee::save(const KCalendarCore::Incidence::Ptr &incidence)
{
    incidence->clearAttendees();
    const KCalendarCore::Attendee::List attendees = mDataModel->attendees();

    for (const KCalendarCore::Attendee &attendee : attendees) {
        bool skip = false;
        if (attendee.fullName().isEmpty()) {
            continue;
        }
        if (KEmailAddress::isValidAddress(attendee.email())) {
            if (KMessageBox::warningYesNo(nullptr,
                                          i18nc("@info",
                                                "%1 does not look like a valid email address. "
                                                "Are you sure you want to invite this participant?",
                                                attendee.email()),
                                          i18nc("@title:window", "Invalid Email Address"))
                != KMessageBox::Yes) {
                skip = true;
            }
        }
        if (!skip) {
            incidence->addAttendee(attendee);
        }
    }

    // Must not have an organizer for items without attendees
    if (!incidence->attendeeCount()) {
        return;
    }

    if (mUi->mOrganizerStack->currentIndex() == 0) {
        incidence->setOrganizer(mUi->mOrganizerCombo->currentText());
    } else {
        incidence->setOrganizer(mUi->mOrganizerLabel->text());
    }
}

bool IncidenceAttendee::isDirty() const
{
    if (iAmOrganizer()) {
        KCalendarCore::Event tmp;
        tmp.setOrganizer(mUi->mOrganizerCombo->currentText());

        if (mLoadedIncidence->organizer().email() != tmp.organizer().email()) {
            qCDebug(INCIDENCEEDITOR_LOG) << "Organizer changed. Old was " << mLoadedIncidence->organizer().name() << mLoadedIncidence->organizer().email()
                                         << "; new is " << tmp.organizer().name() << tmp.organizer().email();
            return true;
        }
    }

    const KCalendarCore::Attendee::List originalList = mLoadedIncidence->attendees();
    KCalendarCore::Attendee::List newList;

    const auto lstAttendees = mDataModel->attendees();
    for (const KCalendarCore::Attendee &attendee : lstAttendees) {
        if (!attendee.fullName().isEmpty()) {
            newList.append(attendee);
        }
    }

    // The lists sizes *must* be the same. When the organizer is attending the
    // event as well, he should be in the attendees list as well.
    if (originalList.size() != newList.size()) {
        return true;
    }

    // Okay, again not the most efficient algorithm, but I'm assuming that in the
    // bulk of the use cases, the number of attendees is not much higher than 10 or so.
    for (const KCalendarCore::Attendee &attendee : originalList) {
        bool found = false;
        for (int i = 0; i < newList.count(); ++i) {
            if (newList[i] == attendee) {
                newList.remove(i);
                found = true;
                break;
            }
        }

        if (!found) {
            // One of the attendees in the original list was not found in the new list.
            return true;
        }
    }

    return false;
}

void IncidenceAttendee::changeStatusForMe(KCalendarCore::Attendee::PartStat stat)
{
    const IncidenceEditorNG::EditorConfig *config = IncidenceEditorNG::EditorConfig::instance();
    Q_ASSERT(config);

    for (int i = 0; i < mDataModel->rowCount(); ++i) {
        QModelIndex index = mDataModel->index(i, AttendeeTableModel::Email);
        if (config->thatIsMe(mDataModel->data(index, Qt::DisplayRole).toString())) {
            index = mDataModel->index(i, AttendeeTableModel::Status);
            mDataModel->setData(index, stat);
            break;
        }
    }

    checkDirtyStatus();
}

void IncidenceAttendee::acceptForMe()
{
    changeStatusForMe(KCalendarCore::Attendee::Accepted);
}

void IncidenceAttendee::declineForMe()
{
    changeStatusForMe(KCalendarCore::Attendee::Declined);
}

void IncidenceAttendee::fillOrganizerCombo()
{
    mUi->mOrganizerCombo->clear();
    const QStringList lst = IncidenceEditorNG::EditorConfig::instance()->fullEmails();
    QStringList uniqueList;
    for (QStringList::ConstIterator it = lst.begin(), end = lst.end(); it != end; ++it) {
        if (!uniqueList.contains(*it)) {
            uniqueList << *it;
        }
    }
    mUi->mOrganizerCombo->addItems(uniqueList);
}

void IncidenceAttendee::checkIfExpansionIsNeeded(const KCalendarCore::Attendee &attendee)
{
    QString fullname = attendee.fullName();

    // stop old job
    KJob *oldJob = mMightBeGroupJobs.key(attendee.uid());
    if (oldJob != nullptr) {
        disconnect(oldJob);
        oldJob->deleteLater();
        mMightBeGroupJobs.remove(oldJob);
    }

    mGroupList.remove(attendee.uid());

    if (!fullname.isEmpty()) {
        auto job = new Akonadi::ContactGroupSearchJob();
        job->setQuery(Akonadi::ContactGroupSearchJob::Name, fullname);
        connect(job, &Akonadi::ContactGroupSearchJob::result, this, &IncidenceAttendee::groupSearchResult);

        mMightBeGroupJobs.insert(job, attendee.uid());
    }
}

void IncidenceAttendee::groupSearchResult(KJob *job)
{
    auto searchJob = qobject_cast<Akonadi::ContactGroupSearchJob *>(job);
    Q_ASSERT(searchJob);

    Q_ASSERT(mMightBeGroupJobs.contains(job));
    const auto uid = mMightBeGroupJobs.take(job);

    const KContacts::ContactGroup::List contactGroups = searchJob->contactGroups();
    if (contactGroups.isEmpty()) {
        updateGroupExpand();
        return; // Nothing todo, probably a normal email address was entered
    }

    // TODO: Give the user the possibility to choose a group when there is more than one?!
    KContacts::ContactGroup group = contactGroups.first();

    const int row = rowOfAttendee(uid);
    QModelIndex index = dataModel()->index(row, AttendeeTableModel::CuType);
    dataModel()->setData(index, KCalendarCore::Attendee::Group);

    mGroupList.insert(uid, group);
    updateGroupExpand();
}

void IncidenceAttendee::updateGroupExpand()
{
    mUi->mGroupSubstitution->setEnabled(!mGroupList.isEmpty());
}

void IncidenceAttendee::slotGroupSubstitutionPressed()
{
    for (auto it = mGroupList.cbegin(), end = mGroupList.cend(); it != end; ++it) {
        auto expandJob = new Akonadi::ContactGroupExpandJob(it.value(), this);
        connect(expandJob, &Akonadi::ContactGroupExpandJob::result, this, &IncidenceAttendee::expandResult);
        mExpandGroupJobs.insert(expandJob, it.key());
        expandJob->start();
    }
}

void IncidenceAttendee::expandResult(KJob *job)
{
    auto expandJob = qobject_cast<Akonadi::ContactGroupExpandJob *>(job);
    Q_ASSERT(expandJob);
    Q_ASSERT(mExpandGroupJobs.contains(job));
    const auto uid = mExpandGroupJobs.take(job);
    const int row = rowOfAttendee(uid);
    const auto attendee = dataModel()->attendees().at(row);
    const QString currentEmail = attendee.email();
    const KContacts::Addressee::List groupMembers = expandJob->contacts();
    bool wasACorrectEmail = false;
    for (const KContacts::Addressee &member : groupMembers) {
        if (member.preferredEmail() == currentEmail) {
            wasACorrectEmail = true;
            break;
        }
    }

    if (!wasACorrectEmail) {
        dataModel()->removeRow(row);
        for (const KContacts::Addressee &member : groupMembers) {
            KCalendarCore::Attendee newAt(member.realName(), member.preferredEmail(), attendee.RSVP(), attendee.status(), attendee.role(), member.uid());
            dataModel()->insertAttendee(row, newAt);
        }
    }
}

void IncidenceAttendee::insertAddresses(const KContacts::Addressee::List &list)
{
    for (const KContacts::Addressee &contact : list) {
        insertAttendeeFromAddressee(contact);
    }
}

void IncidenceAttendee::slotSelectAddresses()
{
    QPointer<Akonadi::AbstractEmailAddressSelectionDialog> dialog;
    KPluginLoader loader(QStringLiteral("akonadi/emailaddressselectionldapdialogplugin"));
    KPluginFactory *factory = loader.factory();
    if (factory) {
        dialog = factory->create<Akonadi::AbstractEmailAddressSelectionDialog>(mParentWidget);
    } else {
        dialog = new Akonadi::EmailAddressSelectionDialog(mParentWidget);
    }
    dialog->view()->view()->setSelectionMode(QAbstractItemView::ExtendedSelection);
    dialog->setWindowTitle(i18nc("@title:window", "Select Attendees"));
    connect(dialog.data(), &Akonadi::AbstractEmailAddressSelectionDialog::insertAddresses, this, &IncidenceEditorNG::IncidenceAttendee::insertAddresses);
    if (dialog->exec() == QDialog::Accepted) {
        const Akonadi::EmailAddressSelection::List list = dialog->selectedAddresses();
        for (const Akonadi::EmailAddressSelection &selection : list) {
            if (selection.item().hasPayload<KContacts::ContactGroup>()) {
                auto job = new Akonadi::ContactGroupExpandJob(selection.item().payload<KContacts::ContactGroup>(), this);
                connect(job, &Akonadi::ContactGroupExpandJob::result, this, &IncidenceAttendee::expandResult);
                KCalendarCore::Attendee::PartStat partStat = KCalendarCore::Attendee::NeedsAction;
                bool rsvp = true;

                int pos = 0;
                KCalendarCore::Attendee newAt(selection.name(), selection.email(), rsvp, partStat, KCalendarCore::Attendee::ReqParticipant);
                dataModel()->insertAttendee(pos, newAt);

                mExpandGroupJobs.insert(job, newAt.uid());
                job->start();
            } else {
                KContacts::Addressee contact;
                contact.setName(selection.name());
                contact.insertEmail(selection.email());

                if (selection.item().hasPayload<KContacts::Addressee>()) {
                    contact.setUid(selection.item().payload<KContacts::Addressee>().uid());
                }
                insertAttendeeFromAddressee(contact);
            }
        }
    }
    delete dialog;
}

void IncidenceEditorNG::IncidenceAttendee::slotSolveConflictPressed()
{
    const int duration = mDateTime->startTime().secsTo(mDateTime->endTime());
    QScopedPointer<SchedulingDialog> dialog(new SchedulingDialog(mDateTime->startDate(), mDateTime->startTime(), duration, mConflictResolver, mParentWidget));
    dialog->slotUpdateIncidenceStartEnd(mDateTime->currentStartDateTime(), mDateTime->currentEndDateTime());
    if (dialog->exec() == QDialog::Accepted) {
        qCDebug(INCIDENCEEDITOR_LOG) << dialog->selectedStartDate() << dialog->selectedStartTime();
        if (dialog->selectedStartDate().isValid() && dialog->selectedStartTime().isValid()) {
            mDateTime->setStartDate(dialog->selectedStartDate());
            mDateTime->setStartTime(dialog->selectedStartTime());
        }
    }
}

void IncidenceAttendee::slotConflictResolverAttendeeChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (AttendeeTableModel::FullName <= bottomRight.column() && AttendeeTableModel::FullName >= topLeft.column()) {
        for (int i = topLeft.row(); i <= bottomRight.row(); ++i) {
            QModelIndex email = dataModel()->index(i, AttendeeTableModel::Email);
            auto attendee = dataModel()->data(email, AttendeeTableModel::AttendeeRole).value<KCalendarCore::Attendee>();
            if (mConflictResolver->containsAttendee(attendee)) {
                mConflictResolver->removeAttendee(attendee);
            }
            if (!dataModel()->data(email).toString().isEmpty()) {
                mConflictResolver->insertAttendee(attendee);
            }
        }
    }
    checkDirtyStatus();
}

void IncidenceAttendee::slotConflictResolverAttendeeAdded(const QModelIndex &index, int first, int last)
{
    for (int i = first; i <= last; ++i) {
        QModelIndex email = dataModel()->index(i, AttendeeTableModel::Email, index);
        if (!dataModel()->data(email).toString().isEmpty()) {
            mConflictResolver->insertAttendee(dataModel()->data(email, AttendeeTableModel::AttendeeRole).value<KCalendarCore::Attendee>());
        }
    }
    checkDirtyStatus();
}

void IncidenceAttendee::slotConflictResolverAttendeeRemoved(const QModelIndex &index, int first, int last)
{
    for (int i = first; i <= last; ++i) {
        QModelIndex email = dataModel()->index(i, AttendeeTableModel::Email, index);
        if (!dataModel()->data(email).toString().isEmpty()) {
            mConflictResolver->removeAttendee(dataModel()->data(email, AttendeeTableModel::AttendeeRole).value<KCalendarCore::Attendee>());
        }
    }
    checkDirtyStatus();
}

void IncidenceAttendee::slotConflictResolverLayoutChanged()
{
    const KCalendarCore::Attendee::List attendees = mDataModel->attendees();
    mConflictResolver->clearAttendees();
    for (const KCalendarCore::Attendee &attendee : attendees) {
        if (!attendee.email().isEmpty()) {
            mConflictResolver->insertAttendee(attendee);
        }
    }
    checkDirtyStatus();
}

void IncidenceAttendee::slotFreeBusyAdded(const QModelIndex &parent, int first, int last)
{
    // We are only interested in toplevel changes
    if (parent.isValid()) {
        return;
    }
    QAbstractItemModel *model = mConflictResolver->model();
    for (int i = first; i <= last; ++i) {
        QModelIndex index = model->index(i, 0, parent);
        const KCalendarCore::Attendee &attendee = model->data(index, CalendarSupport::FreeBusyItemModel::AttendeeRole).value<KCalendarCore::Attendee>();
        const KCalendarCore::FreeBusy::Ptr &fb = model->data(index, CalendarSupport::FreeBusyItemModel::FreeBusyRole).value<KCalendarCore::FreeBusy::Ptr>();
        if (!attendee.isNull()) {
            updateFBStatus(attendee, fb);
        }
    }
}

void IncidenceAttendee::slotFreeBusyChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    // We are only interested in toplevel changes
    if (topLeft.parent().isValid()) {
        return;
    }
    QAbstractItemModel *model = mConflictResolver->model();
    for (int i = topLeft.row(); i <= bottomRight.row(); ++i) {
        QModelIndex index = model->index(i, 0);
        const KCalendarCore::Attendee &attendee = model->data(index, CalendarSupport::FreeBusyItemModel::AttendeeRole).value<KCalendarCore::Attendee>();
        const KCalendarCore::FreeBusy::Ptr &fb = model->data(index, CalendarSupport::FreeBusyItemModel::FreeBusyRole).value<KCalendarCore::FreeBusy::Ptr>();
        if (!attendee.isNull()) {
            updateFBStatus(attendee, fb);
        }
    }
}

void IncidenceAttendee::updateFBStatus()
{
    QAbstractItemModel *model = mConflictResolver->model();
    for (int i = 0; i < model->rowCount(); ++i) {
        QModelIndex index = model->index(i, 0);
        const KCalendarCore::Attendee &attendee = model->data(index, CalendarSupport::FreeBusyItemModel::AttendeeRole).value<KCalendarCore::Attendee>();
        const KCalendarCore::FreeBusy::Ptr &fb = model->data(index, CalendarSupport::FreeBusyItemModel::FreeBusyRole).value<KCalendarCore::FreeBusy::Ptr>();
        if (!attendee.isNull()) {
            updateFBStatus(attendee, fb);
        }
    }
}

void IncidenceAttendee::updateFBStatus(const KCalendarCore::Attendee &attendee, const KCalendarCore::FreeBusy::Ptr &fb)
{
    KCalendarCore::Attendee::List attendees = mDataModel->attendees();
    QDateTime startTime = mDateTime->currentStartDateTime();
    QDateTime endTime = mDateTime->currentEndDateTime();
    if (attendees.contains(attendee)) {
        int row = dataModel()->attendees().indexOf(attendee);
        QModelIndex attendeeIndex = dataModel()->index(row, AttendeeTableModel::Available);
        if (fb) {
            KCalendarCore::Period::List busyPeriods = fb->busyPeriods();
            for (auto it = busyPeriods.begin(); it != busyPeriods.end(); ++it) {
                // periods started before and lapping into the incidence (s < startTime && e >= startTime)
                // periods starting in the time of incidence (s >= startTime && s <= endTime)
                if (((*it).start() < startTime && (*it).end() > startTime) || ((*it).start() >= startTime && (*it).start() <= endTime)) {
                    switch (attendee.status()) {
                    case KCalendarCore::Attendee::Accepted:
                        dataModel()->setData(attendeeIndex, AttendeeTableModel::Accepted);
                        return;
                    default:
                        dataModel()->setData(attendeeIndex, AttendeeTableModel::Busy);
                        return;
                    }
                }
            }
            dataModel()->setData(attendeeIndex, AttendeeTableModel::Free);
        } else {
            dataModel()->setData(attendeeIndex, AttendeeTableModel::Unknown);
        }
    }
}

void IncidenceAttendee::slotUpdateConflictLabel(int count)
{
    if (attendeeCount() > 0) {
        mUi->mSolveButton->setEnabled(true);
        if (count > 0) {
            QString label = i18ncp("@label Shows the number of scheduling conflicts", "%1 conflict", "%1 conflicts", count);
            mUi->mConflictsLabel->setText(label);
            mUi->mConflictsLabel->setVisible(true);
        } else {
            mUi->mConflictsLabel->setVisible(false);
        }
    } else {
        mUi->mSolveButton->setEnabled(false);
        mUi->mConflictsLabel->setVisible(false);
    }
}

void IncidenceAttendee::slotGroupSubstitutionAttendeeChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (AttendeeTableModel::FullName <= bottomRight.column() && AttendeeTableModel::FullName >= topLeft.column()) {
        for (int i = topLeft.row(); i <= bottomRight.row(); ++i) {
            QModelIndex email = dataModel()->index(i, AttendeeTableModel::Email);
            auto attendee = dataModel()->data(email, AttendeeTableModel::AttendeeRole).value<KCalendarCore::Attendee>();
            checkIfExpansionIsNeeded(attendee);
        }
    }
    updateGroupExpand();
}

void IncidenceAttendee::slotGroupSubstitutionAttendeeAdded(const QModelIndex &index, int first, int last)
{
    Q_UNUSED(index)
    for (int i = first; i <= last; ++i) {
        QModelIndex email = dataModel()->index(i, AttendeeTableModel::Email);
        auto attendee = dataModel()->data(email, AttendeeTableModel::AttendeeRole).value<KCalendarCore::Attendee>();
        checkIfExpansionIsNeeded(attendee);
    }
    updateGroupExpand();
}

void IncidenceAttendee::slotGroupSubstitutionAttendeeRemoved(const QModelIndex &index, int first, int last)
{
    Q_UNUSED(index)
    for (int i = first; i <= last; ++i) {
        QModelIndex email = dataModel()->index(i, AttendeeTableModel::Email);
        auto attendee = dataModel()->data(email, AttendeeTableModel::AttendeeRole).value<KCalendarCore::Attendee>();
        KJob *job = mMightBeGroupJobs.key(attendee.uid());
        if (job) {
            disconnect(job);
            job->deleteLater();
            mMightBeGroupJobs.remove(job);
        }
        job = mExpandGroupJobs.key(attendee.uid());
        if (job) {
            disconnect(job);
            job->deleteLater();
            mExpandGroupJobs.remove(job);
        }
        mGroupList.remove(attendee.uid());
    }
    updateGroupExpand();
}

void IncidenceAttendee::slotGroupSubstitutionLayoutChanged()
{
    for (auto it = mMightBeGroupJobs.cbegin(), end = mMightBeGroupJobs.cend(); it != end; ++it) {
        KJob *job = it.key();
        disconnect(job);
        job->deleteLater();
    }

    for (auto it = mExpandGroupJobs.cbegin(), end = mExpandGroupJobs.cend(); it != end; ++it) {
        KJob *job = it.key();
        disconnect(job);
        job->deleteLater();
    }
    mMightBeGroupJobs.clear();
    mExpandGroupJobs.clear();
    mGroupList.clear();

    QAbstractItemModel *model = mUi->mAttendeeTable->model();
    if (!model) {
        return;
    }
    for (int i = 0; i < model->rowCount(QModelIndex()); ++i) {
        QModelIndex index = model->index(i, AttendeeTableModel::FullName);
        if (!model->data(index).toString().isEmpty()) {
            QModelIndex email = dataModel()->index(i, AttendeeTableModel::Email);
            auto attendee = dataModel()->data(email, AttendeeTableModel::AttendeeRole).value<KCalendarCore::Attendee>();
            checkIfExpansionIsNeeded(attendee);
        }
    }

    updateGroupExpand();
}

bool IncidenceAttendee::iAmOrganizer() const
{
    if (mLoadedIncidence) {
        const IncidenceEditorNG::EditorConfig *config = IncidenceEditorNG::EditorConfig::instance();
        return config->thatIsMe(mLoadedIncidence->organizer().email());
    }

    return true;
}

void IncidenceAttendee::insertAttendeeFromAddressee(const KContacts::Addressee &a, int pos /*=-1*/)
{
    const bool sameAsOrganizer = mUi->mOrganizerCombo && KEmailAddress::compareEmail(a.preferredEmail(), mUi->mOrganizerCombo->currentText(), false);
    KCalendarCore::Attendee::PartStat partStat = KCalendarCore::Attendee::NeedsAction;
    bool rsvp = true;

    if (iAmOrganizer() && sameAsOrganizer) {
        partStat = KCalendarCore::Attendee::Accepted;
        rsvp = false;
    }
    KCalendarCore::Attendee newAt(a.realName(), a.preferredEmail(), rsvp, partStat, KCalendarCore::Attendee::ReqParticipant, a.uid());
    if (pos < 0) {
        pos = dataModel()->rowCount() - 1;
    }

    dataModel()->insertAttendee(pos, newAt);
}

void IncidenceAttendee::slotEventDurationChanged()
{
    const QDateTime start = mDateTime->currentStartDateTime();
    const QDateTime end = mDateTime->currentEndDateTime();

    if (start >= end) { // This can happen, especially for todos.
        return;
    }

    mConflictResolver->setEarliestDateTime(start);
    mConflictResolver->setLatestDateTime(end);
    updateFBStatus();
}

void IncidenceAttendee::slotOrganizerChanged(const QString &newOrganizer)
{
    if (KEmailAddress::compareEmail(newOrganizer, mOrganizer, false)) {
        return;
    }

    QString name;
    QString email;
    bool success = KEmailAddress::extractEmailAddressAndName(newOrganizer, email, name);

    if (!success) {
        qCWarning(INCIDENCEEDITOR_LOG) << "Could not extract email address and name";
        return;
    }

    int currentOrganizerAttendee = -1;
    int newOrganizerAttendee = -1;

    for (int i = 0; i < mDataModel->rowCount(); ++i) {
        QModelIndex index = mDataModel->index(i, AttendeeTableModel::FullName);
        QString fullName = mDataModel->data(index, Qt::DisplayRole).toString();
        if (fullName == mOrganizer) {
            currentOrganizerAttendee = i;
        }

        if (fullName == newOrganizer) {
            newOrganizerAttendee = i;
        }
    }

    int answer = KMessageBox::No;
    if (currentOrganizerAttendee > -1) {
        answer = KMessageBox::questionYesNo(mParentWidget,
                                            i18nc("@option",
                                                  "You are changing the organizer of this event. "
                                                  "Since the organizer is also attending this event, would you "
                                                  "like to change the corresponding attendee as well?"));
    } else {
        answer = KMessageBox::Yes;
    }

    if (answer == KMessageBox::Yes) {
        if (currentOrganizerAttendee > -1) {
            mDataModel->removeRows(currentOrganizerAttendee, 1);
        }

        if (newOrganizerAttendee == -1) {
            bool rsvp = !iAmOrganizer(); // if it is the user, don't make him rsvp.
            KCalendarCore::Attendee::PartStat status = iAmOrganizer() ? KCalendarCore::Attendee::Accepted : KCalendarCore::Attendee::NeedsAction;

            KCalendarCore::Attendee newAt(name, email, rsvp, status, KCalendarCore::Attendee::ReqParticipant);

            mDataModel->insertAttendee(mDataModel->rowCount(), newAt);
        }
    }
    mOrganizer = newOrganizer;
}

AttendeeTableModel *IncidenceAttendee::dataModel() const
{
    return mDataModel;
}

AttendeeComboBoxDelegate *IncidenceAttendee::responseDelegate() const
{
    return mResponseDelegate;
}

AttendeeComboBoxDelegate *IncidenceAttendee::roleDelegate() const
{
    return mRoleDelegate;
}

AttendeeComboBoxDelegate *IncidenceAttendee::stateDelegate() const
{
    return mStateDelegate;
}

AttendeeLineEditDelegate *IncidenceAttendee::attendeeDelegate() const
{
    return mAttendeeDelegate;
}

void IncidenceAttendee::filterLayoutChanged()
{
    QHeaderView *headerView = mUi->mAttendeeTable->horizontalHeader();
    headerView->setSectionResizeMode(AttendeeTableModel::Role, QHeaderView::ResizeToContents);
    headerView->setSectionResizeMode(AttendeeTableModel::FullName, QHeaderView::Stretch);
    headerView->setSectionResizeMode(AttendeeTableModel::Status, QHeaderView::ResizeToContents);
    headerView->setSectionResizeMode(AttendeeTableModel::Response, QHeaderView::ResizeToContents);
    headerView->setSectionHidden(AttendeeTableModel::CuType, true);
    headerView->setSectionHidden(AttendeeTableModel::Name, true);
    headerView->setSectionHidden(AttendeeTableModel::Email, true);
    headerView->setSectionHidden(AttendeeTableModel::Available, true);
}

void IncidenceAttendee::updateCount()
{
    Q_EMIT attendeeCountChanged(attendeeCount());

    checkDirtyStatus();
}

int IncidenceAttendee::attendeeCount() const
{
    int c = 0;
    QModelIndex index;
    QAbstractItemModel *model = mUi->mAttendeeTable->model();
    if (!model) {
        return 0;
    }
    for (int i = 0; i < model->rowCount(QModelIndex()); ++i) {
        index = model->index(i, AttendeeTableModel::FullName);
        if (!model->data(index).toString().isEmpty()) {
            ++c;
        }
    }
    return c;
}

void IncidenceAttendee::setActions(KCalendarCore::Incidence::IncidenceType actions)
{
    mStateDelegate->clear();
    if (actions == KCalendarCore::Incidence::TypeEvent) {
        mStateDelegate->addItem(QIcon::fromTheme(QStringLiteral(":/task-attention.png")), KCalUtils::Stringify::attendeeStatus(AttendeeData::NeedsAction));
        mStateDelegate->addItem(QIcon::fromTheme(QStringLiteral(":/task-accepted.png")), KCalUtils::Stringify::attendeeStatus(AttendeeData::Accepted));
        mStateDelegate->addItem(QIcon::fromTheme(QStringLiteral(":/task-reject.png")), KCalUtils::Stringify::attendeeStatus(AttendeeData::Declined));
        mStateDelegate->addItem(QIcon::fromTheme(QStringLiteral(":/task-attempt.png")), KCalUtils::Stringify::attendeeStatus(AttendeeData::Tentative));
        mStateDelegate->addItem(QIcon::fromTheme(QStringLiteral(":/task-delegate.png")), KCalUtils::Stringify::attendeeStatus(AttendeeData::Delegated));
    } else {
        mStateDelegate->addItem(QIcon::fromTheme(QStringLiteral(":/task-attention.png")), KCalUtils::Stringify::attendeeStatus(AttendeeData::NeedsAction));
        mStateDelegate->addItem(QIcon::fromTheme(QStringLiteral(":/task-accepted.png")), KCalUtils::Stringify::attendeeStatus(AttendeeData::Accepted));
        mStateDelegate->addItem(QIcon::fromTheme(QStringLiteral(":/task-reject.png")), KCalUtils::Stringify::attendeeStatus(AttendeeData::Declined));
        mStateDelegate->addItem(QIcon::fromTheme(QStringLiteral(":/task-attempt.png")), KCalUtils::Stringify::attendeeStatus(AttendeeData::Tentative));
        mStateDelegate->addItem(QIcon::fromTheme(QStringLiteral(":/task-delegate.png")), KCalUtils::Stringify::attendeeStatus(AttendeeData::Delegated));
        mStateDelegate->addItem(QIcon::fromTheme(QStringLiteral(":/task-complete.png")), KCalUtils::Stringify::attendeeStatus(AttendeeData::Completed));
        mStateDelegate->addItem(QIcon::fromTheme(QStringLiteral(":/task-ongoing.png")), KCalUtils::Stringify::attendeeStatus(AttendeeData::InProcess));
    }
}

void IncidenceAttendee::printDebugInfo() const
{
    qCDebug(INCIDENCEEDITOR_LOG) << "I'm organizer   : " << iAmOrganizer();
    qCDebug(INCIDENCEEDITOR_LOG) << "Loaded organizer: " << mLoadedIncidence->organizer().email();

    if (iAmOrganizer()) {
        KCalendarCore::Event tmp;
        tmp.setOrganizer(mUi->mOrganizerCombo->currentText());
        qCDebug(INCIDENCEEDITOR_LOG) << "Organizer combo: " << tmp.organizer().email();
    }

    const KCalendarCore::Attendee::List originalList = mLoadedIncidence->attendees();
    KCalendarCore::Attendee::List newList;
    qCDebug(INCIDENCEEDITOR_LOG) << "List sizes: " << originalList.count() << newList.count();

    const auto lstAttendees = mDataModel->attendees();
    for (const KCalendarCore::Attendee &attendee : lstAttendees) {
        if (!attendee.fullName().isEmpty()) {
            newList.append(attendee);
        }
    }

    // Okay, again not the most efficient algorithm, but I'm assuming that in the
    // bulk of the use cases, the number of attendees is not much higher than 10 or so.
    for (const KCalendarCore::Attendee &attendee : originalList) {
        bool found = false;
        for (int i = 0; i < newList.count(); ++i) {
            if (newList[i] == attendee) {
                newList.remove(i);
                found = true;
                break;
            }
        }

        if (!found) {
            qCDebug(INCIDENCEEDITOR_LOG) << "Attendee not found: " << attendee.email() << attendee.name() << attendee.status() << attendee.RSVP()
                                         << attendee.role() << attendee.uid() << attendee.cuType() << attendee.delegate() << attendee.delegator()
                                         << "; we have:";
            for (int i = 0, total = newList.count(); i < total; ++i) {
                const KCalendarCore::Attendee newAttendee = newList[i];
                qCDebug(INCIDENCEEDITOR_LOG) << "Attendee: " << newAttendee.email() << newAttendee.name() << newAttendee.status() << newAttendee.RSVP()
                                             << newAttendee.role() << newAttendee.uid() << newAttendee.cuType() << newAttendee.delegate()
                                             << newAttendee.delegator();
            }

            return;
        }
    }
}

int IncidenceAttendee::rowOfAttendee(const QString &uid) const
{
    const auto attendees = dataModel()->attendees();
    const auto it = std::find_if(attendees.begin(), attendees.end(), [uid](const KCalendarCore::Attendee &att) {
        return att.uid() == uid;
    });
    return std::distance(attendees.begin(), it);
}
