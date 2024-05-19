//服务器接收数据：
#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include <QFile>
#include <QTimer>

#include "protocol.h"
#include "operatedb.h"
#include "mytcpserver.h"

//继承QTcpSocket，自己写一个Socket类
class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT   //满足2个条件以支持信号-槽函数: 1.Q_OBJECT   2.QT += core gui network sql
public:
    //构造函数：
    MyTcpSocket();

    //作为接口获得m_Name属性：
    QString getStrName();
signals:
    //下线信号:
    void offline(MyTcpSocket *m_socket);
public slots:
    //接收信息槽函数：
    void receiveMsg();//当Socket有数据过来时会发出readyRead信号函数，此槽函数进行接收

    //处理客户端下线槽函数：
    void clientOffline();

    void handleDownLoadFileData();
private:
    TransFile* m_uploadFile; // 上传文件的信息

    QFile *m_pDownloadFile; // 客户端要下载的文件
    QTimer *m_pTimer; // 计时器

    QString m_StrName;     //用户名，为了区分该socket属于哪个用户
};

#endif // MYTCPSOCKET_H
