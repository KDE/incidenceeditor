/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Tobias Koenig <tokoe@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "incidenceeditor_export.h"

#include "globalsettings_incidenceeditor.h"

namespace IncidenceEditorNG
{
/*!
 * \class IncidenceEditorNG::IncidenceEditorSettings
 * \inmodule IncidenceEditor
 * \inheaderfile IncidenceEditor/IncidenceEditorSettings
 *
 * \brief The IncidenceEditorSettings class
 */
class INCIDENCEEDITOR_EXPORT IncidenceEditorSettings : public IncidenceEditorNG::IncidenceEditorSettingsBase
{
    Q_OBJECT

public:
    /*!
     * Returns the singleton instance of IncidenceEditorSettings.
     */
    static IncidenceEditorSettings *self();

private:
    IncidenceEditorSettings();
    ~IncidenceEditorSettings() override;
    static IncidenceEditorSettings *mSelf;
};
}
