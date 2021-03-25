/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#pragma once

#include "resourceitem.h"

#include <KLDAP/LdapClientSearch>

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QSet>

namespace IncidenceEditorNG
{
class ResourceModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    /* Copied from https://doc.qt.io/qt-5/qtwidgets-itemviews-editabletreemodel-example.html:
     * Editable Tree Model Example
     */
    enum Roles { Resource = Qt::UserRole, FullName };

    explicit ResourceModel(const QStringList &headers, QObject *parent = nullptr);
    ~ResourceModel() override;

    Q_REQUIRED_RESULT QVariant data(const QModelIndex &index, int role) const override;
    Q_REQUIRED_RESULT QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    Q_REQUIRED_RESULT QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    Q_REQUIRED_RESULT QModelIndex parent(const QModelIndex &index) const override;

    Q_REQUIRED_RESULT int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Q_REQUIRED_RESULT int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Q_REQUIRED_RESULT Qt::ItemFlags flags(const QModelIndex &index) const override;

    Q_REQUIRED_RESULT bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;

private:
    ResourceItem *getItem(const QModelIndex &index) const;

    ResourceItem::Ptr mRootItem;

public:
    /* Start search on LDAP Server with the given string.
     * If the model is not ready to search, the string is cached and is executed afterwards.
     */
    void startSearch(const QString &);

private:
    /* Start search with cached string (stored in searchString)
     *
     */
    void startSearch();

    /* Search for collections of resources
     *
     */
    KLDAP::LdapClientSearch *mLdapSearchCollections = nullptr;

    /* Search for matching resources
     *
     */
    KLDAP::LdapClientSearch *mLdapSearch = nullptr;

    /* Map from dn of resource -> collectionItem
     * A Resource can be part of different collection, so a QMuliMap is needed
     *
     */
    QMultiMap<QString, ResourceItem::Ptr> mLdapCollectionsMap;

    /* A Set of all collection ResourceItems
     *
     */
    QSet<ResourceItem::Ptr> mLdapCollections;

    /* Cached searchString (set by startSearch(QString))
     *
     */
    QString mSearchString;

    /* Is the search of collections ended
     *
     */
    bool mFoundCollection = false;

    /* List of all attributes in LDAP an the headers of the model
     *
     */
    QStringList mHeaders;

private:
    /* Slot for founded collections
     *
     */
    void slotLDAPCollectionData(const KLDAP::LdapResultObject::List &);

    /* Slot for matching resources
     *
     */
    void slotLDAPSearchData(const KLDAP::LdapResultObject::List &);
};
}
