#include "netdisktcpclient.h"
#include "operatewidget.h"
#include <QApplication>
#include <online.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //由于已产生静态对象, 不可再产生对象
    // NetDiskTcpClient w;
    // w.show();
    NetDiskTcpClient::getInstance().show();




    return a.exec();
}
