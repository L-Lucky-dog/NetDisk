#include <QInputDialog>
#include "friend.h"
#include "netdisktcpclient.h"


Friend::Friend(QWidget *parent) : QWidget{parent}
{
    m_ShowMsgTE = new QTextEdit;                    //显示信息
    m_FriendListWidget = new QListWidget;           //好友列表
    m_GroupInputLE = new QLineEdit;
    m_GroupShowMsgTE = new QTextEdit;
    m_InputMsgLE = new QLineEdit;                   //信息输入框
    m_DelFriendPB = new QPushButton("删除好友");    //删除好友
    m_FlushFriendPB = new QPushButton("刷新好友列表");  //刷新好友列表
    m_ShowOnlineUserPB = new QPushButton("显示在线用户");//查看在线人数
    m_SearchUserPB = new QPushButton("查找用户");       //搜索用户
    m_MsgSendPB = new QPushButton("发送");            //发送信息
    m_PrivateTalkPB = new QPushButton("私聊");        //私聊

    //将5个按钮添加到垂直布局:
    QVBoxLayout *RightPBVBL = new QVBoxLayout;
    RightPBVBL->addWidget(m_DelFriendPB);
    RightPBVBL->addWidget(m_FlushFriendPB);
    RightPBVBL->addWidget(m_ShowOnlineUserPB);
    RightPBVBL->addWidget(m_SearchUserPB);
    RightPBVBL->addWidget(m_PrivateTalkPB);

    //设置水平布局:
        //将 显示信息、好友列表、RightPBVBL 设置到水平布局TopHBL里
    QHBoxLayout *TopHBL = new QHBoxLayout;
    TopHBL->addWidget(m_ShowMsgTE);
    TopHBL->addWidget(m_FriendListWidget);
    TopHBL->addLayout(RightPBVBL);
        //将 信息输入框、信息发送按钮 设置到水平布局MsgHBL里:
    QHBoxLayout *MsgHBL = new QHBoxLayout;
    MsgHBL->addWidget(m_InputMsgLE);
    MsgHBL->addWidget(m_MsgSendPB);

    m_Online = new Online;

    //将 TopHBL、MsgHBL 设置到主布局内
    QVBoxLayout *Main = new QVBoxLayout;//主布局情况
    Main->addLayout(TopHBL);
    Main->addLayout(MsgHBL);
    Main->addWidget(m_Online);
    m_Online->hide();//只有在线才会显示，否则隐藏

    setLayout(Main);

    //关联: 显示在线用户按钮-----Friend对象
    connect(m_ShowOnlineUserPB,SIGNAL(clicked(bool)),this,SLOT(showOnline()));

    //关联:搜索用户按钮------Friend对象
    connect(m_SearchUserPB,SIGNAL(clicked(bool)),this,SLOT(searchUser()));

    //关联：刷新用户按钮-------刷新用户槽函数
    connect(m_FlushFriendPB,SIGNAL(clicked(bool)),this,SLOT(flushFriend()));

    //关联：删除好友
    connect(m_DelFriendPB,SIGNAL(clicked(bool)),this,SLOT(deleteFriend()));

}

void Friend::showAllOnlineUser(PDU *pdu)
{
    //参数有效性检测：
    if(NULL == pdu)
    {
        return;
    }
    m_Online->showUser(pdu);
}

void Friend::updateFriendList(PDU *pdu)
{
    if(NULL == pdu){
        return;
    }
    uint uiSize = pdu->uiMsgLen/32;
    char Name[32] = {'\0'};
    for(uint i = 0;i<uiSize;i++)
    {
        memcpy(Name,(char*)(pdu->caMsg)+i*32,32);
        m_FriendListWidget->addItem(Name);
    }
}

void Friend::setStrSearchName(const QString &strSearchName)
{
    m_strSearchName = strSearchName;
}

QString Friend::getStrSearchName() const
{
    return m_strSearchName;
}

