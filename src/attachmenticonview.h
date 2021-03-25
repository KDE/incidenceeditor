/*
  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2005 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0

  NOTE: May, 2010. Extracted this code from
        kdepim/incidenceeditors/editorattachments.{h,cpp}
*/

#pragma once

#include <KCalendarCore/Attachment>

#include <QMimeType>
#include <QUrl>

#include <QListWidget>

namespace IncidenceEditorNG
{
class AttachmentIconView : public QListWidget
{
    Q_OBJECT
    friend class EditorAttachments;

public:
    explicit AttachmentIconView(QWidget *parent = nullptr);

    Q_REQUIRED_RESULT QMimeData *mimeData() const;

protected:
    QMimeData *mimeData(const QList<QListWidgetItem *> items) const override;
    void startDrag(Qt::DropActions supportedActions) override;
    void keyPressEvent(QKeyEvent *event) override;
};

class AttachmentIconItem : public QListWidgetItem
{
public:
    AttachmentIconItem(const KCalendarCore::Attachment &att, QListWidget *parent);
    ~AttachmentIconItem() override;

    Q_REQUIRED_RESULT KCalendarCore::Attachment attachment() const;
    Q_REQUIRED_RESULT const QString uri() const;
    Q_REQUIRED_RESULT const QString savedUri() const;
    void setUri(const QString &uri);

    using QListWidgetItem::setData;

    void setData(const QByteArray &data);

    Q_REQUIRED_RESULT const QString mimeType() const;
    void setMimeType(const QString &mime);

    Q_REQUIRED_RESULT const QString label() const;
    void setLabel(const QString &description);

    Q_REQUIRED_RESULT bool isBinary() const;

    static QPixmap icon(const QMimeType &mimeType, const QString &uri, bool binary = false);
    Q_REQUIRED_RESULT QPixmap icon() const;

    void readAttachment();

    Q_REQUIRED_RESULT QUrl tempFileForAttachment();

private:
    KCalendarCore::Attachment mAttachment;
    QString mSaveUri;
    QUrl mTempFile;
};
}

