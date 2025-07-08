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
using namespace Qt::Literals::StringLiterals;
ResourceModel::ResourceModel(const QStringList &headers, QObject *parent)
    : QAbstractItemModel(parent)
    , mRootItem(ResourceItem::Ptr(new ResourceItem(KLDAPCore::LdapDN(), headers, KLDAPCore::LdapClient(0))))
{
    this->mHeaders = headers;
    const QStringList attrs = QStringList() << KLDAPCore::LdapClientSearch::defaultAttributes() << u"uniqueMember"_s;
    mLdapSearchCollections = new KLDAPCore::LdapClientSearch(attrs, this);
    mLdapSearch = new KLDAPCore::LdapClientSearch(headers, this);

    mLdapSearchCollections->setFilter(
        QStringLiteral("&(ou=Resources,*)(objectClass=kolabGroupOfUniqueNames)(objectclass=groupofurls)(!(objectclass=nstombstone))(mail=*)"
                       "(cn=%1)"));
    mLdapSearch->setFilter(
        QStringLiteral("&(objectClass=kolabSharedFolder)(kolabFolderType=event)(mail=*)"
                       "(|(cn=%1)(description=%1)(kolabDescAttribute=%1))"));

    connect(mLdapSearchCollections,
            qOverload<const KLDAPCore::LdapResultObject::List &>(&KLDAPCore::LdapClientSearch ::searchData),
            this,
            &ResourceModel::slotLDAPCollectionData);
    connect(mLdapSearch,
            qOverload<const KLDAPCore::LdapResultObject::List &>(&KLDAPCore::LdapClientSearch::searchData),
            this,
            &ResourceModel::slotLDAPSearchData);

    mLdapSearchCollections->startSearch(u"*"_s);
}

ResourceModel::~ResourceModel() = default;

int ResourceModel::columnCount(const QModelIndex & /* parent */) const
{
    return mRootItem->columnCount();
}

QVariant ResourceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (role == Qt::EditRole || role == Qt::DisplayRole) {
        return getItem(index)->data(index.column());
    } else if (role == Resource) {
        ResourceItem *p = getItem(parent(index));
        return QVariant::fromValue(p->child(index.row()));
    } else if (role == FullName) {
        ResourceItem *item = getItem(index);
        return KEmailAddress::normalizedAddress(item->data(u"cn"_s).toString(), item->data(u"mail"_s).toString());
    }

    return {};
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
        auto item = static_cast<ResourceItem *>(index.internalPointer());
        if (item) {
            return item;
        }
    }
    return mRootItem.data();
}

QVariant ResourceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return translateLDAPAttributeForDisplay(mRootItem->data(section).toString());
    }

    return {};
}

QModelIndex ResourceModel::index(int row, int column, const QModelIndex &parent) const
{
    ResourceItem *parentItem = getItem(parent);

    ResourceItem::Ptr const childItem = parentItem->child(row);
    if (row < parentItem->childCount() && childItem) {
        return createIndex(row, column, childItem.data());
    } else {
        return {};
    }
}

QModelIndex ResourceModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return {};
    }
    ResourceItem *childItem = getItem(index);
    ResourceItem::Ptr const parentItem = childItem->parent();

    if (parentItem == mRootItem) {
        return {};
    }

    return createIndex(parentItem->childNumber(), index.column(), parentItem.data());
}

bool ResourceModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    ResourceItem *parentItem = getItem(parent);

    beginRemoveRows(parent, position, position + rows - 1);
    bool const success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

int ResourceModel::rowCount(const QModelIndex &parent) const
{
    const ResourceItem *parentItem = getItem(parent);

    return parentItem->childCount();
}

void ResourceModel::startSearch(const QString &query)
{
    mSearchString = query;

    if (mFoundCollection) {
        startSearch();
    }
}

void ResourceModel::startSearch()
{
    // Delete all resources -> only collection elements are shown
    for (int i = 0; i < mRootItem->childCount(); ++i) {
        if (mLdapCollections.contains(mRootItem->child(i))) {
            QModelIndex const parentIndex = index(i, 0, QModelIndex());
            beginRemoveRows(parentIndex, 0, mRootItem->child(i)->childCount() - 1);
            (void)mRootItem->child(i)->removeChildren(0, mRootItem->child(i)->childCount());
            endRemoveRows();
        } else {
            beginRemoveRows(QModelIndex(), i, i);
            (void)mRootItem->removeChildren(i, 1);
            endRemoveRows();
        }
    }

    if (mSearchString.isEmpty()) {
        mLdapSearch->startSearch(u"*"_s);
    } else {
        mLdapSearch->startSearch(u'*' + mSearchString + u'*');
    }
}

void ResourceModel::slotLDAPCollectionData(const KLDAPCore::LdapResultObject::List &results)
{
    Q_EMIT layoutAboutToBeChanged();

    mFoundCollection = true;
    mLdapCollectionsMap.clear();
    mLdapCollections.clear();

    // qDebug() <<  "Found ldapCollections";

    for (const KLDAPCore::LdapResultObject &result : std::as_const(results)) {
        ResourceItem::Ptr const item(new ResourceItem(result.object.dn(), mHeaders, *result.client, mRootItem));
        item->setLdapObject(result.object);

        (void)mRootItem->insertChild(mRootItem->childCount(), item);
        mLdapCollections.insert(item);

        // Resources in a collection add this link into ldapCollectionsMap
        const auto members = result.object.attributes()[u"uniqueMember"_s];
        for (const QByteArray &member : members) {
            mLdapCollectionsMap.insert(QString::fromLatin1(member), item);
        }
    }

    Q_EMIT layoutChanged();

    startSearch();
}

void ResourceModel::slotLDAPSearchData(const KLDAPCore::LdapResultObject::List &results)
{
    for (const KLDAPCore::LdapResultObject &result : std::as_const(results)) {
        // Add the found items to all collections, where it is member
        QList<ResourceItem::Ptr> parents = mLdapCollectionsMap.values(result.object.dn().toString());
        if (parents.isEmpty()) {
            parents << mRootItem;
        }

        for (const ResourceItem::Ptr &p : std::as_const(parents)) {
            ResourceItem::Ptr const item(new ResourceItem(result.object.dn(), mHeaders, *result.client, p));
            item->setLdapObject(result.object);

            QModelIndex parentIndex;
            if (p != mRootItem) {
                parentIndex = index(p->childNumber(), 0, parentIndex);
            }
            beginInsertRows(parentIndex, p->childCount(), p->childCount());
            (void)p->insertChild(p->childCount(), item);
            endInsertRows();
        }
    }
}

#include "moc_resourcemodel.cpp"
