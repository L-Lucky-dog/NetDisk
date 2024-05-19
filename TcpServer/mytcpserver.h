#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include <QList>
#include "mytcpsocket.h"


//继承QTcpServer，自写一个派生类进行监听:
class MyTcpServer : public QTcpServer
{
    Q_OBJECT

public:
    //构造函数：
    MyTcpServer();

    //静态实例化函数：
    static MyTcpServer &getInstance();

    //重写incomingConnection()函数：
    void incomingConnection(qintptr socketDescriptor) override;

    //服务器转发给其他客户端消息：
    bool forwardMsg(const QString caDesName,PDU* pdu);

    QString getRootPath() const;

    void setRootPath(const QString &rootPath);
public slots:
    //删除socket-List内socket的槽函数：
    void socketDelete(MyTcpSocket *m_socket);

private:
    //自己创建了一个MyTcpSocket类型的链表容器:
    QList<MyTcpSocket*> m_tcpSocketList;

    //文件系统根目录
    QString m_rootPath;
};

#endif // MYTCPSERVER_H
