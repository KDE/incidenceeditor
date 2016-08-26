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
#include <Libkdepim/TagWidgets>
#include "incidenceeditor_debug.h"

#include <TagAttribute>
#include <TagCreateJob>
#include <TagFetchJob>
#include <TagFetchScope>

using namespace IncidenceEditorNG;

IncidenceCategories::IncidenceCategories(Ui::EventOrTodoDesktop *ui)
    : mUi(ui),
      mDirty(false)
{
    setObjectName(QStringLiteral("IncidenceCategories"));

    connect(mUi->mTagWidget, &Akonadi::TagWidget::selectionChanged, this, &IncidenceCategories::onSelectionChanged);
}

void IncidenceCategories::onSelectionChanged(const Akonadi::Tag::List &list)
{
    mSelectedTags = list;
    mDirty = true;
    checkDirtyStatus();
}

void IncidenceCategories::load(const KCalCore::Incidence::Ptr &incidence)
{
    mLoadedIncidence = incidence;
    mDirty = false;
    mWasDirty = false;
    mSelectedTags.clear();
    mMissingCategories = incidence->categories();

    if (mLoadedIncidence) {
        Akonadi::TagFetchJob *fetchJob = new Akonadi::TagFetchJob(this);
        fetchJob->fetchScope().fetchAttribute<Akonadi::TagAttribute>();
        connect(fetchJob, &Akonadi::TagFetchJob::result, this, &IncidenceCategories::onTagsFetched);
    }
}

void IncidenceCategories::load(const Akonadi::Item &item)
{
    mSelectedTags = item.tags();

    mUi->mTagWidget->setSelection(item.tags());
}

void IncidenceCategories::save(const KCalCore::Incidence::Ptr &incidence)
{
    Q_ASSERT(incidence);
    incidence->setCategories(categories());
}

void IncidenceCategories::save(Akonadi::Item &item)
{
    if (item.tags() != mSelectedTags) {
        item.setTags(mSelectedTags);
    }
}

QStringList IncidenceCategories::categories() const
{
    QStringList list;
    list.reserve(mSelectedTags.count() + mMissingCategories.count());
    Q_FOREACH (const Akonadi::Tag &tag, mSelectedTags) {
        list << tag.name();
    }
    list << mMissingCategories;
    return list;
}

void IncidenceCategories::createMissingCategories()
{
    Q_FOREACH (const QString &category, mMissingCategories) {
        Akonadi::Tag missingTag = Akonadi::Tag::genericTag(category);
        Akonadi::TagCreateJob *createJob = new Akonadi::TagCreateJob(missingTag, this);
        connect(createJob, &Akonadi::TagCreateJob::result, this, &IncidenceCategories::onMissingTagCreated);
    }
}

bool IncidenceCategories::isDirty() const
{
    return mDirty;
}

void IncidenceCategories::printDebugInfo() const
{
    qCDebug(INCIDENCEEDITOR_LOG) << "mSelectedCategories = " << categories();
    qCDebug(INCIDENCEEDITOR_LOG) << "mLoadedIncidence->categories() = " << mLoadedIncidence->categories();
}

void IncidenceCategories::matchExistingCategories(const QStringList &categories,
                                                  const Akonadi::Tag::List &existingTags)
{
    for (const QString &category : categories) {
        auto matchedTagIter = std::find_if(existingTags.cbegin(), existingTags.cend(),
            [&category](const Akonadi::Tag &tag) {
                return tag.name() == category;
            });
        Q_ASSERT(matchedTagIter != existingTags.cend());
        if (!mSelectedTags.contains(*matchedTagIter)) {
            mSelectedTags << *matchedTagIter;
        }
    }
}

void IncidenceCategories::onTagsFetched(KJob *job)
{
    if (job->error()) {
        qCWarning(INCIDENCEEDITOR_LOG) << "Failed to load tags " << job->errorString();
        return;
    }
    Akonadi::TagFetchJob *fetchJob = static_cast<Akonadi::TagFetchJob *>(job);
    const Akonadi::Tag::List jobTags = fetchJob->tags();
    QStringList matchedCategories;
    for (const Akonadi::Tag &tag : jobTags) {
        if (mMissingCategories.removeAll(tag.name()) > 0) {
            matchedCategories << tag.name();
        }
    }
    matchExistingCategories(matchedCategories, jobTags);
    createMissingCategories();
    mUi->mTagWidget->setSelection(mSelectedTags);
}

void IncidenceCategories::onMissingTagCreated(KJob *job)
{
    if (job->error()) {
        qCWarning(INCIDENCEEDITOR_LOG) << "Failed to create tag " << job->errorString();
        return;
    }
    Akonadi::TagCreateJob *createJob = static_cast<Akonadi::TagCreateJob *>(job);
    mMissingCategories.removeAll(createJob->tag().name());
    mSelectedTags << createJob->tag();
    mUi->mTagWidget->setSelection(mSelectedTags);
}
