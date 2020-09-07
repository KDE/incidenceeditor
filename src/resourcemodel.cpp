/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */
#include "resourcemodel.h"
#include "ldaputils.h"

#include <KEmailAddress>
#include <QDebug>

using namespace IncidenceEditorNG;

ResourceModel::ResourceModel(const QStringList &headers, QObject *parent)
    : QAbstractItemModel(parent)
{
    this->headers = headers;
    rootItem = ResourceItem::Ptr(new ResourceItem(KLDAP::LdapDN(), headers, KLDAP::LdapClient(0)));

    ldapSearchCollections.setFilter(QStringLiteral(
                                        "&(ou=Resources,*)(objectClass=kolabGroupOfUniqueNames)(objectclass=groupofurls)(!(objectclass=nstombstone))(mail=*)"
                                        "(cn=%1)"));
    ldapSearch.setFilter(QStringLiteral(
                             "&(objectClass=kolabSharedFolder)(kolabFolderType=event)(mail=*)"
                             "(|(cn=%1)(description=%1)(kolabDescAttribute=%1))"));

    QStringList attrs = ldapSearchCollections.attributes();
    attrs << QStringLiteral("uniqueMember");
    ldapSearchCollections.setAttributes(attrs);
    ldapSearch.setAttributes(headers);

    connect(&ldapSearchCollections, qOverload<const KLDAP::LdapResultObject::List &>(&KLDAP :: LdapClientSearch :: searchData), this, &ResourceModel::slotLDAPCollectionData);
    connect(&ldapSearch, qOverload<const KLDAP::LdapResultObject::List &>(&KLDAP :: LdapClientSearch :: searchData), this, &ResourceModel::slotLDAPSearchData);

    ldapSearchCollections.startSearch(QStringLiteral("*"));
}

ResourceModel::~ResourceModel()
{
}

int ResourceModel::columnCount(const QModelIndex & /* parent */) const
{
    return rootItem->columnCount();
}

QVariant ResourceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::EditRole || role == Qt::DisplayRole) {
        return getItem(index)->data(index.column());
    } else if (role == Resource) {
        ResourceItem *p = getItem(parent(index));
        return QVariant::fromValue(p->child(index.row()));
    } else if (role == FullName) {
        ResourceItem *item = getItem(index);
        return KEmailAddress::normalizedAddress(item->data(QStringLiteral(
                                                               "cn")).toString(),
                                                item->data(QStringLiteral("mail")).toString());
    }

    return QVariant();
}

Qt::ItemFlags ResourceModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

ResourceItem *ResourceModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        ResourceItem *item = static_cast<ResourceItem *>(index.internalPointer());
        if (item) {
            return item;
        }
    }
    return rootItem.data();
}

QVariant ResourceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return translateLDAPAttributeForDisplay(rootItem->data(section).toString());
    }

    return QVariant();
}

QModelIndex ResourceModel::index(int row, int column, const QModelIndex &parent) const
{
    ResourceItem *parentItem = getItem(parent);

    ResourceItem::Ptr childItem = parentItem->child(row);
    if (row < parentItem->childCount() && childItem) {
        return createIndex(row, column, childItem.data());
    } else {
        return QModelIndex();
    }
}

QModelIndex ResourceModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }
    ResourceItem *childItem = getItem(index);
    ResourceItem::Ptr parentItem = childItem->parent();

    if (parentItem == rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->childNumber(), index.column(), parentItem.data());
}

bool ResourceModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    ResourceItem *parentItem = getItem(parent);

    beginRemoveRows(parent, position, position + rows - 1);
    bool success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

int ResourceModel::rowCount(const QModelIndex &parent) const
{
    ResourceItem *parentItem = getItem(parent);

    return parentItem->childCount();
}

void ResourceModel::startSearch(const QString &query)
{
    searchString = query;

    if (foundCollection) {
        startSearch();
    }
}

void ResourceModel::startSearch()
{
    // Delete all resources -> only collection elements are shown
    for (int i = 0; i < rootItem->childCount(); ++i) {
        if (ldapCollections.contains(rootItem->child(i))) {
            QModelIndex parentIndex = index(i, 0, QModelIndex());
            beginRemoveRows(parentIndex, 0, rootItem->child(i)->childCount() - 1);
            rootItem->child(i)->removeChildren(0, rootItem->child(i)->childCount());
            endRemoveRows();
        } else {
            beginRemoveRows(QModelIndex(), i, i);
            rootItem->removeChildren(i, 1);
            endRemoveRows();
        }
    }

    if (searchString.isEmpty()) {
        ldapSearch.startSearch(QStringLiteral("*"));
    } else {
        ldapSearch.startSearch(QLatin1Char('*') + searchString + QLatin1Char('*'));
    }
}

void ResourceModel::slotLDAPCollectionData(const KLDAP::LdapResultObject::List &results)
{
    Q_EMIT layoutAboutToBeChanged();

    foundCollection = true;
    ldapCollectionsMap.clear();
    ldapCollections.clear();

    //qDebug() <<  "Found ldapCollections";

    for (const KLDAP::LdapResultObject &result : qAsConst(results)) {
        ResourceItem::Ptr item(new ResourceItem(
                                   result.object.dn(), headers, *result.client, rootItem));
        item->setLdapObject(result.object);

        rootItem->insertChild(rootItem->childCount(), item);
        ldapCollections.insert(item);

        // Resources in a collection add this link into ldapCollectionsMap
        const auto members = result.object.attributes()[QStringLiteral("uniqueMember")];
        for (const QByteArray &member : members) {
            ldapCollectionsMap.insert(QString::fromLatin1(member), item);
        }
    }

    Q_EMIT layoutChanged();

    startSearch();
}

void ResourceModel::slotLDAPSearchData(const KLDAP::LdapResultObject::List &results)
{
    for (const KLDAP::LdapResultObject &result : qAsConst(results)) {
        //Add the found items to all collections, where it is member
        QList<ResourceItem::Ptr> parents = ldapCollectionsMap.values(result.object.dn().toString());
        if (parents.isEmpty()) {
            parents << rootItem;
        }

        for (const ResourceItem::Ptr &parent : qAsConst(parents)) {
            ResourceItem::Ptr item(new ResourceItem(
                                       result.object.dn(), headers, *result.client, parent));
            item->setLdapObject(result.object);

            QModelIndex parentIndex;
            if (parent != rootItem) {
                parentIndex = index(parent->childNumber(), 0, parentIndex);
            }
            beginInsertRows(parentIndex, parent->childCount(), parent->childCount());
            parent->insertChild(parent->childCount(), item);
            endInsertRows();
        }
    }
}
