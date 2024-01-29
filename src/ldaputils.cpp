/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#include "ldaputils.h"
#include <KLocalizedString>

QString IncidenceEditorNG::translateLDAPAttributeForDisplay(const QString &attribute)
{
    QString ret = attribute;
    if (attribute == QLatin1StringView("cn")) {
        ret = i18nc("ldap attribute cn", "Common name");
    } else if (attribute == QLatin1StringView("mail")) {
        ret = i18nc("ldap attribute mail", "Email");
    } else if (attribute == QLatin1StringView("givenname")) {
        ret = i18nc("ldap attribute givenname", "Given name");
    } else if (attribute == QLatin1StringView("sn")) {
        ret = i18nc("ldap attribute sn", "Surname");
    } else if (attribute == QLatin1StringView("ou")) {
        ret = i18nc("ldap attribute ou", "Organization");
    } else if (attribute == QLatin1StringView("objectClass")) {
        ret = i18nc("ldap attribute objectClass", "Object class");
    } else if (attribute == QLatin1StringView("description")) {
        ret = i18nc("ldap attribute description", "Description");
    } else if (attribute == QLatin1StringView("telephoneNumber")) {
        ret = i18nc("ldap attribute telephoneNumber", "Telephone");
    } else if (attribute == QLatin1StringView("mobile")) {
        ret = i18nc("ldap attribute mobile", "Mobile");
    }
    return ret;
}
