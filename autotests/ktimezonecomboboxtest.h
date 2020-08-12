/*
  SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef KTIMEZONECOMBOBOXTEST_H
#define KTIMEZONECOMBOBOXTEST_H

#include <QObject>

class KTimeZoneComboBoxTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test_timeSpec();
};

#endif
