/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "incidenceeditor-ng.h"
class QUrl;
class KJob;
namespace Ui
{
class EventOrTodoDesktop;
}

class QMenu;

class QListWidgetItem;
class QMimeData;
class QAction;
namespace IncidenceEditorNG
{
class AttachmentIconView;

class IncidenceAttachment : public IncidenceEditor
{
    Q_OBJECT
public:
    using IncidenceEditorNG::IncidenceEditor::load; // So we don't trigger -Woverloaded-virtual
    using IncidenceEditorNG::IncidenceEditor::save; // So we don't trigger -Woverloaded-virtual

    explicit IncidenceAttachment(Ui::EventOrTodoDesktop *ui);

    ~IncidenceAttachment() override;

    void load(const KCalendarCore::Incidence::Ptr &incidence) override;
    void save(const KCalendarCore::Incidence::Ptr &incidence) override;
    Q_REQUIRED_RESULT bool isDirty() const override;

    Q_REQUIRED_RESULT int attachmentCount() const;

Q_SIGNALS:
    void attachmentCountChanged(int newCount);

private:
    void addAttachment();
    void copyToClipboard(); /// Copies selected items to clip board
    void cutToClipboard(); /// Copies selected items to clipboard and removes them from the list
    void editSelectedAttachments();
    void openURL(const QUrl &url);
    void pasteFromClipboard();
    void removeSelectedAttachments();
    void saveAttachment(QListWidgetItem *item);
    void saveSelectedAttachments();
    void showAttachment(QListWidgetItem *item);
    void showContextMenu(const QPoint &pos);
    void showSelectedAttachments();
    void slotItemRenamed(QListWidgetItem *item);
    void slotSelectionChanged();
    void downloadComplete(KJob *);

private:
    //     void addAttachment( KCalendarCore::Attachment *attachment );
    void addDataAttachment(const QByteArray &data, const QString &mimeType = QString(), const QString &label = QString());
    void addUriAttachment(const QString &uri, const QString &mimeType = QString(), const QString &label = QString(), bool inLine = false);
    void handlePasteOrDrop(const QMimeData *mimeData);
    void setupActions();
    void setupAttachmentIconView();

private:
    AttachmentIconView *mAttachmentView = nullptr;
    Ui::EventOrTodoDesktop *const mUi;

    QMenu *mPopupMenu = nullptr;
    QAction *mOpenAction = nullptr;
    QAction *mSaveAsAction = nullptr;
#ifndef QT_NO_CLIPBOARD
    QAction *mCopyAction = nullptr;
    QAction *mCutAction = nullptr;
#endif
    QAction *mDeleteAction = nullptr;
    QAction *mEditAction = nullptr;
};
}

