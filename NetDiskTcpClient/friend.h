#ifndef FRIEND_H
#define FRIEND_H

#include <QWidget>
#include <QTextEdit>//信息显示
#include <QListWidget>//列表
#include <QLineEdit>//信息输入
#include <QPushButton>//按钮
#include <QVBoxLayout>//垂直布局
#include <QHBoxLayout>//水平布局

#include "online.h"
#include "protocol.h"
#include "netdisktcpclient.h"
#include "privatechatwid.h"

class Friend : public QWidget
{
    Q_OBJECT
public:
    //构造函数:
    explicit Friend(QWidget *parent = nullptr);

    //函数：显示在线用户
    void showAllOnlineUser(PDU *pdu);

    //函数：设置在线
    void setOnline(PDU* pdu);

    //函数：刷新好友列表
    void updateFriendList(PDU *pdu);

    //函数：设置搜索用户名
    void setStrSearchName(const QString &strSearchName);

    //函数：得到搜索用户名
    QString getStrSearchName() const;

    QListWidget* getFriendLW() const;

signals:



public slots:
    //槽函数：显示在线用户
    void showOnline();

    //槽函数：搜索用户
    void searchUser();

    //槽函数：刷新好友列表
    void flushFriendList();

    //槽函数：删除好友
    void deleteFriend();

    //槽函数：私聊
    void privateChat();

    //获得对应用户名的私聊窗口
    privateChatWid* searchPriChatWid(const char* chatName);

    //将私聊窗口插入到私聊窗口List
    void insertPriChatWidList(privateChatWid* priChat);

    //槽函数：群聊发送按钮
    void groupChatSendMsg();

    //更新群聊showMsgTE聊天消息
    void updateGroupShowMsgTE(PDU* pdu);

private:
    QTextEdit *m_ShowMsgTE;             //显示信息
    QPushButton *m_MsgSendPB;           //发送信息

    QLineEdit *m_InputMsgLE;            //信息输入框

    QListWidget *m_FriendListWidget;    //好友列表
    QPushButton *m_DelFriendPB;         //删除好友
    QPushButton *m_FlushFriendPB;       //刷新好友列表

    QPushButton *m_ShowOnlineUserPB;    //查看在线人数
    Online *m_Online;                   //在线对象

    QPushButton *m_SearchUserPB;        //搜索用户
    QString m_strSearchName;            //查找用户名

    QLineEdit* m_GroupInputLE;          //群聊信息输入
    QPushButton* m_GroupSendMsgPB;      //群聊发送按钮
    QTextEdit* m_GroupShowMsgTE;        //群聊显示信息框

    QPushButton *m_PrivateTalkPB;       //私聊按钮
    QList<privateChatWid*> m_priChatWidList;    //所有私聊的窗口
};

#endif // FRIEND_H
