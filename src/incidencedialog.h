/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "editoritemmanager.h"
#include "incidenceeditor_export.h"

#include <QDate>
#include <QDialog>

#include <memory>

class QAbstractButton;

namespace Akonadi
{
class IncidenceChanger;
}

namespace IncidenceEditorNG
{
class IncidenceDialogPrivate;
/**
 * @brief The IncidenceDialog class
 */
class INCIDENCEEDITOR_EXPORT IncidenceDialog : public QDialog
{
    Q_OBJECT
public:
    explicit IncidenceDialog(Akonadi::IncidenceChanger *changer = nullptr, QWidget *parent = nullptr, Qt::WindowFlags flags = {});
    ~IncidenceDialog() override;

    /**
     * Loads the @param item into the dialog.
     *
     * To create a new Incidence pass an invalid item with either an
     * KCalendarCore::Event:Ptr or a KCalendarCore::Todo:Ptr set as payload. Note: When the
     * item is invalid, i.e. it has an invalid id, a valid payload <em>must</em>
     * be set.
     *
     * When the item has is valid this method will fetch the payload when this is
     * not already set.
     */
    virtual void load(const Akonadi::Item &item, const QDate &activeDate = QDate());

    /**
     * Sets the Collection combobox to @param collection.
     */
    virtual void selectCollection(const Akonadi::Collection &collection);

    virtual void setIsCounterProposal(bool isCounterProposal);

    /**
      Returns the object that will receive all key events.
    */
    QObject *typeAheadReceiver() const;

    /**
       By default, if you load an incidence into the editor ( load(item) ), then press [OK]
       without changing anything, the dialog is dismissed, and the incidence isn't saved
       to akonadi.

       Call this method with @p initiallyDirty = true if you want the incidence to be saved,
       It's useful if you're creating a dialog with an already crafted content, like in kmail's
       "Create Todo/Reminder Feature".
    */
    void setInitiallyDirty(bool initiallyDirty);

    Q_REQUIRED_RESULT Akonadi::Item item() const;

Q_SIGNALS:
    /**
     * This signal is emitted when an incidence is created.
     * @param collection The collection where it was created.
     */
    void incidenceCreated(const Akonadi::Item &);
    void invalidCollection() const;

protected:
    void closeEvent(QCloseEvent *event) override;

protected Q_SLOTS:
    void slotButtonClicked(QAbstractButton *button);
    void handleSelectedCollectionChange(const Akonadi::Collection &collection);
    void reject() override;

private:
    std::unique_ptr<IncidenceDialogPrivate> const d_ptr;
    Q_DECLARE_PRIVATE(IncidenceDialog)
    Q_DISABLE_COPY(IncidenceDialog)

    void writeConfig();
    void readConfig();
};
}
