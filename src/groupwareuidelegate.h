/*
  SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "incidenceeditor_export.h"

#include <Akonadi/Calendar/ITIPHandler>

// Class to edit counter proposals through incidence editors
namespace IncidenceEditorNG
{
class INCIDENCEEDITOR_EXPORT GroupwareUiDelegate : public Akonadi::GroupwareUiDelegate
{
public:
    void requestIncidenceEditor(const Akonadi::Item &item) override;
    void setCalendar(const Akonadi::ETMCalendar::Ptr &calendar) override;
    void createCalendar() override;
};
}

