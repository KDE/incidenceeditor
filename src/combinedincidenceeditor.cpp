/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "combinedincidenceeditor.h"

#include "incidenceeditor_debug.h"

using namespace IncidenceEditorNG;

/// public methods

CombinedIncidenceEditor::CombinedIncidenceEditor(QWidget *parent)
    : IncidenceEditor(parent)
{
}

CombinedIncidenceEditor::~CombinedIncidenceEditor()
{
    qDeleteAll(mCombinedEditors);
}

void CombinedIncidenceEditor::combine(IncidenceEditor *other)
{
    Q_ASSERT(other);
    mCombinedEditors.append(other);
    connect(other, &IncidenceEditor::dirtyStatusChanged, this, &CombinedIncidenceEditor::handleDirtyStatusChange);
}

bool CombinedIncidenceEditor::isDirty() const
{
    return mDirtyEditorCount > 0;
}

bool CombinedIncidenceEditor::isValid() const
{
    for (IncidenceEditor *editor : std::as_const(mCombinedEditors)) {
        if (!editor->isValid()) {
            const QString reason = editor->lastErrorString();
            editor->focusInvalidField();
            if (!reason.isEmpty()) {
                Q_EMIT showMessage(reason, KMessageWidget::Warning);
            }
            return false;
        }
    }

    return true;
}

void CombinedIncidenceEditor::handleDirtyStatusChange(bool isDirty)
{
    const int prevDirtyCount = mDirtyEditorCount;

    Q_ASSERT(mDirtyEditorCount >= 0);

    if (isDirty) {
        ++mDirtyEditorCount;
    } else {
        --mDirtyEditorCount;
    }

    Q_ASSERT(mDirtyEditorCount >= 0);

    if (prevDirtyCount == 0) {
        Q_EMIT dirtyStatusChanged(true);
    }
    if (mDirtyEditorCount == 0) {
        Q_EMIT dirtyStatusChanged(false);
    }
}

void CombinedIncidenceEditor::load(const KCalendarCore::Incidence::Ptr &incidence)
{
    mLoadedIncidence = incidence;
    for (IncidenceEditor *editor : std::as_const(mCombinedEditors)) {
        // load() may fire dirtyStatusChanged(), reset mDirtyEditorCount to make sure
        // we don't end up with an invalid dirty count.
        editor->blockSignals(true);
        editor->load(incidence);
        editor->blockSignals(false);

        if (editor->isDirty()) {
            // We are going to crash due to assert. Print some useful info before crashing.
            qCWarning(INCIDENCEEDITOR_LOG) << "Faulty editor was " << editor->objectName();
            qCWarning(INCIDENCEEDITOR_LOG) << "Incidence " << (incidence ? incidence->uid() : QStringLiteral("null"));

            editor->printDebugInfo();

            Q_ASSERT_X(false, "load", "editor shouldn't be dirty");
        }
    }

    mWasDirty = false;
    mDirtyEditorCount = 0;
    Q_EMIT dirtyStatusChanged(false);
}

void CombinedIncidenceEditor::load(const Akonadi::Item &item)
{
    for (IncidenceEditor *editor : std::as_const(mCombinedEditors)) {
        // load() may fire dirtyStatusChanged(), reset mDirtyEditorCount to make sure
        // we don't end up with an invalid dirty count.
        editor->blockSignals(true);
        editor->load(item);
        editor->blockSignals(false);

        if (editor->isDirty()) {
            // We are going to crash due to assert. Print some useful info before crashing.
            qCWarning(INCIDENCEEDITOR_LOG) << "Faulty editor was " << editor->objectName();
            // qCWarning(INCIDENCEEDITOR_LOG) << "Incidence " << ( incidence ? incidence->uid() : "null" );

            editor->printDebugInfo();

            Q_ASSERT_X(false, "load", "editor shouldn't be dirty");
        }
    }

    mWasDirty = false;
    mDirtyEditorCount = 0;
    Q_EMIT dirtyStatusChanged(false);
}

void CombinedIncidenceEditor::save(const KCalendarCore::Incidence::Ptr &incidence)
{
    for (IncidenceEditor *editor : std::as_const(mCombinedEditors)) {
        editor->save(incidence);
    }
}

void CombinedIncidenceEditor::save(Akonadi::Item &item)
{
    for (IncidenceEditor *editor : std::as_const(mCombinedEditors)) {
        editor->save(item);
    }
}
