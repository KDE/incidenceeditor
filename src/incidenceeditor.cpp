/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "incidenceeditor-ng.h"

#include "incidenceeditor_debug.h"

using namespace IncidenceEditorNG;

IncidenceEditor::IncidenceEditor(QObject *parent)
    : QObject(parent)
{
}

IncidenceEditor::~IncidenceEditor()
{
}

void IncidenceEditor::checkDirtyStatus()
{
    if (!mLoadedIncidence) {
        qCDebug(INCIDENCEEDITOR_LOG) << "checkDirtyStatus called on an invalid incidence";
        return;
    }

    if (mLoadingIncidence) {
        // Still loading the incidence, ignore changes to widgets.
        return;
    }
    const bool dirty = isDirty();
    if (mWasDirty != dirty) {
        mWasDirty = dirty;
        Q_EMIT dirtyStatusChanged(dirty);
    }
}

bool IncidenceEditor::isValid() const
{
    mLastErrorString.clear();
    return true;
}

QString IncidenceEditor::lastErrorString() const
{
    return mLastErrorString;
}

void IncidenceEditor::focusInvalidField()
{
}

KCalendarCore::IncidenceBase::IncidenceType IncidenceEditor::type() const
{
    if (mLoadedIncidence) {
        return mLoadedIncidence->type();
    } else {
        return KCalendarCore::IncidenceBase::TypeUnknown;
    }
}

void IncidenceEditor::printDebugInfo() const
{
    // implement this in derived classes.
}

void IncidenceEditor::load(const Akonadi::Item &item)
{
    Q_UNUSED(item)
}

void IncidenceEditor::save(Akonadi::Item &item)
{
    Q_UNUSED(item)
}

#include "moc_incidenceeditor-ng.cpp"
