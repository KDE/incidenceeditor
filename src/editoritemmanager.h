/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Calendar/IncidenceChanger>
#include <Collection>
#include <QObject>

namespace Akonadi
{
class Item;
}

class KJob;

namespace IncidenceEditorNG
{
class ItemEditorUi;
class ItemEditorPrivate;

/**
 * Helper class for creating dialogs that let the user create and edit the payload
 * of Akonadi items (e.g. events, contacts, etc). This class supports editing of
 * one item at a time and handles all Akonadi specific logic like Item creation,
 * Item modifying and monitoring of changes to the item during editing.
 */
// template <typename PayloadT>
class EditorItemManager : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates an ItemEditor for a new Item.
     * Receives an option IncidenceChanger, so you can share the undo/redo stack with your
     * application.
     */
    explicit EditorItemManager(ItemEditorUi *ui, Akonadi::IncidenceChanger *changer = nullptr);

    /**
     * Destructs the ItemEditor. Unsaved changes will get lost at this point.
     */
    ~EditorItemManager() override;

    enum ItemState {
        AfterSave, /**< Returns the last saved item */
        BeforeSave /**< Returns an item with the original payload before the last save call */
    };

    /**
     * Returns the last saved item with payload or an invalid item when save is
     * not called yet.
     */
    Q_REQUIRED_RESULT Akonadi::Item item(ItemState state = AfterSave) const;

    /**
     * Loads the @param item into the editor. The item passed must be
     * a valid item.
     */
    void load(const Akonadi::Item &item);

    /**
     * Saves the new or modified item. This method does nothing when the
     * ui is not dirty.
     */
    void save();

    enum SaveAction {
        Create, /**< A new item was created */
        Modify, /**< An existing item was modified */
        None, /**< Nothing happened. */
        Move, /**< An existing item was moved to another collection */
        MoveAndModify /**< An existing item was moved to another collection and modified */
    };

    void setIsCounterProposal(bool isCounterProposal);

Q_SIGNALS:
    void itemSaveFinished(IncidenceEditorNG::EditorItemManager::SaveAction action);

    void itemSaveFailed(IncidenceEditorNG::EditorItemManager::SaveAction action, const QString &message);

    void revertFinished();
    void revertFailed(const QString &message);

private:
    ItemEditorPrivate *const d_ptr;
    Q_DECLARE_PRIVATE(ItemEditor)
    Q_DISABLE_COPY(EditorItemManager)

    Q_PRIVATE_SLOT(d_ptr, void itemChanged(const Akonadi::Item &, const QSet<QByteArray> &))
    Q_PRIVATE_SLOT(d_ptr, void itemFetchResult(KJob *))
    Q_PRIVATE_SLOT(d_ptr, void itemMoveResult(KJob *))
    Q_PRIVATE_SLOT(d_ptr,
                   void onModifyFinished(int changeId, const Akonadi::Item &item, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorString))
    Q_PRIVATE_SLOT(d_ptr,
                   void onCreateFinished(int changeId, const Akonadi::Item &item, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorString))
    Q_PRIVATE_SLOT(d_ptr, void moveJobFinished(KJob *job))
};

class ItemEditorUi
{
public:
    enum RejectReason {
        ItemFetchFailed, ///> Either the fetchjob failed or no items where returned
        ItemHasInvalidPayload, ///> The fetched item has an invalid payload
        ItemMoveFailed ///> Item move failed
    };

    virtual ~ItemEditorUi();

    /**
     * Returns whether or not the identifier set contains payload identifiers that
     * are displayed/editable in the Gui.
     */
    virtual bool containsPayloadIdentifiers(const QSet<QByteArray> &partIdentifiers) const = 0;

    /**
     * Returns whether or not @param item has a payload type that is supported by
     * the gui.
     */
    virtual bool hasSupportedPayload(const Akonadi::Item &item) const = 0;

    /**
     * Returns whether or not the values in the ui differ from the original (i.e.
     * either an empty or a loaded item). This method <em>only</em> involves
     * payload fields. I.e. if only the collection in which the item should be
     * stored has changed, this method should return false.
     */
    virtual bool isDirty() const = 0;

    /**
     * Returns whether or not the values in the ui are valid. This method can also
     * be used to update the ui if necessary. The default implementation returns
     * true, so if the ui doesn't need validation there is no need to reimplement
     * this method.
     */
    virtual bool isValid() const;

    /**
     * Fills the ui with the values of the payload of @param item. The item is
     * guaranteed to have a payload.
     */
    virtual void load(const Akonadi::Item &item) = 0;

    /**
     * Stores the values of the ui into the payload of @param item and returns the
     * item with an updated payload. The returned item must have a valid mimetype
     * too.
     */
    virtual Akonadi::Item save(const Akonadi::Item &item) = 0;

    /**
     * Returns the currently selected collection in which the item will be stored.
     */
    virtual Akonadi::Collection selectedCollection() const = 0;

    /**
     * This function is called if for some reason the creation or editing of the
     * item cannot be continued. The implementing class must abort editing at
     * this point.
     */
    virtual void reject(RejectReason reason, const QString &errorMessage = QString()) = 0;
};
}

