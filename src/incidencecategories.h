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

    void load(const KCalCore::Incidence::Ptr &incidence) Q_DECL_OVERRIDE;
    void load(const Akonadi::Item &item) Q_DECL_OVERRIDE;
    void save(const KCalCore::Incidence::Ptr &incidence) Q_DECL_OVERRIDE;
    void save(Akonadi::Item &item) Q_DECL_OVERRIDE;

    /**
     * Returns the list of currently selected categories.
     */
    QStringList categories() const;

    bool isDirty() const Q_DECL_OVERRIDE;
    void printDebugInfo() const Q_DECL_OVERRIDE;

private:
    void matchExistingCategories(const QStringList &categories, const Akonadi::Tag::List &existingTags);
    void createMissingCategories();

private Q_SLOTS:
    void onSelectionChanged(const Akonadi::Tag::List &);
    void onTagsFetched(KJob *);
    void onMissingTagCreated(KJob *);

private:
    Ui::EventOrTodoDesktop *mUi;
    Akonadi::Tag::List mSelectedTags;

    /**
     * List of categories for which no tag might exist.
     *
     * For each category of the editted incidence, we want to  make sure that there exists a
     * corresponding tag in Akonadi. For missing categories, a \a TagCreateJob is issued.
     * Eventually, there should be no missing categories left. In case tag creation fails for some
     * categories, this list still holds these categories so they don't get lost
     */
    QStringList mMissingCategories;
    bool mDirty;
};

}

#endif
