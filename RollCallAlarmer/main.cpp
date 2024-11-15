#include "RollCallAlarmer.h"
#include <QtWidgets/QApplication>
#include <QFile>
void customMessageHandler(QtMsgType type, const QMessageLogContext&, const QString& str)
{
    QString txt = str;
    QString app_run_addr;
    //获取程序当前运行目录
    QString current_PathName = QCoreApplication::applicationDirPath();
    if (QFile::exists(current_PathName) == false)
    {
        app_run_addr = "debug.log";
    }
    else
    {
        app_run_addr = current_PathName + "/" + "debug.log";
    }
    QFile outFile(app_run_addr);
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << Qt::endl;
}
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qInstallMessageHandler(customMessageHandler);
    RollCallAlarmer w;
    w.show();
    return a.exec();
}
