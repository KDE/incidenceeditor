/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#ifndef LDAPUTILS_H
#define LDAPUTILS_H

#include <QString>

namespace IncidenceEditorNG {
Q_REQUIRED_RESULT QString translateLDAPAttributeForDisplay(const QString &attribute);
} // namespace IncidenceEditorNG

#endif //LDAPUTLS_H
