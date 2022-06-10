/*
  SPDX-FileCopyrightText: 2007 Bruno Virlet <bruno.virlet@gmail.com>
  SPDX-FileCopyrightText: 2008-2009, 2013 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ktimezonecombobox.h"

#include <KLocalizedString>

#include <QVector>

using namespace IncidenceEditorNG;

class IncidenceEditorNG::KTimeZoneComboBoxPrivate
{
public:
    KTimeZoneComboBoxPrivate(KTimeZoneComboBox *parent)
        : mParent(parent)
    {
    }

    void fillComboBox();
    KTimeZoneComboBox *const mParent;
    QVector<QByteArray> mZones;
};

void KTimeZoneComboBoxPrivate::fillComboBox()
{
    mParent->clear();
    mZones.clear();

    // Read all known time zones.
    const QList<QByteArray> lstTimeZoneIds = QTimeZone::availableTimeZoneIds();
    mZones.reserve(lstTimeZoneIds.count() + 3);

    // Prepend the system time zone, UTC and Floating, for convenience.
    mZones.append(QTimeZone::systemTimeZoneId());
    mZones.append("Floating");
    mZones.append("UTC");
    const auto sortStart = mZones.end();

    std::copy(lstTimeZoneIds.begin(), lstTimeZoneIds.end(), std::back_inserter(mZones));
    std::sort(sortStart, mZones.end()); // clazy:exclude=detaching-member

    // Put translated zones into the combobox
    for (const auto &z : std::as_const(mZones)) {
        mParent->addItem(i18n(z.constData()).replace(QLatin1Char('_'), QLatin1Char(' ')));
    }
}

KTimeZoneComboBox::KTimeZoneComboBox(QWidget *parent)
    : QComboBox(parent)
    , d(new KTimeZoneComboBoxPrivate(this))
{
    d->fillComboBox();
}

KTimeZoneComboBox::~KTimeZoneComboBox() = default;

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

void KTimeZoneComboBox::selectTimeZoneFor(const QDateTime &dateTime)
{
    if (dateTime.timeSpec() == Qt::LocalTime)
        setCurrentIndex(1); // Floating
    else
        selectTimeZone(dateTime.timeZone());
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

void KTimeZoneComboBox::applyTimeZoneTo(QDateTime &dt) const
{
    if (isFloating()) {
        dt.setTimeSpec(Qt::LocalTime);
    } else {
        dt.setTimeZone(selectedTimeZone());
    }
}

bool KTimeZoneComboBox::isFloating() const
{
    return currentIndex() == 1;
}