QListWidget *Friend::getFriendLW() const
{
    return m_FriendListWidget;
}

void Friend::showOnline()
{
    if(m_Online->isHidden())
    {
        m_Online->show();

        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        NetDiskTcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        m_Online->hide();
    }

}

void Friend::searchUser()
{
    QString m_strSearchName = QInputDialog::getText(this,"搜索","用户名：");
    if(!m_strSearchName.isEmpty())
    {
        //qDebug() << name;

        PDU *pdu = mkPDU(0);
        memcpy(pdu->caData,m_strSearchName.toStdString().c_str(),m_strSearchName.size());
        pdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USER_REQUEST;

        //从客户端通过socket将pdu数据发送出去:
        NetDiskTcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Friend::flushFriendList()
{
    //1.客户端发送好友请求--->mytcpsocket.h
    QString strName = NetDiskTcpClient::getInstance().loginName();
    PDU* pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST;
    memcpy(pdu->caData,strName.toStdString().c_str(),strName.size());
    NetDiskTcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Friend::deleteFriend()
{
    if(NULL == m_FriendListWidget->currentItem())
    {
        return;
    }
    QString friendName = m_FriendListWidget->currentItem()->text();     //获得选中好友
    friendName = friendName.split("\t")[0];
    QString loginName = NetDiskTcpClient::getInstance().getStrName();   //获取登录用户名

    PDU* pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;
    strncpy(pdu->caData,friendName.toStdString().c_str(),32);
    strncpy(pdu->caData+32,loginName.toStdString().c_str(),32);
    NetDiskTcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Friend::privateChat()
{
    //如果没有选中好友：
    if(NULL == m_FriendListWidget->currentItem())
    {
        return;
    }

    //获得选中好友名：
    QString friName = m_FriendListWidget->currentItem()->text();

    friName = friName.split("\t")[0];

    //获得登录用户名：
    QString loginName = NetDiskTcpClient::getInstance().getStrName();

    privateChatWid* priChat = searchPriChatWid(friName.toStdString().c_str());

    if(priChat == NULL)
    {
        priChat = new privateChatWid;
        priChat->setChatName(friName);
        priChat->setLoginName(loginName);
        priChat->setPriChatTitle(friName.toStdString().c_str());

        //添加该客户端的私聊
        m_priChatWidList.append(priChat);
    }

    //若窗口被隐藏，让其显示
    if(priChat->isHidden())
    {
        priChat->show();
    }

    //若窗口最小化
    if(priChat->isMinimized())
    {
        priChat->showNormal();
    }

}

privateChatWid *Friend::searchPriChatWid(const char *chatName)
{
    for(privateChatWid* ptr:m_priChatWidList)
    {
        if(ptr->chatName() == chatName)
        {
            return ptr;
        }
    }
    return NULL;
}

void Friend::insertPriChatWidList(privateChatWid *priChat)
{
    m_priChatWidList.append(priChat);
}

void Friend::groupChatSendMsg()
{
    QString strMsg = m_GroupInputLE->text();//得到输入框内容
    if(strMsg.isEmpty())
    {
        QMessageBox::warning(this,"群聊","发送信息不能为空");
        return;
    }
    m_GroupInputLE->clear();//清空输入框
    m_GroupShowMsgTE->append(QString("%1 : %2").arg(NetDiskTcpClient::getInstance().getStrName()).arg(strMsg));

    PDU* pdu = mkPDU(strMsg.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_GROUP_CHAT_REQUEST;
    strncpy(pdu->caData,NetDiskTcpClient::getInstance().getStrName().toStdString().c_str(),32);
    strncpy((char*)(pdu->caMsg),strMsg.toStdString().c_str(),strMsg.size());
    NetDiskTcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;

}

void Friend::updateGroupShowMsgTE(PDU *pdu)
{
    QString strMsg = QString("%1 : %2").arg(pdu->caData).arg((char*)pdu->caMsg);
    m_GroupShowMsgTE->append(strMsg);
}
















