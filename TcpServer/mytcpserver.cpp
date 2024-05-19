#include "mytcpserver.h"
#include <QDebug>




MyTcpServer::MyTcpServer()
{

}

MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return instance;

}

void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug()<<"新用户已连接";

    //以下socket三步骤：
    //1. 定义一个新socket
    MyTcpSocket* pTcpSocket = new MyTcpSocket;//new一个

    //2. 设置socket描述符
    pTcpSocket->setSocketDescriptor(socketDescriptor);

    //3. 将新socket，保存到socket列表里
    m_tcpSocketList.append(pTcpSocket);

    connect(pTcpSocket,SIGNAL(offline(MyTcpSocket*)),this,SLOT(socketDelete(MyTcpSocket*)));
}

bool MyTcpServer::forwardMsg(const QString caDesName, PDU *pdu)
{
    if(caDesName == NULL || pdu == NULL)
    {
        return false;
    }

    //查找目标用户的socket
    for(int i = 0;i < m_tcpSocketList.size();i++)
    {
        if(caDesName == m_tcpSocketList.at(i)->getName())
        {
            m_tcpSocketList.at(i)->write((char*)pdu,pdu->uiPDULen);
            return true;
        }
    }
    return false;
}

QString MyTcpServer::getRootPath() const
{
    return m_rootPath;
}

void MyTcpServer::setRootPath(const QString &rootPath)
{
    m_rootPath = rootPath;
}

void MyTcpServer::socketDelete(MyTcpSocket *m_socket)
{
    //利用迭代器删除:
    QList<MyTcpSocket*>::iterator it = m_tcpSocketList.begin();

    //寻找socket地址:
    for(;it!=m_tcpSocketList.end();it++)
    {
        if(m_socket == *it)
        {
            //1.删除指针指向的对象
            delete *it;
            *it = NULL;

            //2.删除链表内的指针
            m_tcpSocketList.erase(it);
            break;
        }
    }

    //测试打印:
    /*for(int i = 0; i<m_tcpSocketList.size(); i++)
    {
        qDebug() << m_tcpSocketList.at(i)->getName();
    }*/

}



