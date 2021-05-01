/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#pragma once

#include <KLDAP/LdapClient>

#include <KLDAP/LdapObject>

#include <QSharedPointer>
#include <QStringList>
#include <QVariant>
#include <QVector>

namespace IncidenceEditorNG
{
class ResourceItem : public QObject
{
    Q_OBJECT
public:
    /* Copied from https://doc.qt.io/qt-5/qtwidgets-itemviews-editabletreemodel-example.html:
     * Editable Tree Model Example
     */

    /**
      A shared pointer to an ResourceItem object.
    */
    using Ptr = QSharedPointer<ResourceItem>;

    ResourceItem(const KLDAP::LdapDN &dn, const QStringList &attrs, const KLDAP::LdapClient &ldapClient, const ResourceItem::Ptr &parent = ResourceItem::Ptr());
    ~ResourceItem() override;

    Q_REQUIRED_RESULT ResourceItem::Ptr child(int number);
    Q_REQUIRED_RESULT int childCount() const;
    Q_REQUIRED_RESULT int columnCount() const;
    Q_REQUIRED_RESULT QVariant data(int column) const;
    Q_REQUIRED_RESULT QVariant data(const QString &column) const;
    Q_REQUIRED_RESULT bool insertChild(int position, const ResourceItem::Ptr &item);
    Q_REQUIRED_RESULT ResourceItem::Ptr parent();
    Q_REQUIRED_RESULT bool removeChildren(int position, int count);
    Q_REQUIRED_RESULT int childNumber() const;

private:
    QList<ResourceItem::Ptr> childItems;
    QVector<QVariant> itemData;
    ResourceItem::Ptr parentItem;

Q_SIGNALS:
    void searchFinished();

public:
    /* Returns the attributes of the requested ldapObject.
     *
     */
    const QStringList &attributes() const;

    /* Returns the ldapObject, that is used as data source.
     *
     */
    const KLDAP::LdapObject &ldapObject() const;

    /* Set the ldapObject, either directly via this function
     * or use startSearch to request the ldapServer for the ldapObject
     * with the dn specified via the constructor.
     *
     */
    void setLdapObject(const KLDAP::LdapObject &);

    /* The used ldapClient.
     *
     */
    const KLDAP::LdapClient &ldapClient() const;

    /* Start querying the ldapServer for a object that name is dn
     *
     */
    void startSearch();

private:
    /* data source
     *
     */
    KLDAP::LdapObject mLdapObject;

    /* dn of the ldapObject
     *
     */
    const KLDAP::LdapDN dn;

    /* Attributes of the ldapObject to request and the header of the Item
     *
     */
    QStringList mAttrs;

    /* ldapClient to request
     *
     */
    KLDAP::LdapClient mLdapClient;

private:
    /* Answer of the LdapServer for the given dn
     *
     */
    void slotLDAPResult(const KLDAP::LdapClient &, const KLDAP::LdapObject &);
};
}

//@cond PRIVATE
Q_DECLARE_TYPEINFO(IncidenceEditorNG::ResourceItem::Ptr, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(IncidenceEditorNG::ResourceItem::Ptr)
//@endcond
