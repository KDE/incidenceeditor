/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Tobias Koenig <tokoe@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "incidenceeditorsettings.h"

using namespace IncidenceEditorNG;

IncidenceEditorSettings *IncidenceEditorSettings::mSelf = nullptr;

IncidenceEditorSettings *IncidenceEditorSettings::self()
{
    if (!mSelf) {
        mSelf = new IncidenceEditorSettings();
        mSelf->load();
    }

    return mSelf;
}

IncidenceEditorSettings::IncidenceEditorSettings()
{
}

IncidenceEditorSettings::~IncidenceEditorSettings()
{
}
