#include "protocol.h"


PDU *mkPDU(uint uiMsgLen)
{
    //总大小 = 结构体大小 + 实际消息长度
    uint uiPDULen = sizeof(PDU)+uiMsgLen;
    PDU *pdu = (PDU *)malloc(uiPDULen);

    //如果申请空间失败，那就结束程序
    if(NULL == pdu)
    {
        exit(EXIT_FAILURE);
    }

    memset(pdu,0,uiPDULen);

    //拷贝总数据大小+实际消息大小到对象中
    pdu->uiPDULen = uiPDULen;
    pdu->uiMsgLen = uiMsgLen;

    return pdu;
}

