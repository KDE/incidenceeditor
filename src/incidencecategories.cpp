/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "incidencecategories.h"

#include "editorconfig.h"

#include "incidenceeditor_debug.h"
#include "ui_dialogdesktop.h"

#include <CalendarSupport/Utils>

#include <TagAttribute>
#include <TagCreateJob>
#include <TagFetchJob>
#include <TagFetchScope>

using namespace IncidenceEditorNG;

IncidenceCategories::IncidenceCategories(Ui::EventOrTodoDesktop *ui)
    : mUi(ui)
{
    setObjectName(QStringLiteral("IncidenceCategories"));

    connect(mUi->mTagWidget, &Akonadi::TagWidget::selectionChanged, this, &IncidenceCategories::onSelectionChanged);
}

void IncidenceCategories::onSelectionChanged(const Akonadi::Tag::List &list)
{
    Q_UNUSED(list)
    mDirty = true;
    checkDirtyStatus();
}

void IncidenceCategories::load(const KCalendarCore::Incidence::Ptr &incidence)
{
    Q_UNUSED(incidence)
    mDirty = false;
    mWasDirty = false;
}

void IncidenceCategories::load(const Akonadi::Item &item)
{
    mLoadedIncidence = CalendarSupport::incidence(item);
    mDirty = false;
    mWasDirty = false;

    Q_ASSERT(mLoadedIncidence);
    if (mLoadedIncidence) {
        mMissingCategories = mLoadedIncidence->categories();
        const auto tags = item.tags();
        Akonadi::Tag::List selectedTags;
        selectedTags.reserve(mMissingCategories.count());
        for (const auto &tag : tags) {
            if (mMissingCategories.removeAll(tag.name()) > 0) {
                selectedTags << tag;
            }
        }
        createMissingCategories();
        mUi->mTagWidget->blockSignals(true);
        mUi->mTagWidget->setSelection(selectedTags);
        mUi->mTagWidget->blockSignals(false);
    }
}

void IncidenceCategories::save(const KCalendarCore::Incidence::Ptr &incidence)
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
    for (const QString &category : std::as_const(mMissingCategories)) {
        Akonadi::Tag missingTag = Akonadi::Tag::genericTag(category);
        auto createJob = new Akonadi::TagCreateJob(missingTag, this);
        connect(createJob, &Akonadi::TagCreateJob::result, this, &IncidenceCategories::onMissingTagCreated);
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
    qCDebug(INCIDENCEEDITOR_LOG) << "mLoadedIncidence->categories() = " << mLoadedIncidence->categories();
}

void IncidenceCategories::onMissingTagCreated(KJob *job)
{
    if (job->error()) {
        qCWarning(INCIDENCEEDITOR_LOG) << "Failed to create tag " << job->errorString();
        return;
    }
    auto createJob = static_cast<Akonadi::TagCreateJob *>(job);
    int count = mMissingCategories.removeAll(createJob->tag().name());
    Q_ASSERT(count > 0);

    QVector<Akonadi::Tag> selectedTags;
    selectedTags.reserve(mUi->mTagWidget->selection().count() + 1);
    selectedTags << mUi->mTagWidget->selection() << createJob->tag();
    mUi->mTagWidget->setSelection(selectedTags);
}
