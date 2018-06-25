/*
  Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#ifndef INCIDENCEEDITOR_INCIDENCECATEGORIES_H
#define INCIDENCEEDITOR_INCIDENCECATEGORIES_H

#include "incidenceeditor-ng.h"

namespace Ui {
class EventOrTodoDesktop;
}

namespace IncidenceEditorNG {
class IncidenceCategories : public IncidenceEditor
{
    Q_OBJECT
public:
    explicit IncidenceCategories(Ui::EventOrTodoDesktop *ui);

    void load(const KCalCore::Incidence::Ptr &incidence) override;
    void save(const KCalCore::Incidence::Ptr &incidence) override;
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
    void onTagsFetched(KJob *);
    void onMissingTagCreated(KJob *);

    Ui::EventOrTodoDesktop *mUi = nullptr;

    /**
     * List of categories for which no tag might exist.
     *
     * For each category of the editted incidence, we want to  make sure that there exists a
     * corresponding tag in Akonadi. For missing categories, a \a TagCreateJob is issued.
     * Eventually, there should be no missing categories left. In case tag creation fails for some
     * categories, this list still holds these categories so they don't get lost
     */
    QStringList mMissingCategories;
    bool mDirty = false;
};
}

#endif
