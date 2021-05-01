/*
  SPDX-FileCopyrightText: 2004 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "freebusyurldialog.h"

#include <KLineEdit>
#include <KLocalizedString>

#include "incidenceeditor_debug.h"
#include <KConfig>
#include <KConfigGroup>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QStandardPaths>
#include <QVBoxLayout>

using namespace IncidenceEditorNG;

FreeBusyUrlDialog::FreeBusyUrlDialog(const AttendeeData::Ptr &attendee, QWidget *parent)
    : QDialog(parent)
{
    setModal(true);
    setWindowTitle(i18nc("@title:window", "Edit Free/Busy Location"));
    auto mainLayout = new QVBoxLayout(this);

    auto topFrame = new QFrame(this);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(topFrame);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FreeBusyUrlDialog::reject);
    mainLayout->addWidget(buttonBox);
    okButton->setDefault(true);

    QBoxLayout *topLayout = new QVBoxLayout(topFrame);
    topLayout->setContentsMargins(0, 0, 0, 0);

    mWidget = new FreeBusyUrlWidget(attendee, topFrame);
    topLayout->addWidget(mWidget);

    mWidget->loadConfig();
    connect(okButton, &QPushButton::clicked, this, &FreeBusyUrlDialog::slotOk);
}

void FreeBusyUrlDialog::slotOk()
{
    mWidget->saveConfig();
    accept();
}

FreeBusyUrlWidget::FreeBusyUrlWidget(const AttendeeData::Ptr &attendee, QWidget *parent)
    : QWidget(parent)
    , mAttendee(attendee)
{
    QBoxLayout *topLayout = new QVBoxLayout(this);

    auto label = new QLabel(xi18n("Location of Free/Busy information for %1 <placeholder>%2</placeholder>:", mAttendee->name(), mAttendee->email()), this);
    topLayout->addWidget(label);

    mUrlEdit = new KLineEdit(this);
    mUrlEdit->setFocus();
    mUrlEdit->setWhatsThis(i18nc("@info:whatsthis", "Enter the location of the Free/Busy information for the attendee."));
    mUrlEdit->setToolTip(i18nc("@info:tooltip", "Enter the location of the information."));
    topLayout->addWidget(mUrlEdit);
}

FreeBusyUrlWidget::~FreeBusyUrlWidget()
{
}

static QString freeBusyUrlStore()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/korganizer/freebusyurls");
}

void FreeBusyUrlWidget::loadConfig()
{
    KConfig config(freeBusyUrlStore());
    mUrlEdit->setText(config.group(mAttendee->email()).readEntry("url"));
}

void FreeBusyUrlWidget::saveConfig()
{
    const QString url = mUrlEdit->text();
    KConfig config(freeBusyUrlStore());
    config.group(mAttendee->email()).writeEntry("url", url);
}
