/*
  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2005 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "attachmenteditdialog.h"
using namespace Qt::Literals::StringLiterals;

#include "attachmenticonview.h"
#include "config-enterprise.h"
#include "ui_attachmenteditdialog.h"
#include <KFormat>
#include <KIO/StoredTransferJob>
#include <KIconLoader>
#include <KJobWidgets>
#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QLocale>
#include <QMimeDatabase>
#include <QPushButton>

using namespace IncidenceEditorNG;

AttachmentEditDialog::AttachmentEditDialog(AttachmentIconItem *item, QWidget *parent, bool modal)
    : QDialog(parent)
    , mItem(item)
    , mUi(new Ui::AttachmentEditDialog)
{
    setWindowTitle(i18nc("@title:window", "Edit Attachment"));
    QMimeDatabase const db;
    mMimeType = db.mimeTypeForName(item->mimeType());
    auto page = new QWidget(this);
    auto mainLayout = new QVBoxLayout(this);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    mOkButton->setDefault(true);
    mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AttachmentEditDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AttachmentEditDialog::reject);
    mainLayout->addWidget(page);
    mainLayout->addWidget(buttonBox);

    mUi->setupUi(page);
    mUi->mLabelEdit->setText(item->label().isEmpty() ? item->uri() : item->label());
    mUi->mIcon->setPixmap(item->icon().pixmap(KIconLoader::SizeSmallMedium));
    mUi->mInlineCheck->setChecked(item->isBinary());

    const QString typecomment = item->mimeType().isEmpty() ? i18nc("@label unknown mimetype", "Unknown") : mMimeType.comment();
    mUi->mTypeLabel->setText(typecomment);

    setModal(modal);
    mOkButton->setEnabled(false);

    mUi->mInlineCheck->setEnabled(false);
    if (item->attachment().isUri() || item->attachment().data().isEmpty()) {
        mUi->mStackedWidget->setCurrentIndex(0);
        mUi->mURLRequester->setUrl(QUrl(item->uri()));
        urlChanged(item->uri());
    } else {
        mUi->mInlineCheck->setEnabled(true);
        mUi->mStackedWidget->setCurrentIndex(1);
        KFormat const format;
        mUi->mSizeLabel->setText(u"%1 (%2)"_s.arg(format.formatByteSize(item->attachment().size()), QLocale().toString(item->attachment().size())));
    }

    connect(mUi->mInlineCheck, &QCheckBox::checkStateChanged, this, &AttachmentEditDialog::inlineChanged);
    connect(mUi->mURLRequester,
            &KUrlRequester::urlSelected,
            this,
            static_cast<void (AttachmentEditDialog::*)(const QUrl &)>(&AttachmentEditDialog::urlChanged));
    connect(mUi->mURLRequester,
            &KUrlRequester::textChanged,
            this,
            static_cast<void (AttachmentEditDialog::*)(const QString &)>(&AttachmentEditDialog::urlChanged));
}

AttachmentEditDialog::~AttachmentEditDialog()
{
    delete mUi;
}

void AttachmentEditDialog::accept()
{
    slotApply();
    QDialog::accept();
}

void AttachmentEditDialog::slotApply()
{
    QUrl url = mUi->mURLRequester->url();

    if (mUi->mLabelEdit->text().isEmpty()) {
        if (url.isLocalFile()) {
            mItem->setLabel(url.fileName());
        } else {
            mItem->setLabel(url.url());
        }
    } else {
        mItem->setLabel(mUi->mLabelEdit->text());
    }
    if (mItem->label().isEmpty()) {
        mItem->setLabel(i18nc("@label", "New attachment"));
    }
    mItem->setMimeType(mMimeType.name());

    QString correctedUrl = url.url();
    if (!url.isEmpty() && url.isRelative()) {
        // If the user used KURLRequester's KURLCompletion
        // (used the line edit instead of the file dialog)
        // the returned url is not absolute and is always relative
        // to the home directory (not pwd), so we must prepend home

        correctedUrl = QDir::home().filePath(url.toLocalFile());
        url = QUrl::fromLocalFile(correctedUrl);
        if (url.isValid()) {
            urlChanged(url);
            mItem->setLabel(url.fileName());
            mItem->setUri(correctedUrl);
            mItem->setMimeType(mMimeType.name());
        }
    }

    if (mUi->mStackedWidget->currentIndex() == 0) {
        if (mUi->mInlineCheck->isChecked()) {
            auto job = KIO::storedGet(url);
            KJobWidgets::setWindow(job, nullptr);
            if (job->exec()) {
                QByteArray const data = job->data();
                mItem->setData(data);
            }
        } else {
            mItem->setUri(correctedUrl);
        }
    }
}

void AttachmentEditDialog::inlineChanged(int state)
{
    mOkButton->setEnabled(!mUi->mURLRequester->url().toDisplayString().trimmed().isEmpty() || mUi->mStackedWidget->currentIndex() == 1);
    if (state == Qt::Unchecked && mUi->mStackedWidget->currentIndex() == 1) {
        mUi->mStackedWidget->setCurrentIndex(0);
        if (!mItem->savedUri().isEmpty()) {
            mUi->mURLRequester->setUrl(QUrl(mItem->savedUri()));
        } else {
            mUi->mURLRequester->setUrl(QUrl(mItem->uri()));
        }
    }
}

void AttachmentEditDialog::urlChanged(const QString &url)
{
    const bool urlIsNotEmpty = !url.trimmed().isEmpty();
    mOkButton->setEnabled(urlIsNotEmpty);
    mUi->mInlineCheck->setEnabled(urlIsNotEmpty || mUi->mStackedWidget->currentIndex() == 1);
}

void AttachmentEditDialog::urlChanged(const QUrl &url)
{
    QMimeDatabase const db;
    mMimeType = db.mimeTypeForUrl(url);
    mUi->mTypeLabel->setText(mMimeType.comment());
    mUi->mIcon->setPixmap(AttachmentIconItem::icon(mMimeType, url.path()).pixmap(KIconLoader::SizeSmallMedium));
}

#include "moc_attachmenteditdialog.cpp"
