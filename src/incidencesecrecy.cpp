/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "incidencesecrecy.h"
#include "ui_dialogdesktop.h"

#include <KCalUtils/Stringify>

using namespace IncidenceEditorNG;

IncidenceSecrecy::IncidenceSecrecy(Ui::EventOrTodoDesktop *ui)
    : mUi(ui)
{
    setObjectName(QStringLiteral("IncidenceSecrecy"));
    mUi->mSecrecyCombo->addItems(KCalUtils::Stringify::incidenceSecrecyList());
    connect(mUi->mSecrecyCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &IncidenceSecrecy::checkDirtyStatus);
}

void IncidenceSecrecy::load(const KCalendarCore::Incidence::Ptr &incidence)
{
    mLoadedIncidence = incidence;
    if (mLoadedIncidence) {
        Q_ASSERT(mUi->mSecrecyCombo->count() == KCalUtils::Stringify::incidenceSecrecyList().count());
        mUi->mSecrecyCombo->setCurrentIndex(mLoadedIncidence->secrecy());
    } else {
        mUi->mSecrecyCombo->setCurrentIndex(0);
    }

    mWasDirty = false;
}

void IncidenceSecrecy::save(const KCalendarCore::Incidence::Ptr &incidence)
{
    Q_ASSERT(incidence);
    switch (mUi->mSecrecyCombo->currentIndex()) {
    case 1:
        incidence->setSecrecy(KCalendarCore::Incidence::SecrecyPrivate);
        break;
    case 2:
        incidence->setSecrecy(KCalendarCore::Incidence::SecrecyConfidential);
        break;
    default:
        incidence->setSecrecy(KCalendarCore::Incidence::SecrecyPublic);
    }
}

bool IncidenceSecrecy::isDirty() const
{
    if (mLoadedIncidence) {
        if (mLoadedIncidence->secrecy() != mUi->mSecrecyCombo->currentIndex()) {
            return true;
        }
    } else {
        if (mUi->mSecrecyCombo->currentIndex() != 0) {
            return true;
        }
    }

    return false;
}
