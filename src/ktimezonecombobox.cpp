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
#include "helper_p.h"
#include <KCalCore/ICalTimeZones>

#include <KLocalizedString>
#include <KTimeZone>
#include <KSystemTimeZones>

#include <QTimeZone>

using namespace IncidenceEditorNG;

class Q_DECL_HIDDEN KTimeZoneComboBox::Private
{
public:
    Private(KTimeZoneComboBox *parent)
        : mParent(parent), mAdditionalZones(0)
    {}

    void fillComboBox();
    KTimeZoneComboBox *const mParent;
    QStringList mZones;
    QVector<QByteArray> mAdditionalZones;
};

void KTimeZoneComboBox::Private::fillComboBox()
{
    mParent->clear();
    mZones.clear();

    // Read all system time zones
    const QList<QByteArray> lstTimeZoneIds = QTimeZone::availableTimeZoneIds();
    for (const QByteArray &id : lstTimeZoneIds) {
        mZones.push_back(QString::fromLatin1(id));
    }
    mZones.sort();

    // Prepend the list of additional timezones
    for (const QByteArray &id : qAsConst(mAdditionalZones))
        mZones.prepend(QString::fromLatin1(id));

    // Prepend Local, UTC and Floating, for convenience
    mZones.prepend(QStringLiteral("UTC"));        // do not use i18n here  index=2
    mZones.prepend(QStringLiteral("Floating"));   // do not use i18n here  index=1
    mZones.prepend(QString::fromLatin1(QTimeZone::systemTimeZoneId()));    // index=0

    // Put translated zones into the combobox
    for (const QString &z : qAsConst(mZones)) {
        mParent->addItem(i18n(z.toUtf8().constData()).replace(QLatin1Char('_'), QLatin1Char(' ')));
    }
}

KTimeZoneComboBox::KTimeZoneComboBox(QWidget *parent)
    : KComboBox(parent), d(new KTimeZoneComboBox::Private(this))
{
    d->fillComboBox();
}

void KTimeZoneComboBox::setAdditionalTimeZones(const QVector<QByteArray> &zones)
{
    d->mAdditionalZones = zones;
    d->fillComboBox();
}

KTimeZoneComboBox::~KTimeZoneComboBox()
{
    delete d;
}

void KTimeZoneComboBox::selectTimeSpec(const KDateTime::Spec &spec)
{
    int nCurrentlySet = -1;

    int i = 0;
    for (const QString &z : qAsConst(d->mZones)) {
        if (z == spec.timeZone().name()) {
            nCurrentlySet = i;
            break;
        }
        ++i;
    }

    if (nCurrentlySet == -1) {
        if (spec.isUtc()) {
            setCurrentIndex(2);   // UTC
        } else if (spec.isLocalZone()) {
            setCurrentIndex(0);   // Local
        } else {
            setCurrentIndex(1);   // Floating event
        }
    } else {
        setCurrentIndex(nCurrentlySet);
    }
}

KDateTime::Spec KTimeZoneComboBox::selectedTimeSpec() const
{
    KDateTime::Spec spec;
    if (currentIndex() >= 0) {
        if (currentIndex() == 0) {   // Local
            spec = KDateTime::Spec(KDateTime::LocalZone);
        } else if (currentIndex() == 1) {   // Floating event
            spec = KDateTime::Spec(KDateTime::ClockTime);
        } else if (currentIndex() == 2) {   // UTC
            spec.setType(KDateTime::UTC);
        } else {
            const KTimeZone systemTz = KSystemTimeZones::zone(d->mZones[currentIndex()]);
            // If it's not valid, then it's an additional Tz
            if (systemTz.isValid()) {
                spec.setType(systemTz);
            } else {
                KCalCore::ICalTimeZones zones;
                const KCalCore::ICalTimeZone additionalTz = zones.zone(d->mZones[currentIndex()]);
                spec.setType(additionalTz);
            }
        }
    }

    return spec;
}

void KTimeZoneComboBox::selectLocalTimeSpec()
{
    selectTimeSpec(KDateTime::Spec(KSystemTimeZones::local()));
}

void KTimeZoneComboBox::setFloating(bool floating, const KDateTime::Spec &spec)
{
    if (floating) {
        selectTimeSpec(KDateTime::Spec(KDateTime::ClockTime));
    } else {
        if (spec.isValid()) {
            selectTimeSpec(spec);
        } else {
            selectLocalTimeSpec();
        }
    }
}
