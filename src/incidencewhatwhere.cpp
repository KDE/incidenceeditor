/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "incidencewhatwhere.h"
#include "ui_dialogdesktop.h"

#include "incidenceeditor_debug.h"
#include <KLocalizedString>

using namespace IncidenceEditorNG;

IncidenceWhatWhere::IncidenceWhatWhere(Ui::EventOrTodoDesktop *ui)
    : IncidenceEditor(nullptr)
    , mUi(ui)
{
    setObjectName(QStringLiteral("IncidenceWhatWhere"));
    connect(mUi->mSummaryEdit, &QLineEdit::textChanged, this, &IncidenceWhatWhere::checkDirtyStatus);
    connect(mUi->mLocationEdit, &QLineEdit::textChanged, this, &IncidenceWhatWhere::checkDirtyStatus);
}

void IncidenceWhatWhere::load(const KCalendarCore::Incidence::Ptr &incidence)
{
    qCDebug(INCIDENCEEDITOR_LOG);
    mLoadedIncidence = incidence;
    if (mLoadedIncidence) {
        mUi->mSummaryEdit->setText(mLoadedIncidence->summary());
        mUi->mLocationEdit->setText(mLoadedIncidence->location());
    } else {
        mUi->mSummaryEdit->clear();
        mUi->mLocationEdit->clear();
    }

    mUi->mLocationEdit->setVisible(type() != KCalendarCore::Incidence::TypeJournal);
    mUi->mLocationLabel->setVisible(type() != KCalendarCore::Incidence::TypeJournal);

    mWasDirty = false;
}

void IncidenceWhatWhere::save(const KCalendarCore::Incidence::Ptr &incidence)
{
    Q_ASSERT(incidence);
    incidence->setSummary(mUi->mSummaryEdit->text());
    incidence->setLocation(mUi->mLocationEdit->text());
}

bool IncidenceWhatWhere::isDirty() const
{
    if (mLoadedIncidence) {
        return (mUi->mSummaryEdit->text() != mLoadedIncidence->summary()) || (mUi->mLocationEdit->text() != mLoadedIncidence->location());
    } else {
        return mUi->mSummaryEdit->text().isEmpty() && mUi->mLocationEdit->text().isEmpty();
    }
}

void IncidenceWhatWhere::focusInvalidField()
{
    if (mUi->mSummaryEdit->text().isEmpty()) {
        mUi->mSummaryEdit->setFocus();
    }
}

bool IncidenceWhatWhere::isValid() const
{
    if (mUi->mSummaryEdit->text().isEmpty()) {
        qCDebug(INCIDENCEEDITOR_LOG) << "Specify a summary";
        mLastErrorString = i18nc("@info", "Please enter a summary.");
        return false;
    } else {
        mLastErrorString.clear();
        return true;
    }
}

void IncidenceWhatWhere::validate()
{
    if (mUi->mSummaryEdit->text().isEmpty()) {
        mUi->mSummaryEdit->setFocus();
    }
}
