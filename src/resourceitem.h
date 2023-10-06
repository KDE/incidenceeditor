/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#pragma once

#include <KLDAPWidgets/LdapClient>

#include <KLDAPCore/LdapObject>

#include <QList>
#include <QSharedPointer>
#include <QStringList>
#include <QVariant>

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

    ResourceItem(const KLDAPCore::LdapDN &dn,
                 const QStringList &attrs,
                 const KLDAPWidgets::LdapClient &ldapClient,
                 const ResourceItem::Ptr &parent = ResourceItem::Ptr());
    ~ResourceItem() override;

    [[nodiscard]] ResourceItem::Ptr child(int number);
    [[nodiscard]] int childCount() const;
    [[nodiscard]] int columnCount() const;
    [[nodiscard]] QVariant data(int column) const;
    [[nodiscard]] QVariant data(const QString &column) const;
    [[nodiscard]] bool insertChild(int position, const ResourceItem::Ptr &item);
    [[nodiscard]] ResourceItem::Ptr parent();
    [[nodiscard]] bool removeChildren(int position, int count);
    [[nodiscard]] int childNumber() const;

private:
    QList<ResourceItem::Ptr> childItems;
    QList<QVariant> itemData;
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
    const KLDAPCore::LdapObject &ldapObject() const;

    /* Set the ldapObject, either directly via this function
     * or use startSearch to request the ldapServer for the ldapObject
     * with the dn specified via the constructor.
     *
     */
    void setLdapObject(const KLDAPCore::LdapObject &);

    /* The used ldapClient.
     *
     */
    const KLDAPWidgets::LdapClient &ldapClient() const;

    /* Start querying the ldapServer for a object that name is dn
     *
     */
    void startSearch();

private:
    /* data source
     *
     */
    KLDAPCore::LdapObject mLdapObject;

    /* dn of the ldapObject
     *
     */
    const KLDAPCore::LdapDN dn;

    /* Attributes of the ldapObject to request and the header of the Item
     *
     */
    QStringList mAttrs;

    /* ldapClient to request
     *
     */
    KLDAPWidgets::LdapClient mLdapClient;

private:
    /* Answer of the LdapServer for the given dn
     *
     */
    void slotLDAPResult(const KLDAPWidgets::LdapClient &, const KLDAPCore::LdapObject &);
};
}

//@cond PRIVATE
Q_DECLARE_TYPEINFO(IncidenceEditorNG::ResourceItem::Ptr, Q_RELOCATABLE_TYPE);
Q_DECLARE_METATYPE(IncidenceEditorNG::ResourceItem::Ptr)
//@endcond
