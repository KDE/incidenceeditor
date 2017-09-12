/*
  Copyright (C) 2007 Bruno Virlet <bruno.virlet@gmail.com>
  Copyright 2008-2009,2013 Allen Winter <winter@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "ktimezonecombobox.h"

#include <KLocalizedString>

#include <QTimeZone>
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
    std::sort(mZones.begin(), mZones.end());

    // Prepend Local, UTC and Floating, for convenience
    mZones.prepend("UTC");        // do not use i18n here  index=2
    mZones.prepend("Floating");   // do not use i18n here  index=1
    mZones.prepend(QTimeZone::systemTimeZoneId());    // index=0

    // Put translated zones into the combobox
    for (const auto &z : qAsConst(mZones)) {
        mParent->addItem(i18n(z.constData()).replace(QLatin1Char('_'), QLatin1Char(' ')));
    }
}

KTimeZoneComboBox::KTimeZoneComboBox(QWidget *parent)
    : KComboBox(parent)
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
    for (const auto &z : qAsConst(d->mZones)) {
        if (z == zone.id()) {
            nCurrentlySet = i;
            break;
        }
        ++i;
    }

    if (nCurrentlySet == -1) {
        if (zone == QTimeZone::utc()) {
            setCurrentIndex(2);   // UTC
        } else if (zone == QTimeZone::systemTimeZone()) {
            setCurrentIndex(0);   // Local
        } else {
            setCurrentIndex(1);   // Floating event
        }
    } else {
        setCurrentIndex(nCurrentlySet);
    }
}

QTimeZone KTimeZoneComboBox::selectedTimeZone() const
{
    QTimeZone zone;
    if (currentIndex() >= 0) {
        if (currentIndex() == 0) {   // Local
            zone = QTimeZone::systemTimeZone();
        } else if (currentIndex() == 1) {   // Floating event
            zone = QTimeZone();
        } else if (currentIndex() == 2) {   // UTC
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
        selectTimeZone(QTimeZone());
    } else {
        if (zone.isValid()) {
            selectTimeZone(zone);
        } else {
            selectLocalTimeZone();
        }
    }
}
