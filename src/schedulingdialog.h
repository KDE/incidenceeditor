/*
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "ui_schedulingdialog.h"

#include <QDateTime>
#include <QDialog>

namespace CalendarSupport
{
class FreePeriodModel;
}

namespace IncidenceEditorNG
{
class ConflictResolver;
class VisualFreeBusyWidget;

class SchedulingDialog : public QDialog, private Ui_Dialog
{
    Q_OBJECT
public:
    explicit SchedulingDialog(const QDate &startDate, const QTime &startTime, int duration, ConflictResolver *resolver, QWidget *parent);
    ~SchedulingDialog() override;

    Q_REQUIRED_RESULT QDate selectedStartDate() const;
    Q_REQUIRED_RESULT QTime selectedStartTime() const;

public Q_SLOTS:
    void slotUpdateIncidenceStartEnd(const QDateTime &startDateTime, const QDateTime &endDateTime);

Q_SIGNALS:
    void startDateChanged(const QDate &newDate);
    void startTimeChanged(const QTime &newTime);
    void endDateChanged(const QDate &newDate);
    void endTimeChanged(const QTime &newTime);

private:
    void slotWeekdaysChanged();
    void slotMandatoryRolesChanged();
    void slotStartDateChanged(const QDate &newDate);

    void slotRowSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
    void slotSetEndTimeLabel(const QTime &startTime);
    void updateWeekDays(const QDate &oldDate);
    void fillCombos();

    QDate mStDate;
    QDate mSelectedDate;
    QTime mSelectedTime;
    int mDuration; //!< In seconds

    ConflictResolver *mResolver = nullptr;
    CalendarSupport::FreePeriodModel *const mPeriodModel;
    VisualFreeBusyWidget *mVisualWidget = nullptr;
};
}

