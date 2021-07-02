/*
  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2005 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0

  NOTE: May, 2010. Extracted this code from
        kdepim/incidenceeditors/editorattachments.{h,cpp}
*/

#include <config-enterprise.h>

#include "attachmenticonview.h"

#include <KIconLoader>
#include <KUrlMimeData>
#include <QDir>
#include <QTemporaryFile>

#include <KIconEngine>
#include <QDrag>
#include <QKeyEvent>
#include <QMimeData>
#include <QMimeDatabase>

using namespace IncidenceEditorNG;

AttachmentIconItem::AttachmentIconItem(const KCalendarCore::Attachment &att, QListWidget *parent)
    : QListWidgetItem(parent)
{
    if (!att.isEmpty()) {
        mAttachment = att;
    } else {
        // for the enterprise, inline attachments are the default
#if KDEPIM_ENTERPRISE_BUILD
        mAttachment = KCalendarCore::Attachment(QByteArray()); // use the non-uri constructor
        // as we want inline by default
#else
        mAttachment = KCalendarCore::Attachment(QString());
#endif
    }
    readAttachment();
    setFlags(flags() | Qt::ItemIsDragEnabled);
}

AttachmentIconItem::~AttachmentIconItem()
{
}

KCalendarCore::Attachment AttachmentIconItem::attachment() const
{
    return mAttachment;
}

const QString AttachmentIconItem::uri() const
{
    return mAttachment.uri();
}

const QString AttachmentIconItem::savedUri() const
{
    return mSaveUri;
}

void AttachmentIconItem::setUri(const QString &uri)
{
    mSaveUri = uri;
    mAttachment.setUri(mSaveUri);
    readAttachment();
}

void AttachmentIconItem::setData(const QByteArray &data)
{
    mAttachment.setDecodedData(data);
    readAttachment();
}

const QString AttachmentIconItem::mimeType() const
{
    return mAttachment.mimeType();
}

void AttachmentIconItem::setMimeType(const QString &mime)
{
    mAttachment.setMimeType(mime);
    readAttachment();
}

const QString AttachmentIconItem::label() const
{
    return mAttachment.label();
}

void AttachmentIconItem::setLabel(const QString &description)
{
    if (mAttachment.label() == description) {
        return;
    }
    mAttachment.setLabel(description);
    readAttachment();
}

bool AttachmentIconItem::isBinary() const
{
    return mAttachment.isBinary();
}

QPixmap AttachmentIconItem::icon() const
{
    QMimeDatabase db;
    return icon(db.mimeTypeForName(mAttachment.mimeType()), mAttachment.uri(), mAttachment.isBinary());
}

QPixmap AttachmentIconItem::icon(const QMimeType &mimeType, const QString &uri, bool binary)
{
    const QString iconStr = mimeType.iconName();
    QStringList overlays;
    if (!uri.isEmpty() && !binary) {
        overlays << QStringLiteral("emblem-link");
    }
    return QIcon(new KIconEngine(iconStr, KIconLoader::global(), overlays)).pixmap(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
}

void AttachmentIconItem::readAttachment()
{
    setText(mAttachment.label());
    setFlags(flags() | Qt::ItemIsEditable);

    QMimeDatabase db;
    if (mAttachment.mimeType().isEmpty() || !(db.mimeTypeForName(mAttachment.mimeType()).isValid())) {
        QMimeType mimeType;
        if (mAttachment.isUri()) {
            mimeType = db.mimeTypeForUrl(QUrl(mAttachment.uri()));
        } else {
            mimeType = db.mimeTypeForData(mAttachment.decodedData());
        }
        mAttachment.setMimeType(mimeType.name());
    }

    setIcon(icon());
}

AttachmentIconView::AttachmentIconView(QWidget *parent)
    : QListWidget(parent)
{
    setMovement(Static);
    setAcceptDrops(true);
    setSelectionMode(ExtendedSelection);
    setSelectionRectVisible(false);
    setIconSize(QSize(KIconLoader::SizeLarge, KIconLoader::SizeLarge));
    setFlow(LeftToRight);
    setWrapping(true);
#ifndef QT_NO_DRAGANDDROP
    setDragDropMode(DragDrop);
    setDragEnabled(true);
#endif
    setEditTriggers(EditKeyPressed);
    setContextMenuPolicy(Qt::CustomContextMenu);
}

QUrl AttachmentIconItem::tempFileForAttachment()
{
    if (mTempFile.isValid()) {
        return mTempFile;
    }
    QTemporaryFile *file = nullptr;

    QMimeDatabase db;
    QStringList patterns = db.mimeTypeForName(mAttachment.mimeType()).globPatterns();

    if (!patterns.empty()) {
        file = new QTemporaryFile(QDir::tempPath() + QLatin1String("/attachementview_XXXXX") + patterns.first().remove(QLatin1Char('*')));
    } else {
        file = new QTemporaryFile();
    }
    file->setParent(listWidget());

    file->setAutoRemove(true);
    file->open();
    // read-only not to give the idea that it could be written to
    file->setPermissions(QFile::ReadUser);
    file->write(QByteArray::fromBase64(mAttachment.data()));
    mTempFile = QUrl::fromLocalFile(file->fileName());
    file->close();
    return mTempFile;
}

QMimeData *AttachmentIconView::mimeData(const QList<QListWidgetItem *> items) const // clazy:exclude=function-args-by-ref
{
    // create a list of the URL:s that we want to drag
    QList<QUrl> urls;
    QStringList labels;
    for (QListWidgetItem *it : items) {
        if (it->isSelected()) {
            auto item = static_cast<AttachmentIconItem *>(it);
            if (item->isBinary()) {
                urls.append(item->tempFileForAttachment());
            } else {
                urls.append(QUrl(item->uri()));
            }
            labels.append(QString::fromLatin1(QUrl::toPercentEncoding(item->label())));
        }
    }
    if (selectionMode() == NoSelection) {
        auto item = static_cast<AttachmentIconItem *>(currentItem());
        if (item) {
            urls.append(QUrl(item->uri()));
            labels.append(QString::fromLatin1(QUrl::toPercentEncoding(item->label())));
        }
    }

    QMap<QString, QString> metadata;
    metadata[QStringLiteral("labels")] = labels.join(QLatin1Char(':'));

    auto mimeData = new QMimeData;
    mimeData->setUrls(urls);
    KUrlMimeData::setMetaData(metadata, mimeData);
    return mimeData;
}

QMimeData *AttachmentIconView::mimeData() const
{
    return mimeData(selectedItems());
}

void AttachmentIconView::startDrag(Qt::DropActions supportedActions)
{
    Q_UNUSED(supportedActions)
#ifndef QT_NO_DRAGANDDROP
    QPixmap pixmap;
    if (selectedItems().size() > 1) {
        pixmap = KIconLoader::global()->loadIcon(QStringLiteral("mail-attachment"), KIconLoader::Desktop);
    }
    if (pixmap.isNull()) {
        pixmap = static_cast<AttachmentIconItem *>(currentItem())->icon();
    }

    const QPoint hotspot(pixmap.width() / 2, pixmap.height() / 2);

    auto drag = new QDrag(this);
    drag->setMimeData(mimeData());

    drag->setPixmap(pixmap);
    drag->setHotSpot(hotspot);
    drag->exec(Qt::CopyAction);
#endif
}

void AttachmentIconView::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && currentItem() && state() != EditingState) {
        Q_EMIT itemDoubleClicked(currentItem()); // ugly, but itemActivated() also includes single click
        return;
    }
    QListWidget::keyPressEvent(event);
}
