/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#include "ldaputils.h"
using namespace Qt::Literals::StringLiterals;

#include <KLocalizedString>

QString IncidenceEditorNG::translateLDAPAttributeForDisplay(const QString &attribute)
{
    QString ret = attribute;
    if (attribute == "cn"_L1) {
        ret = i18nc("ldap attribute cn", "Common name");
    } else if (attribute == "mail"_L1) {
        ret = i18nc("ldap attribute mail", "Email");
    } else if (attribute == "givenname"_L1) {
        ret = i18nc("ldap attribute givenname", "Given name");
    } else if (attribute == "sn"_L1) {
        ret = i18nc("ldap attribute sn", "Surname");
    } else if (attribute == "ou"_L1) {
        ret = i18nc("ldap attribute ou", "Organization");
    } else if (attribute == "objectClass"_L1) {
        ret = i18nc("ldap attribute objectClass", "Object class");
    } else if (attribute == "description"_L1) {
        ret = i18nc("ldap attribute description", "Description");
    } else if (attribute == "telephoneNumber"_L1) {
        ret = i18nc("ldap attribute telephoneNumber", "Telephone");
    } else if (attribute == "mobile"_L1) {
        ret = i18nc("ldap attribute mobile", "Mobile");
    }
    return ret;
}
