/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "incidenceeditor-ng.h"

#include <AkonadiCore/Item>
#include <KMessageWidget>

namespace IncidenceEditorNG
{
/**
 * The CombinedIncidenceEditor combines optional widgets with zero or more
 * IncidenceEditors. The CombinedIncidenceEditor keeps track of the dirty state
 * of the IncidenceEditors that where combined.
 */
class CombinedIncidenceEditor : public IncidenceEditor
{
    Q_OBJECT
public:
    explicit CombinedIncidenceEditor(QWidget *parent = nullptr);
    /**
     * Deletes this editor as well as all editors which are combined into this
     * one.
     */
    ~CombinedIncidenceEditor() override;

    void combine(IncidenceEditor *other);

    /**
     * Returns whether or not the current values in the editor differ from the
     * initial values or if one of the combined editors is dirty.
     */
    Q_REQUIRED_RESULT bool isDirty() const override;
    Q_REQUIRED_RESULT bool isValid() const override;

    /**
     * Loads all data from @param incidence into the combined editors. Note, if
     * you reimplement the load method in a subclass, make sure to call this
     * implementation too.
     */
    void load(const KCalendarCore::Incidence::Ptr &incidence) override;
    void load(const Akonadi::Item &item) override;
    void save(const KCalendarCore::Incidence::Ptr &incidence) override;
    void save(Akonadi::Item &item) override;

Q_SIGNALS:
    void showMessage(const QString &reason, KMessageWidget::MessageType) const;

private:
    void handleDirtyStatusChange(bool isDirty);
    QVector<IncidenceEditor *> mCombinedEditors;
    int mDirtyEditorCount = 0;
};
}

