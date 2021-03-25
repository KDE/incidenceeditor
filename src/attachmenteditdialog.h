/*
  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2005 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <KCalendarCore/Attachment>
#include <QDialog>
#include <QMimeType>
#include <QUrl>

class QPushButton;

namespace Ui
{
class AttachmentEditDialog;
}

namespace IncidenceEditorNG
{
class AttachmentIconItem;

class AttachmentEditDialog : public QDialog
{
    Q_OBJECT
public:
    AttachmentEditDialog(AttachmentIconItem *item, QWidget *parent, bool modal = true);
    ~AttachmentEditDialog() override;
    void accept() override;

protected Q_SLOTS:
    void inlineChanged(int state);
    void urlChanged(const QUrl &url);
    void urlChanged(const QString &url);
    virtual void slotApply();

private:
    KCalendarCore::Attachment mAttachment;
    AttachmentIconItem *mItem = nullptr;
    QMimeType mMimeType;
    Ui::AttachmentEditDialog *const mUi;
    QPushButton *mOkButton = nullptr;
};
}

