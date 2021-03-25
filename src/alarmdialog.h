/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KCalendarCore/Incidence>

#include <QDialog>

namespace Ui
{
class AlarmDialog;
}

namespace IncidenceEditorNG
{
class AlarmDialog : public QDialog
{
    Q_OBJECT
public:
    enum Unit { Minutes, Hours, Days };

    enum When { BeforeStart = 0, AfterStart, BeforeEnd, AfterEnd };

public:
    /**
      Constructs a new alarm dialog.
      @p incidenceType will influence i18n strings, that will be different for to-dos.
     */
    explicit AlarmDialog(KCalendarCore::Incidence::IncidenceType incidenceType, QWidget *parent = nullptr);
    ~AlarmDialog() override;
    void load(const KCalendarCore::Alarm::Ptr &alarm);
    void save(const KCalendarCore::Alarm::Ptr &alarm) const;
    void setAllowBeginReminders(bool allow);
    void setAllowEndReminders(bool allow);
    void setOffset(int offset);
    void setUnit(Unit unit);
    void setWhen(When when);

private:
    void fillCombo();

private:
    Ui::AlarmDialog *const mUi;
    const KCalendarCore::Incidence::IncidenceType mIncidenceType;
    bool mAllowBeginReminders = true;
    bool mAllowEndReminders = true;
};
}

