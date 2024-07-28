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

    [[nodiscard]] QMimeData *mimeData() const;

protected:
    QMimeData *mimeData(const QList<QListWidgetItem *> &items) const override;
    void startDrag(Qt::DropActions supportedActions) override;
    void keyPressEvent(QKeyEvent *event) override;
};

class AttachmentIconItem : public QListWidgetItem
{
public:
    AttachmentIconItem(const KCalendarCore::Attachment &att, QListWidget *parent);
    ~AttachmentIconItem() override;

    [[nodiscard]] KCalendarCore::Attachment attachment() const;
    [[nodiscard]] const QString uri() const;
    [[nodiscard]] const QString savedUri() const;
    void setUri(const QString &uri);

    using QListWidgetItem::setData;

    void setData(const QByteArray &data);

    [[nodiscard]] const QString mimeType() const;
    void setMimeType(const QString &mime);

    [[nodiscard]] const QString label() const;
    void setLabel(const QString &description);

    [[nodiscard]] bool isBinary() const;

    [[nodiscard]] static QIcon icon(const QMimeType &mimeType, const QString &uri, bool binary = false);
    [[nodiscard]] QIcon icon() const;

    void readAttachment();

    [[nodiscard]] QUrl tempFileForAttachment();

private:
    KCalendarCore::Attachment mAttachment;
    QString mSaveUri;
    QUrl mTempFile;
};
}
