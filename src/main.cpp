#include "basedialog.h"
#include <QApplication>
#include <QTextCodec>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForTr(codec);

    QFont font("simsun", 18, 60);
    font.setBold(true);
    a.setFont(font);

    BaseDialog w;
    w.show();
    return a.exec();
}
