/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

// clazy:excludeall=const-signal-or-slot

#pragma once

#include "editoritemmanager.h"
using namespace Qt::Literals::StringLiterals;

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
/*!
 * \class IncidenceEditorNG::IncidenceDialog
 * \inmodule IncidenceEditor
 * \inheaderfile IncidenceEditor/IncidenceDialog
 *
 * \brief The IncidenceDialog class
 */
class INCIDENCEEDITOR_EXPORT IncidenceDialog : public QDialog
{
    Q_OBJECT
public:
    /*!
     * Creates a new IncidenceDialog.
     * \a changer The IncidenceChanger to use for modifications (optional).
     * \a parent The parent widget.
     * \a flags The window flags.
     */
    explicit IncidenceDialog(Akonadi::IncidenceChanger *changer = nullptr, QWidget *parent = nullptr, Qt::WindowFlags flags = {});
    /*!
     * Destroys the IncidenceDialog.
     */
    ~IncidenceDialog() override;

    /*!
     * Loads the \a item into the dialog.
     *
     * To create a new Incidence pass an invalid item with either an
     * KCalendarCore::Event:Ptr or a KCalendarCore::Todo:Ptr set as payload. Note When the
     * item is invalid, i.e. it has an invalid id, a valid payload <em>must</em>
     * be set.
     *
     * When the item has is valid this method will fetch the payload when this is
     * not already set.
     * \a item The Akonadi item to load.
     * \a activeDate The date to use as the active date (optional).
     */
    virtual void load(const Akonadi::Item &item, const QDate &activeDate = QDate());

    /*!
     * Sets the Collection combobox to \a collection.
     * \a collection The collection to select.
     */
    virtual void selectCollection(const Akonadi::Collection &collection);

    /*!
     * Sets whether this dialog is displaying a counter proposal.
     * \a isCounterProposal True if this is a counter proposal.
     */
    virtual void setIsCounterProposal(bool isCounterProposal);

    /*!
      Returns the object that will receive all key events.
    */
    QObject *typeAheadReceiver() const;

    /*!
       By default, if you load an incidence into the editor ( load(item) ), then press [OK]
       without changing anything, the dialog is dismissed, and the incidence isn't saved
       to akonadi.

       Call this method with \a initiallyDirty = true if you want the incidence to be saved,
       It's useful if you're creating a dialog with an already crafted content, like in kmail's
       "Create Todo/Reminder Feature".
       \a initiallyDirty True if the dialog should be initially marked as modified.
    */
    void setInitiallyDirty(bool initiallyDirty);

    /*!
     * Returns the Akonadi item currently being edited.
     */
    [[nodiscard]] Akonadi::Item item() const;

Q_SIGNALS:
    /*!
     * This signal is emitted when an incidence is created.
     * \a item The Akonadi item that was created.
     */
    void incidenceCreated(const Akonadi::Item &);
    /*!
     * This signal is emitted when an invalid collection is selected.
     */
    void invalidCollection() const;

protected:
    /*!
     * Handles the close event of the dialog.
     */
    void closeEvent(QCloseEvent *event) override;

protected Q_SLOTS:
    /*!
     * Handles button clicks in the dialog.
     * \a button The button that was clicked.
     */
    void slotButtonClicked(QAbstractButton *button);
    /*!
     * Handles changes to the selected collection.
     * \a collection The newly selected collection.
     */
    void handleSelectedCollectionChange(const Akonadi::Collection &collection);
    /*!
     * Rejects and closes the dialog.
     */
    void reject() override;

private:
    std::unique_ptr<IncidenceDialogPrivate> const d_ptr;
    Q_DECLARE_PRIVATE(IncidenceDialog)
    Q_DISABLE_COPY(IncidenceDialog)

    INCIDENCEEDITOR_NO_EXPORT void writeConfig();
    INCIDENCEEDITOR_NO_EXPORT void readConfig();
};
}
