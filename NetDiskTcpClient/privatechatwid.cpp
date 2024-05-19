#include "privatechatwid.h"
#include "ui_privatechatwid.h"

privateChatWid::privateChatWid(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::privateChatWid)
{
    ui->setupUi(this);
}

privateChatWid::~privateChatWid()
{
    delete ui;
}

QString privateChatWid::chatName() const
{
    return m_ChatName;
}

void privateChatWid::setChatName(const QString &ChatName)
{
    m_ChatName = ChatName;
}

QString privateChatWid::loginName() const
{
    return m_LoginName;
}

void privateChatWid::setLoginName(const QString &LoginName)
{
    m_LoginName = LoginName;
}

void privateChatWid::updateShowMsgTE(PDU *pdu)
{
    if(NULL == pdu)
    {
        return;
    }
    char caSendName[32] = {'\0'};
    strncpy(caSendName,pdu->caData+32,32);
    QString strMsg = QString("%1 : %2").arg(caSendName).arg((char*)pdu->caMsg);
    ui->showMsg_ED->append(strMsg);
}

void privateChatWid::setPriChatTitle(const char *caTitle)
{
    ui->label->setText(caTitle);
}

void privateChatWid::on_sendMsg_PB_clicked()
{
    QString sendMsg = ui->inputMsg_ED->text();
    if(sendMsg.isEmpty())
    {
        QMessageBox::warning(this,"私聊","消息不能为空");
        return;
    }

    //显示在自己showMsg窗口上
    ui->inputMsg_ED->clear();//清空输入框内容
    ui->showMsg_ED->append(QString("%1 : %2").arg(m_LoginName).arg(sendMsg));

    //发送消息给服务器来转发给对方
    PDU* pdu = mkPDU(sendMsg.size());
    pdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;
    strncpy(pdu->caData,m_ChatName.toStdString().c_str(),32);   //目标用户名
    strncpy(pdu->caData+32,m_LoginName.toStdString().c_str(),32);   //请求方用户名
    strncpy((char*)pdu->caMsg,sendMsg.toStdString().c_str(),sendMsg.size());    //发送内容

    NetDiskTcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

