/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "incidenceeditor_export.h"

/* Classes which are exported only for unit tests */
#ifdef BUILD_TESTING
#ifndef INCIDENCEEDITOR_TESTS_EXPORT
#define INCIDENCEEDITOR_TESTS_EXPORT INCIDENCEEDITOR_EXPORT
#endif
#else /* not compiling tests */
#define INCIDENCEEDITOR_TESTS_EXPORT
#endif
