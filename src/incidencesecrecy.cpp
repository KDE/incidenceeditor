/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
  SPDX-FileCopyrightText: Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "incidencesecrecy.h"
using namespace Qt::Literals::StringLiterals;

#include "ui_dialogdesktop.h"

#include <KCalUtils/Stringify>

using namespace IncidenceEditorNG;

IncidenceSecrecy::IncidenceSecrecy(Ui::EventOrTodoDesktop *ui)
    : mUi(ui)
{
    setObjectName("IncidenceSecrecy"_L1);
    for (const auto secrecy :
         {KCalendarCore::Incidence::SecrecyPublic, KCalendarCore::Incidence::SecrecyPrivate, KCalendarCore::Incidence::SecrecyConfidential}) {
        mUi->mSecrecyCombo->addItem(KCalUtils::Stringify::incidenceSecrecy(secrecy), secrecy);
    }
    connect(mUi->mSecrecyCombo, &QComboBox::currentIndexChanged, this, &IncidenceSecrecy::checkDirtyStatus);
}

void IncidenceSecrecy::load(const KCalendarCore::Incidence::Ptr &incidence)
{
    mLoadedIncidence = incidence;
    mUi->mSecrecyCombo->setCurrentIndex(mUi->mSecrecyCombo->findData(mLoadedIncidence->secrecy()));
    mWasDirty = false;
}

void IncidenceSecrecy::save(const KCalendarCore::Incidence::Ptr &incidence)
{
    Q_ASSERT(incidence);
    qDebug() << mUi->mSecrecyCombo->currentData();
    incidence->setSecrecy(mUi->mSecrecyCombo->currentData().value<KCalendarCore::Incidence::Secrecy>());
}

bool IncidenceSecrecy::isDirty() const
{
    if (mLoadedIncidence) {
        if (mLoadedIncidence->secrecy() != mUi->mSecrecyCombo->currentData().value<KCalendarCore::Incidence::Secrecy>()) {
            return true;
        }
    } else {
        if (mUi->mSecrecyCombo->currentIndex() != 0) {
            return true;
        }
    }

    return false;
}

#include "moc_incidencesecrecy.cpp"
