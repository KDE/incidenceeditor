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
/**
 * The IncidenceWhatWhere editor keeps track of the following Incidence parts:
 * - Summary
 * - Location
 */
class IncidenceWhatWhere : public IncidenceEditor
{
    Q_OBJECT
public:
    using IncidenceEditorNG::IncidenceEditor::load; // So we don't trigger -Woverloaded-virtual
    using IncidenceEditorNG::IncidenceEditor::save; // So we don't trigger -Woverloaded-virtual

    explicit IncidenceWhatWhere(Ui::EventOrTodoDesktop *ui);

    void load(const KCalendarCore::Incidence::Ptr &incidence) override;
    void save(const KCalendarCore::Incidence::Ptr &incidence) override;
    [[nodiscard]] bool isDirty() const override;
    [[nodiscard]] bool isValid() const override;
    void focusInvalidField() override;
    virtual void validate();

private:
    Ui::EventOrTodoDesktop *const mUi;
};
} // IncidenceEditorNG
