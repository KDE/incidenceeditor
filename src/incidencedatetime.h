/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "incidenceeditor-ng.h"

#include <KCalendarCore/Event>
#include <KCalendarCore/Journal>
#include <KCalendarCore/Todo>

#include <QDate>

namespace Ui
{
class EventOrTodoDesktop;
}

namespace IncidenceEditorNG
{
class IncidenceDateTime : public IncidenceEditor
{
    Q_OBJECT
public:
    using IncidenceEditorNG::IncidenceEditor::load; // So we don't trigger -Woverloaded-virtual
    using IncidenceEditorNG::IncidenceEditor::save; // So we don't trigger -Woverloaded-virtual
    explicit IncidenceDateTime(Ui::EventOrTodoDesktop *ui);
    ~IncidenceDateTime() override;

    void load(const KCalendarCore::Incidence::Ptr &incidence) override;
    void save(const KCalendarCore::Incidence::Ptr &incidence) override;
    Q_REQUIRED_RESULT bool isDirty() const override;

    /**
     * Sets the active date for the editing session. This defaults to the current
     * date. It should be set <em>before</em> loading a non-empty (i.e. existing
     * incidence).
     */
    void setActiveDate(const QDate &activeDate);

    Q_REQUIRED_RESULT QDate startDate() const; /// Returns the current start date.
    Q_REQUIRED_RESULT QTime startTime() const; /// Returns the current start time.
    Q_REQUIRED_RESULT QDate endDate() const; /// Returns the current end date.
    Q_REQUIRED_RESULT QTime endTime() const; /// Returns the current endtime.

    /// Created from the values in the widgets
    Q_REQUIRED_RESULT QDateTime currentStartDateTime() const;
    Q_REQUIRED_RESULT QDateTime currentEndDateTime() const;

    void setStartTime(const QTime &newTime);
    void setStartDate(const QDate &newDate);

    Q_REQUIRED_RESULT bool startDateTimeEnabled() const;
    Q_REQUIRED_RESULT bool endDateTimeEnabled() const;

    void focusInvalidField() override;

    Q_REQUIRED_RESULT bool isValid() const override;
    void printDebugInfo() const override;

Q_SIGNALS:
    // used to indicate that the widgets were activated
    void startDateFocus(QObject *obj);
    void endDateFocus(QObject *obj);
    void startTimeFocus(QObject *obj);
    void endTimeFocus(QObject *obj);

    // general
    void startDateTimeToggled(bool enabled);
    void startDateChanged(const QDate &newDate);
    void startTimeChanged(const QTime &newTime);
    void endDateTimeToggled(bool enabled);
    void endDateChanged(const QDate &newDate);
    void endTimeChanged(const QTime &newTime);

private Q_SLOTS: /// General
    void setTimeZonesVisibility(bool visible);
    void toggleTimeZoneVisibility();
    void updateStartTime(const QTime &newTime);
    void updateStartDate(const QDate &newDate);
    void updateStartSpec();
    void updateStartToolTips();
    void updateEndToolTips();

private Q_SLOTS: /// Todo specific
    void enableStartEdit(bool enable);
    void enableEndEdit(bool enable);
    void enableTimeEdits();

private:
    bool isDirty(const KCalendarCore::Todo::Ptr &todo) const;
    bool isDirty(const KCalendarCore::Event::Ptr &event) const;
    bool isDirty(const KCalendarCore::Journal::Ptr &journal) const;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void load(const KCalendarCore::Event::Ptr &event, bool isTemplate = false, bool templateOverridesTimes = false);
    void load(const KCalendarCore::Todo::Ptr &todo, bool isTemplate = false, bool templateOverridesTimes = false);
    void load(const KCalendarCore::Journal::Ptr &journal, bool isTemplate = false, bool templateOverridesTimes = false);
    void save(const KCalendarCore::Event::Ptr &event);
    void save(const KCalendarCore::Todo::Ptr &todo);
    void save(const KCalendarCore::Journal::Ptr &journal);
    void setDateTimes(const QDateTime &start, const QDateTime &end);
    void setTimes(const QDateTime &start, const QDateTime &end);
    void setTimeZoneLabelEnabled(bool enable);
    bool timeZonesAreLocal(const QDateTime &start, const QDateTime &end);

private:
    Ui::EventOrTodoDesktop *mUi = nullptr;

    QDate mActiveDate;
    /**
     * These might differ from mLoadedIncidence->(dtStart|dtDue) as these take
     * in account recurrence if needed. The values are calculated once on load().
     * and don't change afterwards.
     */
    QDateTime mInitialStartDT;
    QDateTime mInitialEndDT;

    /**
     * We need to store the current start date/time to be able to update the end
     * time appropriate when the start time changes.
     */
    QDateTime mCurrentStartDateTime;

    /// Remembers state when switching between takes whole day and timed event/to-do.
    bool mTimezoneCombosWereVisibile;
};
}

