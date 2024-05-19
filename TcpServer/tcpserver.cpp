#include "tcpserver.h"
#include "ui_tcpserver.h"
#include "mytcpserver.h"
#include <QByteArray>
#include <QMessageBox>
#include <QDebug>
#include <QHostAddress>
#include <QFile>

TcpServer::TcpServer(QWidget *parent) : QWidget(parent), ui(new Ui::TcpServer)
{
    ui->setupUi(this);

    loadConfig();   //加载配置文件，绑定IP和端口以进行监听

    MyTcpServer::getInstance().listen(QHostAddress(m_IP),m_Port);//监听函数:只要有客户端连接过来就自动调用incomingConnection()

}

TcpServer::~TcpServer()
{
    delete ui;
}

void TcpServer::loadConfig()
{
    QFile file(":/server.config");
    if(file.open(QIODeviceBase::ReadOnly))
    {
        QByteArray baData = file.readAll();  //因为readAll是字节类型，所以需要定义一个字节变量保存
        QString strData = baData.toStdString().c_str(); //转换成字符串类型

        strData.replace("\r\n"," ");
        QStringList strList = strData.split(" ");  //切分：①IP地址   ②端口

        m_IP = strList.at(0);
        m_Port = strList.at(1).toUShort(); //之前端口被转换成了字符串，现在需要再将其转换成整形

        qDebug()<<"IP:"<<m_IP<<" Port:"<<m_Port;

        file.close();//关闭文件
    }
    else
    {
        QMessageBox::critical(this,"open config","open config failed");
    }
}
