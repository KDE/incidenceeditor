/*
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "schedulingdialog.h"
#include "conflictresolver.h"
#include "visualfreebusywidget.h"
#include <CalendarSupport/FreePeriodModel>
#include <KCalUtils/Stringify>

#include <KLocalizedString>

#include <QDialogButtonBox>
#include <QLocale>
#include <QPushButton>
#include <QVBoxLayout>

using namespace IncidenceEditorNG;

SchedulingDialog::SchedulingDialog(const QDate &startDate, const QTime &startTime, int duration, ConflictResolver *resolver, QWidget *parent)
    : QDialog(parent)
    , mResolver(resolver)
    , mPeriodModel(new CalendarSupport::FreePeriodModel(this))
{
    setWindowTitle(i18nc("@title:window", "Scheduling"));
    auto mainLayout = new QVBoxLayout(this);
    auto w = new QWidget(this);
    setupUi(w);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SchedulingDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SchedulingDialog::reject);

    mainLayout->addWidget(w);
    mainLayout->addWidget(buttonBox);
    fillCombos();

    Q_ASSERT(duration > 0);
    mDuration = duration;

    mVisualWidget = new VisualFreeBusyWidget(resolver->model(), 8);
    auto ganttlayout = new QVBoxLayout(mGanttTab);

    mGanttTab->setLayout(ganttlayout);
    ganttlayout->addWidget(mVisualWidget);

    connect(mStartDate, &KDateComboBox::dateEdited, mResolver, &ConflictResolver::setEarliestDate);
    connect(mStartTime, &KTimeComboBox::timeEdited, mResolver, &ConflictResolver::setEarliestTime);
    connect(mEndDate, &KDateComboBox::dateEdited, mResolver, &ConflictResolver::setLatestDate);
    connect(mEndTime, &KTimeComboBox::timeEdited, mResolver, &ConflictResolver::setLatestTime);

    connect(mStartDate, &KDateComboBox::dateEdited, this, &SchedulingDialog::slotStartDateChanged);

    connect(mWeekdayCombo, &IncidenceEditorNG::KWeekdayCheckCombo::checkedItemsChanged, this, &SchedulingDialog::slotWeekdaysChanged);
    connect(mWeekdayCombo, &IncidenceEditorNG::KWeekdayCheckCombo::checkedItemsChanged, this, &SchedulingDialog::slotMandatoryRolesChanged);

    connect(mResolver, &ConflictResolver::freeSlotsAvailable, mPeriodModel, &CalendarSupport::FreePeriodModel::slotNewFreePeriods);
    connect(mMoveBeginTimeEdit, &KTimeComboBox::timeEdited, this, &SchedulingDialog::slotSetEndTimeLabel);

    mTableView->setModel(mPeriodModel);
    connect(mTableView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &SchedulingDialog::slotRowSelectionChanged);

    mStartDate->setDate(startDate);
    mEndDate->setDate(mStartDate->date().addDays(7));
    mStartTime->setTime(startTime);
    mEndTime->setTime(startTime);

    mResolver->setEarliestDate(mStartDate->date());
    mResolver->setEarliestTime(mStartTime->time());
    mResolver->setLatestDate(mEndDate->date());
    mResolver->setLatestTime(mEndTime->time());

    mMoveApptGroupBox->hide();
}

SchedulingDialog::~SchedulingDialog()
{
}

void SchedulingDialog::slotUpdateIncidenceStartEnd(const QDateTime &startDateTime, const QDateTime &endDateTime)
{
    mVisualWidget->slotUpdateIncidenceStartEnd(startDateTime, endDateTime);
}

void SchedulingDialog::fillCombos()
{
    // Note: we depend on the following order
    mRolesCombo->addItem(QIcon::fromTheme(QStringLiteral("meeting-participant")), KCalUtils::Stringify::attendeeRole(KCalendarCore::Attendee::ReqParticipant));
    mRolesCombo->addItem(QIcon::fromTheme(QStringLiteral("meeting-participant-optional")),
                         KCalUtils::Stringify::attendeeRole(KCalendarCore::Attendee::OptParticipant));
    mRolesCombo->addItem(QIcon::fromTheme(QStringLiteral("meeting-observer")), KCalUtils::Stringify::attendeeRole(KCalendarCore::Attendee::NonParticipant));
    mRolesCombo->addItem(QIcon::fromTheme(QStringLiteral("meeting-chair")), KCalUtils::Stringify::attendeeRole(KCalendarCore::Attendee::Chair));

    mRolesCombo->setWhatsThis(i18nc("@info:whatsthis", "Edits the role of the attendee."));

    mRolesCombo->setItemCheckState(0, Qt::Checked);
    mRolesCombo->setItemCheckState(1, Qt::Checked);
    mRolesCombo->setItemCheckState(2, Qt::Checked);
    mRolesCombo->setItemCheckState(3, Qt::Checked);

    QBitArray days(7);
    days.setBit(0); // Monday
    days.setBit(1); // Tuesday
    days.setBit(2); // Wednesday
    days.setBit(3); // Thursday
    days.setBit(4); // Friday.. surprise!

    mWeekdayCombo->setDays(days);
    mResolver->setAllowedWeekdays(days);
}

void SchedulingDialog::slotStartDateChanged(const QDate &newDate)
{
    QDate oldDate = mStDate;
    mStDate = newDate;
    if (newDate.isValid() && oldDate.isValid()) {
        updateWeekDays(oldDate);
    }
}

void SchedulingDialog::updateWeekDays(const QDate &oldDate)
{
    const int oldStartDayIndex = mWeekdayCombo->weekdayIndex(oldDate);
    const int newStartDayIndex = mWeekdayCombo->weekdayIndex(mStDate);

    mWeekdayCombo->setItemCheckState(oldStartDayIndex, Qt::Unchecked);
    mWeekdayCombo->setItemEnabled(oldStartDayIndex, true);
    mWeekdayCombo->setItemCheckState(newStartDayIndex, Qt::Checked);
    mWeekdayCombo->setItemEnabled(newStartDayIndex, false);
}

void SchedulingDialog::slotWeekdaysChanged()
{
    // notify the resolver
    mResolver->setAllowedWeekdays(mWeekdayCombo->days());
}

void SchedulingDialog::slotMandatoryRolesChanged()
{
    QSet<KCalendarCore::Attendee::Role> roles;
    for (int i = 0; i < mRolesCombo->count(); ++i) {
        if (mRolesCombo->itemCheckState(i) == Qt::Checked) {
            roles << KCalendarCore::Attendee::Role(i);
        }
    }
    mResolver->setMandatoryRoles(roles);
}

void SchedulingDialog::slotRowSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)
    if (!current.isValid()) {
        mMoveApptGroupBox->hide();
        return;
    }
    auto period = current.data(CalendarSupport::FreePeriodModel::PeriodRole).value<KCalendarCore::Period>();
    const QDate startDate = period.start().date();

    const int dayOfWeek = startDate.dayOfWeek();
    const QString dayLabel = ki18nc(
                                 "@label Day of week followed by day of the month, then the month. "
                                 "Example: Monday, 12 June",
                                 "%1, %2 %3")
                                 .subs(QLocale::system().dayName(dayOfWeek, QLocale::LongFormat))
                                 .subs(startDate.day())
                                 .subs(QLocale::system().monthName(startDate.month(), QLocale::LongFormat))
                                 .toString();

    mMoveDayLabel->setText(dayLabel);
    mMoveBeginTimeEdit->setTimeRange(period.start().time(), period.end().addSecs(-mDuration).time());
    mMoveBeginTimeEdit->setTime(period.start().time());
    slotSetEndTimeLabel(period.start().time());
    mMoveApptGroupBox->show();

    mSelectedDate = startDate;
}

void SchedulingDialog::slotSetEndTimeLabel(const QTime &startTime)
{
    const QTime endTime = startTime.addSecs(mDuration);
    const QString endTimeLabel = ki18nc(
                                     "@label This is a suffix following a time selecting widget. "
                                     "Example: [timeedit] to 10:00am",
                                     "to %1")
                                     .subs(QLocale::system().toString(endTime))
                                     .toString();

    mMoveEndTimeLabel->setText(endTimeLabel);
    mSelectedTime = startTime;
}

QDate SchedulingDialog::selectedStartDate() const
{
    return mSelectedDate;
}

QTime SchedulingDialog::selectedStartTime() const
{
    return mSelectedTime;
}
