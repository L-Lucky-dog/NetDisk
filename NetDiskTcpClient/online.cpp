#include "online.h"
#include "ui_online.h"
#include"netdisktcpclient.h"

#include<QDebug>

Online::Online(QWidget *parent) : QWidget(parent), ui(new Ui::Online)
{
    ui->setupUi(this);
}

Online::~Online()
{
    delete ui;
}

void Online::showUser(PDU *pdu)
{
    //参数有效性检测：
    if(NULL == pdu)
    {
        return;
    }

    //
    uint uiSize = pdu->uiMsgLen/32;

    //
    char caTemp[32];

    //
    for(uint i = 0; i<uiSize; i++)
    {
        memcpy(caTemp,(char*)(pdu->caMsg)+i*32,32);
        ui->online_listW->addItem(caTemp);
    }
}

void Online::on_addFriend_pb_clicked()
{
    //currenItem()函数：返回当前选中项
    QListWidgetItem *pItem = ui->online_listW->currentItem();

    QString strPerUserName = pItem->text();
    QString strLoginName = NetDiskTcpClient::getInstance().loginName();

    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;

    //你要添加谁：
    memcpy(pdu->caData,strPerUserName.toStdString().c_str(),strPerUserName.size());
    //你是谁：
    memcpy(pdu->caData+32,strLoginName.toStdString().c_str(),strLoginName.size());

    //发送数据->
    NetDiskTcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);

    free(pdu);
    pdu = NULL;
}

