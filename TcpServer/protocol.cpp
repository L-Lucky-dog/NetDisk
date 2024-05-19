#include "protocol.h"


PDU *mkPDU(uint uiMsgLen)
{
    uint uiPDULen = sizeof(PDU)+uiMsgLen;//总的大小 = 前面不变 + 实际消息长度

    PDU *pdu = (PDU *)malloc(uiPDULen);

    if(NULL == pdu)  //如果申请空间失败，那就结束程序
    {
        exit(EXIT_FAILURE);
    }

    memset(pdu,0,uiPDULen);

    //拷贝总数据大小+实际消息大小到对象中
    pdu->uiPDULen = uiPDULen;
    pdu->uiMsgLen = uiMsgLen;

    return pdu;
}

