
#include "../src/individualmaildialog.h"

#include <KGuiItem>
#include <QApplication>

using namespace IncidenceEditorNG;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    KCalCore::Attendee::List attendees;
    KGuiItem buttonYes = KGuiItem(QStringLiteral("Send Email"));
    KGuiItem buttonNo = KGuiItem(QStringLiteral("Do not send"));

    KCalCore::Attendee::Ptr attendee1(new KCalCore::Attendee(QStringLiteral("test1"), QStringLiteral("test1@example.com")));
    KCalCore::Attendee::Ptr attendee2(new KCalCore::Attendee(QStringLiteral("test2"), QStringLiteral("test2@example.com")));
    KCalCore::Attendee::Ptr attendee3(new KCalCore::Attendee(QStringLiteral("test3"), QStringLiteral("test3@example.com")));

    attendees << attendee1 << attendee2 << attendee3;

    IndividualMailDialog dialog(QStringLiteral("title"), attendees, buttonYes, buttonNo, nullptr);
    dialog.show();
    return app.exec();
}
