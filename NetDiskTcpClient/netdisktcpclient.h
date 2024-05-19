
#ifndef NETDISKTCPCLIENT_H
#define NETDISKTCPCLIENT_H
#include <QWidget>
#include <QFile>
#include <QTcpSocket>

#include "operatewidget.h"

QT_BEGIN_NAMESPACE namespace Ui
{
    class NetDiskTcpClient;

}QT_END_NAMESPACE


class NetDiskTcpClient : public QWidget
{
    Q_OBJECT

public:
    //构造函数：
    NetDiskTcpClient(QWidget *parent = nullptr);

    //析构函数：
    ~NetDiskTcpClient();

    //加载资源文件：
    void loadConfig();

    //访问private内m_LoginName的接口：
    QString loginName();

    //获得实例函数：
    static NetDiskTcpClient &getInstance();

    //获得登录用户名函数：
    QString getStrName() const;
    void setStrName(const QString &strName);

    QString getStrCurPath() const;
    void setStrCurPath(const QString &strCurPath);

    QString getStrRootPath() const;
    void setStrRootPath(const QString &strRootPath);

    //获得socket实例函数：
    QTcpSocket &getTcpSocket();

public slots:

    //信号处理槽函数，成功连接到服务器会发送connected()信号
    void showConnect();

    //接收数据槽函数:
    void receiveMsg();


 private slots:
    //void on_send_pb_clicked();//发送测试槽函数

    //登录点击槽函数:
    void on_login_pb_clicked();

    //注册点击槽函数:
    void on_regist_pb_clicked();

    //注销点击槽函数:
    void on_logout_pb_clicked();

private:
    Ui::NetDiskTcpClient *ui;   //客户端ui界面
    QString m_IP;               //存放IP
    quint16 m_Port;             //存放端口，quint=q unsigned int
    QTcpSocket m_ClientSocket;     //声明一个socket
    QString m_LoginName;        //登录用户名
    QString m_StrRootPath;         //用户根目录
    QString m_StrCurPath;          //当前目录
};
#endif // NETDISKTCPCLIENT_H
