/*
 *  SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "attendeetablemodel.h"

#include <KEmailAddress>

#include <KLocalizedString>

using namespace IncidenceEditorNG;

AttendeeTableModel::AttendeeTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int AttendeeTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    return mAttendeeList.count();
}

int AttendeeTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 8;
}

Qt::ItemFlags AttendeeTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::ItemIsEnabled;
    }
    if (index.column() == Available || index.column() == Name || index.column() == Email) { // Available is read only
        return QAbstractTableModel::flags(index);
    } else {
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    }
}

QVariant AttendeeTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (index.row() >= mAttendeeList.size()) {
        return {};
    }

    const KCalendarCore::Attendee attendee = mAttendeeList[index.row()];
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case Role:
            return attendee.role();
        case FullName:
            return attendee.fullName();
        case Available: {
            AvailableStatus const available = mAttendeeAvailable[index.row()];
            if (role == Qt::DisplayRole) {
                switch (available) {
                case Free:
                    return i18n("Free");
                case Busy:
                    return i18n("Busy");
                case Accepted:
                    return i18n("Accepted");
                case Unknown:
                default:
                    return i18n("Unknown");
                }
            } else {
                return available;
            }
        }
        case Status:
            return attendee.status();
        case CuType:
            return attendee.cuType();
        case Response:
            return attendee.RSVP();
        case Name:
            return attendee.name();
        case Email:
            return attendee.email();
        default:
            break;
        }
    }
    if (role == AttendeeRole) {
        return QVariant::fromValue(attendee);
    }
    return {};
}

bool AttendeeTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        QString email;
        QString name;
        KCalendarCore::Attendee &attendee = mAttendeeList[index.row()]; // clazy:exclude=detaching-member
        switch (index.column()) {
        case Role:
            attendee.setRole(static_cast<KCalendarCore::Attendee::Role>(value.toInt()));
            break;
        case FullName:
            if (mRemoveEmptyLines && value.toString().trimmed().isEmpty()) {
                // Do not remove last empty line if mKeepEmpty==true
                // (only works if initially there is only one empty line)
                if (!mKeepEmpty || !(attendee.name().isEmpty() && attendee.email().isEmpty())) {
                    removeRows(index.row(), 1);
                    return true;
                }
            }
            KEmailAddress::extractEmailAddressAndName(value.toString(), email, name);
            attendee.setName(name);
            attendee.setEmail(email);

            addEmptyAttendee();
            break;
        case Available:
            mAttendeeAvailable[index.row()] = static_cast<AvailableStatus>(value.toInt());
            break;
        case Status:
            attendee.setStatus(static_cast<KCalendarCore::Attendee::PartStat>(value.toInt()));
            break;
        case CuType:
            attendee.setCuType(static_cast<KCalendarCore::Attendee::CuType>(value.toInt()));
            break;
        case Response:
            attendee.setRSVP(value.toBool());
            break;
        default:
            return false;
        }
        Q_EMIT dataChanged(index, index);
        return true;
    }
    return false;
}

QVariant AttendeeTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return {};
    }

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case Role:
            return i18nc("vCard attendee role", "Role");
        case FullName:
            return i18nc("Attendees  (name+emailaddress)", "Name");
        case Available:
            return i18nc("Is attendee available for incidence", "Available");
        case Status:
            return i18nc("Status of attendee in an incidence (accepted, declined, delegated, …)", "Status");
        case CuType:
            return i18nc("Type of calendar user (vCard attribute)", "User Type");
        case Response:
            return i18nc("Has attendee to respond to the invitation", "Response");
        case Name:
            return i18nc("Attendee name", "Name");
        case Email:
            return i18nc("Attendee email", "Email");
        default:
            break;
        }
    }

    return {};
}

bool AttendeeTableModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    beginInsertRows(parent, position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        KCalendarCore::Attendee const attendee(QLatin1StringView(""), QLatin1StringView(""));
        mAttendeeList.insert(position, attendee);
        mAttendeeAvailable.insert(mAttendeeAvailable.begin() + position, AvailableStatus{});
    }

    endInsertRows();
    return true;
}

bool AttendeeTableModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    beginRemoveRows(parent, position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        mAttendeeAvailable.erase(mAttendeeAvailable.begin() + position);
        mAttendeeList.remove(position);
    }

    endRemoveRows();
    return true;
}

bool AttendeeTableModel::insertAttendee(int position, const KCalendarCore::Attendee &attendee)
{
    beginInsertRows(QModelIndex(), position, position);
    mAttendeeList.insert(position, attendee);
    mAttendeeAvailable.insert(mAttendeeAvailable.begin() + position, AvailableStatus{});
    endInsertRows();

    addEmptyAttendee();

    return true;
}

void AttendeeTableModel::setAttendees(const KCalendarCore::Attendee::List &attendees)
{
    beginResetModel();

    mAttendeeList = attendees;
    mAttendeeAvailable.clear();
    mAttendeeAvailable.resize(attendees.size());

    addEmptyAttendee();

    endResetModel();
}

KCalendarCore::Attendee::List AttendeeTableModel::attendees() const
{
    return mAttendeeList;
}

void AttendeeTableModel::addEmptyAttendee()
{
    if (mKeepEmpty) {
        bool create = true;
        for (const KCalendarCore::Attendee &attendee : std::as_const(mAttendeeList)) {
            if (attendee.fullName().isEmpty()) {
                create = false;
                break;
            }
        }

        if (create) {
            insertRows(rowCount(), 1);
        }
    }
}

bool AttendeeTableModel::keepEmpty() const
{
    return mKeepEmpty;
}

void AttendeeTableModel::setKeepEmpty(bool keepEmpty)
{
    if (keepEmpty != mKeepEmpty) {
        mKeepEmpty = keepEmpty;
        addEmptyAttendee();
    }
}

bool AttendeeTableModel::removeEmptyLines() const
{
    return mRemoveEmptyLines;
}

void AttendeeTableModel::setRemoveEmptyLines(bool removeEmptyLines)
{
    mRemoveEmptyLines = removeEmptyLines;
}

ResourceFilterProxyModel::ResourceFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool ResourceFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex cuTypeIndex = sourceModel()->index(sourceRow, AttendeeTableModel::CuType, sourceParent);
    KCalendarCore::Attendee::CuType const cuType = static_cast<KCalendarCore::Attendee::CuType>(sourceModel()->data(cuTypeIndex).toUInt());

    return cuType == KCalendarCore::Attendee::Resource || cuType == KCalendarCore::Attendee::Room;
}

AttendeeFilterProxyModel::AttendeeFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool AttendeeFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex cuTypeIndex = sourceModel()->index(sourceRow, AttendeeTableModel::CuType, sourceParent);
    KCalendarCore::Attendee::CuType const cuType = static_cast<KCalendarCore::Attendee::CuType>(sourceModel()->data(cuTypeIndex).toUInt());

    return !(cuType == KCalendarCore::Attendee::Resource || cuType == KCalendarCore::Attendee::Room);
}

#include "moc_attendeetablemodel.cpp"
