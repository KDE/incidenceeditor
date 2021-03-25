/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "incidenceeditor-ng.h"

namespace Ui
{
class EventOrTodoDesktop;
}

namespace IncidenceEditorNG
{
class IncidenceSecrecy : public IncidenceEditor
{
    Q_OBJECT
public:
    using IncidenceEditorNG::IncidenceEditor::load; // So we don't trigger -Woverloaded-virtual
    using IncidenceEditorNG::IncidenceEditor::save; // So we don't trigger -Woverloaded-virtual

    explicit IncidenceSecrecy(Ui::EventOrTodoDesktop *ui);

    void load(const KCalendarCore::Incidence::Ptr &incidence) override;
    void save(const KCalendarCore::Incidence::Ptr &incidence) override;
    Q_REQUIRED_RESULT bool isDirty() const override;

private:
    Ui::EventOrTodoDesktop *const mUi;
};
}

