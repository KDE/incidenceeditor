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

#include "incidencecategories.h"

#include "editorconfig.h"

#include "ui_dialogdesktop.h"
#include <LibkdepimAkonadi/TagWidgets>
#include "incidenceeditor_debug.h"

#include <TagAttribute>
#include <TagCreateJob>
#include <TagFetchJob>
#include <TagFetchScope>

using namespace IncidenceEditorNG;

IncidenceCategories::IncidenceCategories(Ui::EventOrTodoDesktop *ui)
    : mUi(ui)
    , mDirty(false)
{
    setObjectName(QStringLiteral("IncidenceCategories"));

    connect(mUi->mTagWidget, &Akonadi::TagWidget::selectionChanged, this,
            &IncidenceCategories::onSelectionChanged);
}

void IncidenceCategories::onSelectionChanged(const Akonadi::Tag::List &list)
{
    Q_UNUSED(list);
    mDirty = true;
    checkDirtyStatus();
}

void IncidenceCategories::load(const KCalCore::Incidence::Ptr &incidence)
{
    mLoadedIncidence = incidence;
    mDirty = false;
    mWasDirty = false;
    mMissingCategories.clear();

    if (mLoadedIncidence) {
        Akonadi::TagFetchJob *fetchJob = new Akonadi::TagFetchJob(this);
        fetchJob->fetchScope().fetchAttribute<Akonadi::TagAttribute>();
        connect(fetchJob, &Akonadi::TagFetchJob::result, this, &IncidenceCategories::onTagsFetched);
    }
}

void IncidenceCategories::save(const KCalCore::Incidence::Ptr &incidence)
{
    Q_ASSERT(incidence);
    if (mDirty) {
        incidence->setCategories(categories());
    }
}

void IncidenceCategories::save(Akonadi::Item &item)
{
    const auto &selectedTags = mUi->mTagWidget->selection();
    if (mDirty) {
        item.setTags(selectedTags);
    }
}

QStringList IncidenceCategories::categories() const
{
    QStringList list;
    const auto &selectedTags = mUi->mTagWidget->selection();
    list.reserve(selectedTags.count() + mMissingCategories.count());
    for (const Akonadi::Tag &tag : selectedTags) {
        list << tag.name();
    }
    list << mMissingCategories;
    return list;
}

void IncidenceCategories::createMissingCategories()
{
    for (const QString &category : qAsConst(mMissingCategories)) {
        Akonadi::Tag missingTag = Akonadi::Tag::genericTag(category);
        Akonadi::TagCreateJob *createJob = new Akonadi::TagCreateJob(missingTag, this);
        connect(createJob, &Akonadi::TagCreateJob::result, this,
                &IncidenceCategories::onMissingTagCreated);
    }
}

bool IncidenceCategories::isDirty() const
{
    return mDirty;
}

void IncidenceCategories::printDebugInfo() const
{
    qCDebug(INCIDENCEEDITOR_LOG) << "selected categories = " << categories();
    qCDebug(INCIDENCEEDITOR_LOG) << "mMissingCategories = " << mMissingCategories;
    qCDebug(INCIDENCEEDITOR_LOG) << "mLoadedIncidence->categories() = "
                                 << mLoadedIncidence->categories();
}

void IncidenceCategories::onTagsFetched(KJob *job)
{
    if (job->error()) {
        qCWarning(INCIDENCEEDITOR_LOG) << "Failed to load tags " << job->errorString();
        return;
    }
    Akonadi::TagFetchJob *fetchJob = static_cast<Akonadi::TagFetchJob *>(job);
    const Akonadi::Tag::List jobTags = fetchJob->tags();

    Q_ASSERT(mLoadedIncidence);
    mMissingCategories = mLoadedIncidence->categories();

    Akonadi::Tag::List selectedTags;
    selectedTags.reserve(mMissingCategories.count());
    for (const auto &tag : jobTags) {
        if (mMissingCategories.removeAll(tag.name()) > 0) {
            selectedTags << tag;
        }
    }

    createMissingCategories();
    mUi->mTagWidget->setSelection(selectedTags);
}

void IncidenceCategories::onMissingTagCreated(KJob *job)
{
    if (job->error()) {
        qCWarning(INCIDENCEEDITOR_LOG) << "Failed to create tag " << job->errorString();
        return;
    }
    Akonadi::TagCreateJob *createJob = static_cast<Akonadi::TagCreateJob *>(job);
    int count = mMissingCategories.removeAll(createJob->tag().name());
    Q_ASSERT(count > 0);

    QVector<Akonadi::Tag> selectedTags;
    selectedTags.reserve(mUi->mTagWidget->selection().count() + 1);
    selectedTags << mUi->mTagWidget->selection() << createJob->tag();
    mUi->mTagWidget->setSelection(selectedTags);
}
