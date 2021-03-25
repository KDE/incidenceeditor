/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "incidenceeditor_export.h"

#include <AkonadiCore/Item>
#include <KCalendarCore/Incidence>
namespace IncidenceEditorNG
{
/**
 * KCal Incidences are complicated objects. The user interfaces to create/modify
 * are therefore complex too. The IncedenceEditor class is a divide and conquer
 * approach to this complexity. An IncidenceEditor is an editor for a specific
 * part(s) of an Incidence.
 */
class INCIDENCEEDITOR_EXPORT IncidenceEditor : public QObject
{
    Q_OBJECT
public:
    ~IncidenceEditor() override;

    /**
     * Load the values of @param incidence into the editor widgets. The passed
     * incidence is kept for comparing with the current values of the editor.
     */
    virtual void load(const KCalendarCore::Incidence::Ptr &incidence) = 0;
    /// This was introduced to replace categories with Akonadi::Tags
    virtual void load(const Akonadi::Item &item);

    /**
     * Store the current values of the editor into @param incidence .
     */
    virtual void save(const KCalendarCore::Incidence::Ptr &incidence) = 0;
    /// This was introduced to replace categories with Akonadi::Tags
    virtual void save(Akonadi::Item &item);

    /**
     * Returns whether or not the current values in the editor differ from the
     * initial values.
     */
    virtual bool isDirty() const = 0;

    /**
     * Returns whether or not the content of this editor is valid. The default
     * implementation returns always true.
     */
    virtual bool isValid() const;

    /**
       Returns the last error, which is set in isValid() on error,
       and cleared on success.
    */
    Q_REQUIRED_RESULT QString lastErrorString() const;

    /**
     * Sets focus on the invalid field.
     */
    virtual void focusInvalidField();

    /**
     * Returns the type of the Incidence that is currently loaded.
     */
    Q_REQUIRED_RESULT KCalendarCore::IncidenceBase::IncidenceType type() const;

    /** Convenience method to get a pointer for a specific const Incidence Type. */
    template<typename IncidenceT> QSharedPointer<IncidenceT> incidence() const
    {
        return mLoadedIncidence.dynamicCast<IncidenceT>();
    }

    /**
       Re-implement this and print important member values and widget
       enabled/disabled states that could have lead to isDirty() returning
       true when the user didn't do any interaction with the editor.

       This method is called in CombinedIncidenceEditor before crashing
       due to assert( !editor->isDirty() )
    */
    virtual void printDebugInfo() const;

Q_SIGNALS:
    /**
     * Signals whether the dirty status of this editor has changed. The new dirty
     * status is passed as argument.
     */
    void dirtyStatusChanged(bool isDirty);

public Q_SLOTS:
    /**
     * Checks if the dirty status has changed until last check and emits the
     * dirtyStatusChanged signal if needed.
     */
    void checkDirtyStatus();

protected:
    /** Only subclasses can instantiate IncidenceEditors */
    IncidenceEditor(QObject *parent = nullptr);

    template<typename IncidenceT> QSharedPointer<IncidenceT> incidence(const KCalendarCore::Incidence::Ptr &inc)
    {
        return inc.dynamicCast<IncidenceT>();
    }

protected:
    KCalendarCore::Incidence::Ptr mLoadedIncidence;
    mutable QString mLastErrorString;
    bool mWasDirty = false;
    bool mLoadingIncidence = false;
};
} // IncidenceEditorNG

