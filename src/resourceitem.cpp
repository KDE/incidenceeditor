/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */
#include "resourceitem.h"

#include <KLDAP/LdapServer>

using namespace IncidenceEditorNG;

ResourceItem::ResourceItem(const KLDAP::LdapDN &dn, const QStringList &attrs, const KLDAP::LdapClient &ldapClient, const ResourceItem::Ptr &parent)
    : parentItem(parent)
    , dn(dn)
    , mAttrs(attrs)
    , mLdapClient(0, this)
{
    if (!dn.isEmpty()) {
        KLDAP::LdapServer server = ldapClient.server();

        server.setScope(KLDAP::LdapUrl::Base);
        server.setBaseDn(dn);
        mLdapClient.setServer(server);

        connect(&mLdapClient, &KLDAP::LdapClient::result, this, &ResourceItem::slotLDAPResult);

        mAttrs << QStringLiteral("uniqueMember");
        mLdapClient.setAttributes(attrs);
    } else {
        itemData.reserve(mAttrs.count());
        for (const QString &header : std::as_const(mAttrs)) {
            itemData << header;
        }
    }
}

ResourceItem::~ResourceItem()
{
}

ResourceItem::Ptr ResourceItem::child(int number)
{
    return childItems.value(number);
}

int ResourceItem::childCount() const
{
    return childItems.count();
}

int ResourceItem::childNumber() const
{
    if (parentItem) {
        int i = 0;
        for (const ResourceItem::Ptr &child : std::as_const(parentItem->childItems)) {
            if (child == this) {
                return i;
            }
            i++;
        }
    }

    return 0;
}

int ResourceItem::columnCount() const
{
    return itemData.count();
}

QVariant ResourceItem::data(int column) const
{
    return itemData.value(column);
}

QVariant ResourceItem::data(const QString &column) const
{
    if (!mLdapObject.attributes()[column].isEmpty()) {
        return QString::fromUtf8(mLdapObject.attributes()[column][0]);
    }
    return QVariant();
}

bool ResourceItem::insertChild(int position, const ResourceItem::Ptr &item)
{
    if (position < 0 || position > childItems.size()) {
        return false;
    }

    childItems.insert(position, item);

    return true;
}

ResourceItem::Ptr ResourceItem::parent()
{
    return parentItem;
}

bool ResourceItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > childItems.size()) {
        return false;
    }

    for (int row = 0; row < count; ++row) {
        childItems.removeAt(position);
    }

    return true;
}

const QStringList &ResourceItem::attributes() const
{
    return mAttrs;
}

const KLDAP::LdapObject &ResourceItem::ldapObject() const
{
    return mLdapObject;
}

void ResourceItem::startSearch()
{
    mLdapClient.startQuery(QStringLiteral("objectclass=*"));
}

void ResourceItem::setLdapObject(const KLDAP::LdapObject &obj)
{
    slotLDAPResult(mLdapClient, obj);
}

const KLDAP::LdapClient &ResourceItem::ldapClient() const
{
    return mLdapClient;
}

void ResourceItem::slotLDAPResult(const KLDAP::LdapClient &client, const KLDAP::LdapObject &obj)
{
    Q_UNUSED(client)
    mLdapObject = obj;
    for (const QString &header : std::as_const(mAttrs)) {
        if (!obj.attributes()[header].isEmpty()) {
            itemData << QString::fromUtf8(obj.attributes()[header][0]);
        } else {
            itemData << QString();
        }
    }
    Q_EMIT searchFinished();
}
