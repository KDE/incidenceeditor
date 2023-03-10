/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "incidenceeditor-ng.h"

#include <memory>

namespace Ui
{
class EventOrTodoDesktop;
}

namespace IncidenceEditorNG
{
class IncidenceDescriptionPrivate;

/**
 * The IncidenceDescriptionEditor keeps track of the following Incidence parts:
 * - description
 */
class IncidenceDescription : public IncidenceEditor
{
    Q_OBJECT
public:
    using IncidenceEditorNG::IncidenceEditor::load; // So we don't trigger -Woverloaded-virtual
    using IncidenceEditorNG::IncidenceEditor::save; // So we don't trigger -Woverloaded-virtual

    explicit IncidenceDescription(Ui::EventOrTodoDesktop *ui);

    ~IncidenceDescription() override;

    void load(const KCalendarCore::Incidence::Ptr &incidence) override;
    void save(const KCalendarCore::Incidence::Ptr &incidence) override;
    Q_REQUIRED_RESULT bool isDirty() const override;

    void printDebugInfo() const override;

private:
    void toggleRichTextDescription();
    void enableRichTextDescription(bool enable);
    void setupToolBar();

private:
    Ui::EventOrTodoDesktop *const mUi;
    //@cond PRIVATE
    Q_DECLARE_PRIVATE(IncidenceDescription)
    std::unique_ptr<IncidenceDescriptionPrivate> const d;
    //@endcond
};
}
