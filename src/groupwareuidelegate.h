/*
  SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "incidenceeditor_export.h"

#include <Akonadi/ITIPHandler>

// Class to edit counter proposals through incidence editors
namespace IncidenceEditorNG
{

/*!
 * \class IncidenceEditorNG::GroupwareUiDelegate
 * \inmodule IncidenceEditor
 * \inheaderfile IncidenceEditor/GroupwareUiDelegate
 */
class INCIDENCEEDITOR_EXPORT GroupwareUiDelegate : public Akonadi::GroupwareUiDelegate
{
public:
    /*!
     * Requests to open an incidence editor for the given item.
     * \a item The Akonadi item to edit.
     */
    void requestIncidenceEditor(const Akonadi::Item &item) override;
    /*!
     * Sets the calendar to use for groupware operations.
     * \a calendar The ETM calendar instance.
     */
    void setCalendar(const Akonadi::ETMCalendar::Ptr &calendar) override;
    /*!
     * Creates a new calendar for groupware operations.
     */
    void createCalendar() override;
};
}
