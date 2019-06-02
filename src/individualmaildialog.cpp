/*
 * Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * As a special exception, permission is given to link this program
 * with any edition of Qt, and distribute the resulting executable,
 * without including the source code for Qt in the source distribution.
 */

#include "individualmaildialog.h"

#include <KGuiItem>
#include <KLocalizedString>

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>

using namespace IncidenceEditorNG;

IndividualMailDialog::IndividualMailDialog(const QString &question, const KCalCore::Attendee::List &attendees, const KGuiItem &buttonYes, const KGuiItem &buttonNo, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "Group Scheduling Email"));
    m_detailsWidget = new QWidget();
    QGridLayout *layout = new QGridLayout(m_detailsWidget);
    mAttendeeDecision.reserve(attendees.size());
    int row = 0;
    for (const KCalCore::Attendee::Ptr &attendee : attendees) {
        QComboBox *options = new QComboBox();
        options->addItem(i18nc("@item:inlistbox ITIP Messages for one attendee",
                               "Send update"), QVariant(Update));
        options->addItem(i18nc("@item:inlistbox ITIP Messages for one attendee",
                               "Send no update"), QVariant(NoUpdate));
        options->addItem(i18nc("@item:inlistbox ITIP Messages for one attendee",
                               "Edit mail"), QVariant(Edit));
        options->setWhatsThis(i18nc("@info:whatsthis",
                                    "Options for this particular attendee."));
        options->setToolTip(i18nc("@info:tooltip",
                                  "Choose an option for this attendee."));
        mAttendeeDecision.push_back(std::make_pair(attendee, options));

        layout->addWidget(new QLabel(attendee->fullName()), row, 0);
        layout->addWidget(options, row, 1);
        ++row;
    }
    QSizePolicy sizePolicy = m_detailsWidget->sizePolicy();
    sizePolicy.setHorizontalStretch(1);
    sizePolicy.setVerticalStretch(1);
    m_detailsWidget->setSizePolicy(sizePolicy);

    QWidget *mW = new QLabel(question);

    auto topLayout = new QVBoxLayout(this);
    topLayout->addWidget(mW);
    topLayout->addWidget(m_detailsWidget);

    m_buttons = new QDialogButtonBox(this);
    m_buttons->setStandardButtons(
        QDialogButtonBox::Yes | QDialogButtonBox::No | QDialogButtonBox::Help);
    auto yesButton = m_buttons->button(QDialogButtonBox::Yes);
    yesButton->setText(buttonYes.text());
    connect(yesButton, &QPushButton::clicked, this, [this]() {
        done(QDialogButtonBox::Yes);
    });
    auto noButton = m_buttons->button(QDialogButtonBox::No);
    noButton->setText(buttonNo.text());
    connect(noButton, &QPushButton::clicked, this, [this]() {
        done(QDialogButtonBox::No);
    });
    auto detailsButton = m_buttons->button(QDialogButtonBox::Help);
    detailsButton->setIcon(QIcon::fromTheme(QStringLiteral("help-about")));
    connect(detailsButton, &QPushButton::clicked, this, [this]() {
        m_detailsWidget->setVisible(!m_detailsWidget->isVisible());
        updateButtonState();
        adjustSize();
    });
    m_detailsWidget->setVisible(false);
    updateButtonState();

    topLayout->addWidget(m_buttons);
}

IndividualMailDialog::~IndividualMailDialog()
{
}

KCalCore::Attendee::List IndividualMailDialog::editAttendees() const
{
    KCalCore::Attendee::List edit;
    for (auto it = mAttendeeDecision.cbegin(), end = mAttendeeDecision.cend(); it != end; ++it) {
        const int index = (*it).second->currentIndex();
        if ((*it).second->itemData(index, Qt::UserRole) == Edit) {
            edit.append((*it).first);
        }
    }
    return edit;
}

KCalCore::Attendee::List IndividualMailDialog::updateAttendees() const
{
    KCalCore::Attendee::List update;
    for (auto it = mAttendeeDecision.cbegin(), end = mAttendeeDecision.cend(); it != end; ++it) {
        const int index = (*it).second->currentIndex();
        if ((*it).second->itemData(index, Qt::UserRole) == Update) {
            update.append((*it).first);
        }
    }
    return update;
}

void IndividualMailDialog::updateButtonState()
{
    auto detailsButton = m_buttons->button(QDialogButtonBox::Help);
    if (m_detailsWidget->isVisible()) {
        detailsButton->setText(i18nc("@action:button show list of attendees",
                                     "Individual mailsettings <<"));
    } else {
        detailsButton->setText(i18nc("@action:button show list of attendees",
                                     "Individual mailsettings >>"));
    }
}
