/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#pragma once

#include <QString>

namespace IncidenceEditorNG
{
[[nodiscard]] QString translateLDAPAttributeForDisplay(const QString &attribute);
} // namespace IncidenceEditorNG
