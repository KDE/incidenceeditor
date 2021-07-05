/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

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
class IncidenceCategories : public IncidenceEditor
{
    Q_OBJECT
public:
    explicit IncidenceCategories(Ui::EventOrTodoDesktop *ui);

    void load(const KCalendarCore::Incidence::Ptr &incidence) override;
    void load(const Akonadi::Item &item) override;
    void save(const KCalendarCore::Incidence::Ptr &incidence) override;
    void save(Akonadi::Item &item) override;

    /**
     * Returns the list of currently selected categories.
     */
    Q_REQUIRED_RESULT QStringList categories() const;

    Q_REQUIRED_RESULT bool isDirty() const override;
    void printDebugInfo() const override;

private:
    void createMissingCategories();

    void onSelectionChanged(const Akonadi::Tag::List &);
    void onMissingTagCreated(KJob *);

    Ui::EventOrTodoDesktop *mUi = nullptr;

    /**
     * List of categories for which no tag might exist.
     *
     * For each category of the edited incidence, we want to  make sure that there exists a
     * corresponding tag in Akonadi. For missing categories, a \a TagCreateJob is issued.
     * Eventually, there should be no missing categories left. In case tag creation fails for some
     * categories, this list still holds these categories so they don't get lost
     */
    QStringList mMissingCategories;
    bool mDirty = false;
};
}

