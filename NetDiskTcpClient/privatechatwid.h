#ifndef PRIVATECHATWID_H
#define PRIVATECHATWID_H

#include <QWidget>
#include <QMessageBox>

#include "protocol.h"
#include "netdisktcpclient.h"

namespace Ui {
class privateChatWid;
}

class privateChatWid : public QWidget
{
    Q_OBJECT

public:
    explicit privateChatWid(QWidget *parent = nullptr);
    ~privateChatWid();

    QString chatName() const;
    void setChatName(const QString &ChatName);

    QString loginName() const;
    void setLoginName(const QString &LoginName);

    void updateShowMsgTE(PDU* pdu);     //更新showMsgTE聊天框内容
    void setPriChatTitle(const char* caTitle);  //设置私聊框的title

private slots:
    void on_sendMsg_PB_clicked();//槽函数：发送消息按钮

private:
    Ui::privateChatWid *ui;

    QString m_ChatName; //聊天对象用户名
    QString m_LoginName;//请求用户名
};

#endif // PRIVATECHATWID_H
