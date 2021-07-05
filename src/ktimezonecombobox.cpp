/*
  SPDX-FileCopyrightText: 2007 Bruno Virlet <bruno.virlet@gmail.com>
  SPDX-FileCopyrightText: 2008-2009, 2013 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ktimezonecombobox.h"

#include <KLocalizedString>

#include <QVector>

using namespace IncidenceEditorNG;

class Q_DECL_HIDDEN KTimeZoneComboBox::Private
{
public:
    Private(KTimeZoneComboBox *parent)
        : mParent(parent)
    {
    }

    void fillComboBox();
    KTimeZoneComboBox *const mParent;
    QVector<QByteArray> mZones;
};

void KTimeZoneComboBox::Private::fillComboBox()
{
    mParent->clear();
    mZones.clear();

    // Read all system time zones
    const QList<QByteArray> lstTimeZoneIds = QTimeZone::availableTimeZoneIds();
    mZones.reserve(lstTimeZoneIds.count());
    std::copy(lstTimeZoneIds.begin(), lstTimeZoneIds.end(), std::back_inserter(mZones));
    std::sort(mZones.begin(), mZones.end());    // clazy:exclude=detaching-member

    // Prepend Local, UTC and Floating, for convenience
    mZones.prepend("UTC"); // do not use i18n here  index=2
    mZones.prepend("Floating"); // do not use i18n here  index=1
    mZones.prepend(QTimeZone::systemTimeZoneId()); // index=0

    // Put translated zones into the combobox
    for (const auto &z : std::as_const(mZones)) {
        mParent->addItem(i18n(z.constData()).replace(QLatin1Char('_'), QLatin1Char(' ')));
    }
}

KTimeZoneComboBox::KTimeZoneComboBox(QWidget *parent)
    : QComboBox(parent)
    , d(new KTimeZoneComboBox::Private(this))
{
    d->fillComboBox();
}

KTimeZoneComboBox::~KTimeZoneComboBox()
{
    delete d;
}

void KTimeZoneComboBox::selectTimeZone(const QTimeZone &zone)
{
    int nCurrentlySet = -1;

    int i = 0;
    for (const auto &z : std::as_const(d->mZones)) {
        if (z == zone.id()) {
            nCurrentlySet = i;
            break;
        }
        ++i;
    }

    if (nCurrentlySet == -1) {
        if (zone == QTimeZone::utc()) {
            setCurrentIndex(2); // UTC
        } else if (zone == QTimeZone::systemTimeZone()) {
            setCurrentIndex(0); // Local
        } else {
            setCurrentIndex(1); // Floating event
        }
    } else {
        setCurrentIndex(nCurrentlySet);
    }
}

QTimeZone KTimeZoneComboBox::selectedTimeZone() const
{
    QTimeZone zone;
    if (currentIndex() >= 0) {
        if (currentIndex() == 0) { // Local
            zone = QTimeZone::systemTimeZone();
        } else if (currentIndex() == 1) { // Floating event
            zone = QTimeZone::systemTimeZone();
        } else if (currentIndex() == 2) { // UTC
            zone = QTimeZone::utc();
        } else {
            zone = QTimeZone(d->mZones[currentIndex()]);
        }
    }

    return zone;
}

void KTimeZoneComboBox::selectLocalTimeZone()
{
    selectTimeZone(QTimeZone::systemTimeZone());
}

void KTimeZoneComboBox::setFloating(bool floating, const QTimeZone &zone)
{
    if (floating) {
        setCurrentIndex(1);
    } else {
        if (zone.isValid()) {
            selectTimeZone(zone);
        } else {
            selectLocalTimeZone();
        }
    }
}
