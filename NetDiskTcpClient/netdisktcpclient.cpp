#include "netdisktcpclient.h"
#include "ui_netdisktcpclient.h"
#include <QByteArray>
#include <QMessageBox>
#include <QDebug>
#include <QHostAddress>



NetDiskTcpClient::NetDiskTcpClient(QWidget *parent) : QWidget(parent), ui(new Ui::NetDiskTcpClient)
{
    ui->setupUi(this);

    resize(500,250);//重新编辑窗口大小

    loadConfig();

    connect(&m_ClientSocket,SIGNAL(connected()),this,SLOT(showConnect()));//Socket发送信号，相关函数处理
    connect(&m_ClientSocket,SIGNAL(readyRead()),this,SLOT(receiveMsg()));//与客户端的socket关联起来

    m_ClientSocket.connectToHost(QHostAddress(m_IP),m_Port); //通过Socket连接服务器

}

NetDiskTcpClient::~NetDiskTcpClient()
{
    delete ui;
}

void NetDiskTcpClient::loadConfig()
{
    QFile file(":/client.config");
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

QString NetDiskTcpClient::loginName()
{
    return m_LoginName;
}

NetDiskTcpClient &NetDiskTcpClient::getInstance()
{
    //静态的，总是统一对象实例：
    static NetDiskTcpClient instance;
    return instance;
}

QString NetDiskTcpClient::getStrName() const
{
    return m_LoginName;
}

void NetDiskTcpClient::setStrName(const QString &strName)
{
    m_LoginName = strName;
}

QString NetDiskTcpClient::getStrCurPath() const
{
    return m_StrCurPath;
}

void NetDiskTcpClient::setStrCurPath(const QString &strCurPath)
{
    m_StrCurPath = strCurPath;
}

QString NetDiskTcpClient::getStrRootPath() const
{
    return m_StrRootPath;
}

void NetDiskTcpClient::setStrRootPath(const QString &strRootPath)
{
    m_StrRootPath = strRootPath;
}

QTcpSocket &NetDiskTcpClient::getTcpSocket()
{
    return m_ClientSocket;
}

void NetDiskTcpClient::showConnect()
{
    QMessageBox::information(this,"连接服务器","连接服务器成功");
}

void NetDiskTcpClient::receiveMsg()
{
    qDebug() << m_ClientSocket.bytesAvailable();//当前可读有多少数据


    //接收数据:
    uint uiPDULen = 0;
    m_ClientSocket.read((char *)&uiPDULen,sizeof(uint));
    uint uiMsgLen = uiPDULen-sizeof(PDU);//实际消息长度
    PDU* pdu = mkPDU(uiMsgLen);
    m_ClientSocket.read((char*)pdu+sizeof(uint),uiPDULen-sizeof(uint));//将实际长度的数据加载到后置位


    //接收数据后，判断消息类型：
    switch(pdu->uiMsgType)
    {

    //如果是注册请求：
    case ENUM_MSG_TYPE_REGIST_RESPOND:
    {
        if(0==strcmp(pdu->caData,REGIST_OK))
        {
            QMessageBox::information(this,"注册",REGIST_OK);
        }
        else if(0==strcmp(pdu->caData,REGIST_FAILED))
        {
            QMessageBox::warning(this,"注册",REGIST_FAILED);
        }
        break;
    }

    //如果是登录请求:
    case ENUM_MSG_TYPE_LOGIN_RESPOND:
    {
        if(0==strcmp(pdu->caData,LOGIN_OK))
        {
            QMessageBox::information(this,"登录",LOGIN_OK);

            char caName[32] = {'\0'};
            strncpy(caName,pdu->caData+32,32);

            //设置用户根目录和当前目录
            m_StrRootPath = QString((char*)pdu->caMsg);
            qDebug() << "用户根目录：" << m_StrRootPath;
            m_StrCurPath = m_StrRootPath;
            m_LoginName = caName;
            qDebug() << "用户已登录：" << caName << "strName:" <<m_LoginName;

            //登录跳转：
            operateWidget::getInstance().setuserLable(caName);
            operateWidget::getInstance().show();

            //默认请求一次好友列表：
            operateWidget::getInstance().getFriend()->flushFriendList();

            //隐藏登录界面
            this->hide();

        }
        else if(0==strcmp(pdu->caData,LOGIN_FAILED))
        {
            QMessageBox::warning(this,"登录",LOGIN_FAILED);
        }
        break;
    }

    //如果是在线人员请求：
    case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND:
    {
        //通过操作界面->好友界面->通过showAllOnlineUser函数传输pdu
        operateWidget::getInstance().getFriend()->showAllOnlineUser(pdu);
        break;
    }

    //如果是查找用户请求:
    case ENUM_MSG_TYPE_SEARCH_USER_RESPOND:
    {
        if(0 == strcmp(SEARCH_USER_FAILED,pdu->caData))
        {
            QMessageBox::information(this,"搜索",QString("%1: 不存在").arg(operateWidget::getInstance().getFriend()->m_strSearchName));
        }
        else if(0 == strcmp(SEARCH_USER_ONLINE,pdu->caData))
        {
            QMessageBox::information(this,"搜索",QString("%1: 在线").arg(operateWidget::getInstance().getFriend()->m_strSearchName));
        }
        else if(0 == strcmp(SEARCH_USER_OFFLINE,pdu->caData))
        {
            QMessageBox::information(this,"搜索",QString("%1: 离线").arg(operateWidget::getInstance().getFriend()->m_strSearchName));
        }

        break;
    }

    case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND:
    {
        QMessageBox::information(this,"添加好友",pdu->caData);
        break;
    }

    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
    {
        char sourceName[32];
        strncpy(sourceName,pdu->caData+32,32);
        int ret = QMessageBox::information(this,"好友申请",QString("%1 请求添加好友").arg(sourceName),QMessageBox::Yes,QMessageBox::No);

        PDU* resPdu = mkPDU(0);

        strncpy(resPdu->caData,pdu->caData,32);
        strncpy(resPdu->caData+32,pdu->caData+32,32);

        if(ret == QMessageBox::Yes)
        {
            resPdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE;
        }
        else
        {
            resPdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
        }
        m_ClientSocket.write((char*)resPdu,resPdu->uiPDULen);

        break;
    }

    case ENUM_MSG_TYPE_ADD_FRIEND_AGREE:
    {
        QMessageBox::information(this,"添加好友",QString("%1 同意好友申请").arg(pdu->caData));
        break;
    }

    case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:
    {
        QMessageBox::information(this,"添加好友",QString("%1 拒绝好友申请").arg(pdu->caData));
        break;
    }

    case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND:
    {
        operateWidget::getInstance().getFriend()->updateFriendList(pdu);
        break;
    }

    case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND:
    {
        QMessageBox::information(this,"删除好友",pdu->caData);
        break;
    }

    //处理服务器转发过来的删除好友请求
    case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
    {
        char sourceName[32];
        strncpy(sourceName,pdu->caData+32,32);
        QMessageBox::information(this,"删除好友",QString("%1 已删除与您的好友关系").arg(sourceName));
        break;
    }

    //私聊好友请求(接受端)
    case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
    {
        //获取发送方用户名
        char sourceName[32];

        strncpy(sourceName,pdu->caData+32,32);
        privateChatWid* priChatW = operateWidget::getInstance().getFriend();
        if(NULL == priChatW)
        {
            priChatW = new privateChatWid;
            priChatW->setChatName(sourceName);
            priChatW->setLoginName(m_LoginName);
            priChatW->setPriChatTitle(sourceName);
            operateWidget::getInstance().getFriend()->insertPriChatWidList(priChatW);
        }
        priChatW->updateShowMsgTE(pdu);
        priChatW->show();
        if(priChatW->isMinimized())
        {
            priChatW->showNormal();
        }
        break;
    }

    case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
    {
        operateWidget::getInstance().getFriend()->updateGroupShowMsgTE(pdu);
        break;
    }

    //默认情况:
    default:
        break;
    }

    free(pdu);//发送完释放空间
    pdu = NULL;
}

// void NetDiskTcpClient::on_send_pb_clicked()//只要点击 发送 按钮，就会加载此函数
// {
//     QString textMsg = ui->lineEdit()->text();//获得输入框的数据
//     if(!textMsg.isEmpty())//判断输入框数据是否为空？
//     {
//         PDU *pdu = makePDU(textMsg.size());//产生一个PDU对象

//         pdu->uiMsgType = 8888;

//         memcpy(pdu->caMsg,textMsg.toStdString().c_str(),textMsg.size());//将要发送数据拷贝到pdu->caMsg空间内

//         m_tcpSocket.write((char *)pdu,pdu->uiPDULen);//将大小为pdu->uiPDULen的数据发送出去

//         free(pdu);//发送完释放空间
//         pdu = NULL;
//     }
//     else
//     {
//         QMessageBox::warning(this,"信息发送","发送信息不能为空");
//     }
// }

void NetDiskTcpClient::on_login_pb_clicked()
{
    //获得数据：
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();

    //如果获得的数据不为空：
    if(!strName.isEmpty() && !strPwd.isEmpty())
    {
        m_LoginName = strName;

        //设置一个PDU用来储存数据：
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;//设置消息类型
        strncpy(pdu->caData,strName.toStdString().c_str(),32);//将用户名的前32个字节拷贝到caData内
        strncpy(pdu->caData+32,strPwd.toStdString().c_str(),32);

        //之后通过socket将其发送出去:
        m_ClientSocket.write((char *)pdu,pdu->uiPDULen);//将大小为pdu->uiPDULen的数据写入进去
        free(pdu);//发送完释放空间
        pdu = NULL;//记得指针置为NULL
    }
    else
    {
        QMessageBox::critical(this,"登录","登录失败：用户名或密码为空！");
    }
}

void NetDiskTcpClient::on_regist_pb_clicked()
{
    //获得数据：
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();

    //如果获得的数据不为空：
    if(!strName.isEmpty() && !strPwd.isEmpty())
    {
        //设置一个PDU用来储存数据：
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;//设置消息类型
        strncpy(pdu->caData,strName.toStdString().c_str(),32);//将用户名的前32个字节拷贝到caData内
        strncpy(pdu->caData+32,strPwd.toStdString().c_str(),32);

        //之后通过socket将其发送出去:
        m_ClientSocket.write((char *)pdu,pdu->uiPDULen);//将大小为pdu->uiPDULen的数据写入进去
        free(pdu);//发送完释放空间
        pdu = NULL;//记得指针置为NULL
    }
    else
    {
        QMessageBox::critical(this,"注册","注册失败：用户名或密码不能为空！");
    }
}

void NetDiskTcpClient::on_logout_pb_clicked()
{

}

