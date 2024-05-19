#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QWidget>
#include <QFile>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE

namespace Ui
{
    class TcpServer;
}

QT_END_NAMESPACE

class TcpServer : public QWidget
{
    Q_OBJECT
public:
    //构造函数：
    TcpServer(QWidget *parent = nullptr);

    //析构函数：
    ~TcpServer();

    //加载资源函数：绑定IP和端口以进行监听
    void loadConfig();
private:
    Ui::TcpServer *ui;      //ui界面
    QString m_IP;           //IP号
    quint16 m_Port;         //端口号
    QTcpSocket m_tcpSocket; //socket属性
};

#endif // TCPSERVER_H
