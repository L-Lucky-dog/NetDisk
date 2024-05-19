#include "tcpserver.h"
#include <QApplication>
#include "operatedb.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    operateDB::getInstance().init();//初始化

    TcpServer w;
    w.show();
    return a.exec();
}
