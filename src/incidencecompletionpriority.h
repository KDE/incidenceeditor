/*
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
  SPDX-FileContributor: Kevin Krammer <krake@kdab.com>

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
class IncidenceCompletionPriority : public IncidenceEditor
{
    Q_OBJECT
public:
    using IncidenceEditorNG::IncidenceEditor::load; // So we don't trigger -Woverloaded-virtual
    using IncidenceEditorNG::IncidenceEditor::save; // So we don't trigger -Woverloaded-virtual

    explicit IncidenceCompletionPriority(Ui::EventOrTodoDesktop *ui);
    ~IncidenceCompletionPriority() override;

    void load(const KCalendarCore::Incidence::Ptr &incidence) override;
    void save(const KCalendarCore::Incidence::Ptr &incidence) override;
    bool isDirty() const override;

private:
    class Private;
    Private *const d;
};
}

