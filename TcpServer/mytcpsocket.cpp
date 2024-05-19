//服务器接收数据：

#include "mytcpsocket.h"
#include "mytcpserver.h"
#include <QDir>         // 操作文件夹的库
#include <QDateTime>
#include <QDebug>


MyTcpSocket::MyTcpSocket()
{
    m_uploadFile = new TransFile;
    m_uploadFile->bTransform = false; // 默认不是在上传文件
    m_pDownloadFile = new QFile;
    m_pTimer = new QTimer;

    connect(this,SIGNAL(readyRead()),this,SLOT(receiveMsg()));
    connect(this,SIGNAL(disconnected()),this,SLOT(clientOffline()));
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(handledownloadFileData())); // 关联计时器倒计时
}

QString MyTcpSocket::getStrName()
{
    return m_StrName;
}

void MyTcpSocket::receiveMsg()
{
    qDebug() << this->bytesAvailable();//当前可读有多少数据

    //接收数据:
    uint uiPDULen = 0;
    this->read((char *)&uiPDULen,sizeof(uint));
    uint uiMsgLen = uiPDULen-sizeof(PDU);//实际消息长度
    PDU* pdu = mkPDU(uiMsgLen);
    this->read((char*)pdu+sizeof(uint),uiPDULen-sizeof(uint));//将实际长度的数据加载到后置位

    //接收数据后，判断消息类型：
    switch(pdu->uiMsgType)
    {
    //如果是注册请求：
    case ENUM_MSG_TYPE_REGIST_REQUEST:
    {
        //获得用户名和密码
        char caName[32] = {'\0'};
        char caPwd[32] = {'\0'};
        strncpy(caName,pdu->caData,32);
        strncpy(caPwd,pdu->caData+32,32);

        //去数据库处理
        bool ret = operateDB::getInstance().handleRegist(caName,caPwd);

        PDU* respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;

        //判断数据库处理是否成功：
        if(ret)
        {
            strcpy(respdu->caData,REGIST_OK);
        }
        else
        {
            strcpy(respdu->caData,REGIST_FAILED);
        }

        //把结果返回给客户端:
        write((char *)respdu,respdu->uiPDULen);//将大小为pdu->uiPDULen的数据发送出去
        free(respdu);//发送完释放空间
        respdu = NULL;
        break;
    }

    //如果是登录请求：
    case ENUM_MSG_TYPE_LOGIN_REQUEST:
    {

        //获得用户名和密码
        char caName[32] = {'\0'};
        char caPwd[32] = {'\0'};
        strncpy(caName,pdu->caData,32);
        strncpy(caPwd,pdu->caData+32,32);

        //去数据库处理
        bool ret = operateDB::getInstance().handleLogin(caName,caPwd);

        PDU* respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;

        //判断数据库处理是否成功：
        if(ret)
        {
            strcpy(respdu->caData,LOGIN_OK);
            m_Name = caName;
        }
        else
        {
            strcpy(respdu->caData,LOGIN_FAILED);
        }

        //把结果返回给客户端:
        write((char *)respdu,respdu->uiPDULen);//将大小为pdu->uiPDULen的数据发送出去
        free(respdu);//发送完释放空间
        respdu = NULL;

        break;
    }

    //如果是在线用户请求:
    case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:
    {
        //创建一个字符串列表: 接收数据库内在线用户名
        QStringList ret = operateDB::getInstance().handleAllOnline();

        //
        uint uiMsgLen = ret.size()*32;

        PDU *respdu = mkPDU(uiMsgLen);
        respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;

        //将所有在线名字拷贝到caMsg中:
        for(int i = 0; i<ret.size(); i++)
        {
            memcpy((char*)(respdu->caMsg)+i*32,ret.at(i).toStdString().c_str(),ret.at(i).size());
        }

        //将拷贝好后的respdu发送给客户端:
        write((char*)respdu,respdu->uiPDULen);

        free(respdu);
        respdu = NULL;

        break;
    }

    //如果是查找用户请求:
    case ENUM_MSG_TYPE_SEARCH_USER_REQUEST:
    {
        int ret = operateDB::handleSearchUser(pdu->caData);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USER_RESPOND;
        if(-1 == ret)
        {
            strcpy(respdu->caData,SEARCH_USER_FAILED);
        }
        else if(1 == ret)
        {
            strcpy(respdu->caData,SEARCH_USER_ONLINE);
        }
        else if(0 == ret)
        {
            strcpy(respdu->caData,SEARCH_USER_OFFLINE);
        }

        //写入数据:
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }

    //如果是添加好友请求:
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
    {
        char Name[32]={'\0'};
        strncpy(Name,pdu->caData+32,32);
        MyTcpServer::getInstance().resend(Name);
        break;
    }

    //如果是刷新好友列表请求：
    case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST:
    {
        char Name[32]={'\0'};
        strncpy(Name,pdu->caData+32,32);

        //进入数据库，查找Name对应的好友：
        QStringList ret = operateDB::getInstance().handleFlushFriend(Name);
        uint uiMsgLen = ret.size()*32;
        PDU* respdu = mkPDU(uiMsgLen);
        respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
        for(int i=0;i<ret.size();i++)
        {
            memcpy((char*)(respdu->caData)+i*32,ret.at(i).toStdString().c_str(),ret.at(i).size());
        }
        write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }

    case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
    {

    }

    default:
        break;
    }

    free(pdu);//发送完释放空间
    pdu = NULL;


    // qDebug() << caName << caPwd <<pdu->uiMsgType;


    //qDebug() << pdu->uiMsgType << (char*)(pdu->caMsg);

}

void MyTcpSocket::clientOffline()
{
    //1.在线状态设置为非在线状态
    operateDB::getInstance().handleOffline(m_Name.toStdString().c_str());

    //2.发送offline信号
    emit offline(this);
}

void MyTcpSocket::handleDownLoadFileData()
{
    m_pTimer->stop(); // 停止计时器
    // 循环传输数据
    char *pBuffer = new char[4096];
    qint64 iActualSize = 0; // 实际读取文件大小

    while(true)
    {
        iActualSize = m_pDownloadFile->read(pBuffer, 4096);
        if (iActualSize > 0 && iActualSize <= 4096)
        {
            this -> write(pBuffer, iActualSize);
        }
        else if (iActualSize == 0)
        { // 发送完成
            break;
        }
        else
        {
            qDebug() << "发送文件数据给客户端出错！";
            break;
        }
    }

    m_pDownloadFile -> close(); // 关闭文件
    delete [] pBuffer;
    pBuffer = NULL;
    m_pDownloadFile->setFileName(""); // 清除上传文件夹名，以免影响之后上传操作
}

PDU* handleDeleteFriendRequest(PDU* pdu)
{
    char deleteName[32] = {'\0'};
    char sourceName[32] = {'\0'};

    //拷贝读取信息：
    strncpy(deleteName,pdu->caData,32);
    strncpy(sourceName,pdu->caData+32,32);
    bool ret = operateDB::getInstance().handleDeleteFriend(deleteName,sourceName);

    //给请求删除方消息提示，以返回值形式
    PDU* resPdu = mkPDU(0);
    resPdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
    if(ret)
    {
        strncpy(resPdu->caData,DELETE_FRIEND_OK,32);
    }
    else
    {
        strncpy(resPdu->caData,DELETE_FRIEND_FAILED,32);
    }

    //给被删除方消息提示（在线时）
    MyTcpServer::getInstance().forwardMsg(deleteName,pdu);
    return resPdu;
}

//私聊发送消息请求：
PDU* handlePrivateChatRequest(PDU* pdu)
{
    char chatedName[32] = {'\0'};
    char sourceName[32] = {'\0'};

    //拷贝读取的信息
    strncpy(chatedName,pdu->caData,32);
    strncpy(sourceName,pdu->caData+32,32);

    PDU* resPdu = NULL;

    //转发给对方消息，0代表对方下线，1代表对方在线
    bool ret = MyTcpServer::getInstance().forwardMsg(chatedName,pdu);

    //发送失败则给发送者消息
    if(!ret)
    {
        resPdu = mkPDU(0);
        resPdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND;
        strncpy(resPdu->caData,PRIVATE_CHAT_OFFLINE);
    }

    return resPdu;
}

//群聊发送消息请求：
void handleGroupChatRequest(PDU* pdu)
{
    QStringList strList = operateDB::getInstance().handleFlushFriend(pdu->caData);

    for(QString strName : strList)
    {
        MyTcpServer::getInstance().forwardMsg(strMsg,pdu);
    }
}

//注册请求：
PDU* handleRegistRequest(PDU* pdu)
{
    char caName[32] = {'\0'};
    char caPwd[32] = {'\0'};

    //拷贝读取到的信息
    strncpy(caName,pdu->caData,32);
    strncpy(caPwd,pdu->caData+32,32);

    //处理请求，插入数据库
    bool ret = operateDB::getInstance().handleRegist(caName,caPwd);

    //响应客户端
    PDU* resPdu = mkPDU(0);
    resPdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
    if(ret)
    {
        strncpy(resPdu->caData,REGIST_OK);

        //注册成功，为新用户按用户名创建文件夹
        QDir dir;
        ret = dir.mkdir(QString("%1/%2").arg(MyTcpServer::getInstance().getRootPath()).arg(caName));
        qDebug()<<"创建新用户文件夹"<<ret;
    }
    if(!ret)
    {
        strncpy(resPdu->caData,REGIST_FAILED);
    }

    return resPdu;
}

PDU* handleLoginRequest(PDU* pdu,QString &m_strName)
{
    char caName[32] = {'\0'};
    char caPwd[32] = {'\0'};

    strncpy(caName,pdu->caData,32);
    strncpy(caPwd,pdu->caData+32,32);

    bool ret = operateDB::getInstance().handleLogin(caName,caPwd);

    //响应客户端
    PDU* resPdu = NULL;
    if(ret)
    {
        QString strUserRootPath = QString("%1/%2").arg(MyTcpServer::getInstance().getRootPath()).arg(caName);
        qDebug() << "登录用户的路径:" << strUserRootPath;
        resPdu = mkPDU(strUserRootPath.size()+1);
        memcpy(resPdu->caData,LOGIN_OK,32);
        memcpy(resPdu->caData+32,caName,32);

        //登陆成功时，记住socket对应的用户名
        m_strName = caName;

        //返回用户根目录：
        strncpy((char*)pdu->caMsg,strUserRootPath.toStdString().c_str(),strUserRootPath.size()+1);
    }
    else
    {
        resPdu = mkPDU(0);
        strncpy(resPdu->caData,LOGIN_OK);
    }
    resPdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
    qDebug() << "登录处理：" << resPdu->uiMsgType << " " << resPdu->caData << " " << resPdu->caData+32;

    return resPdu;
}

// 创建文件夹请求处理
PDU* handleCreateDirRequest(PDU* pdu)
{
    char caDirName[32];
    char caCurPath[pdu -> uiMsgLen];
    strncpy(caDirName, pdu -> caData, 32);
    strncpy(caCurPath, (char*)pdu -> caMsg, pdu -> uiMsgLen);

    QString strDir = QString("%1/%2").arg(caCurPath).arg(caDirName);
    QDir dir;
    PDU *resPdu = mkPDU(0);
    resPdu -> uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;

    qDebug() << "创建文件夹：" << strDir;
    if(dir.exists(caCurPath)) // 路径存在
    {
        if(dir.exists(strDir)) // 文件夹已经存在
        {
            strncpy(resPdu -> caData, CREATE_DIR_EXIST, 32);
        }
        else
        {
            dir.mkdir(strDir); // 创建文件夹
            strncpy(resPdu -> caData, CREATE_DIR_OK, 32);
        }
    }
    else // 路径不存在
    {
        strncpy(resPdu -> caData, PATH_NOT_EXIST, 32);
    }

    return resPdu;
}

// 刷新文件夹请求处理
PDU* handleFlushDirRequest(PDU* pdu)
{
    char caCurDir[pdu -> uiMsgLen];
    memcpy(caCurDir, (char*)pdu -> caMsg, pdu -> uiMsgLen);
    qDebug() << "刷新文件夹：" << caCurDir;
    QDir dir;
    PDU* resPdu = NULL;

    if(!dir.exists(caCurDir)) // 请求文件夹不存在
    {
        resPdu = mkPDU(0);
        strncpy(resPdu -> caData, PATH_NOT_EXIST, 32);
    }
    else // 存在
    {
        dir.setPath(caCurDir); // 设置为当前目录
        QFileInfoList fileInfoList = dir.entryInfoList(); // 获取当前目录下所有文件
        int iFileNum = fileInfoList.size();

        resPdu = mkPDU(sizeof(FileInfo) * iFileNum);
        FileInfo *pFileInfo = NULL; // 创建一个文件信息结构体指针，方便之后遍历PDU空间来赋值
        strncpy(resPdu -> caData, FLUSH_DIR_OK, 32);

        for(int i = 0; i < iFileNum; ++ i)
        {
            pFileInfo = (FileInfo*)(resPdu -> caMsg) + i; // 通过指针指向，直接修改PDU空间值，每次偏移FileInfo大小
            memcpy(pFileInfo -> caName, fileInfoList[i].fileName().toStdString().c_str(), fileInfoList[i].fileName().size());
            pFileInfo -> bIsDir = fileInfoList[i].isDir();
            pFileInfo -> uiSize = fileInfoList[i].size();
            QDateTime dtLastTime = fileInfoList[i].lastModified(); // 获取文件最后修改时间
            QString strLastTime = dtLastTime.toString("yyyy/MM/dd hh:mm");
            memcpy(pFileInfo -> caTime, strLastTime.toStdString().c_str(), strLastTime.size());
            qDebug() << "文件信息：" << pFileInfo -> caName << " " << pFileInfo -> bIsDir << " " << pFileInfo -> uiSize << " " << pFileInfo -> caTime;
        }
    }
    resPdu -> uiMsgType = ENUM_MSG_TYPE_FLUSH_DIR_RESPOND;

    return resPdu;
}

// 删除文件或文件夹处理
PDU* handleDelFileOrDirRequest(PDU* pdu)
{
    PDU* resPdu = mkPDU(0);
    char strDelPath[pdu -> uiMsgLen];
    memcpy(strDelPath, (char*)pdu -> caMsg, pdu -> uiMsgLen);
    qDebug() << "删除文件：" << strDelPath;
    QDir dir;

    resPdu -> uiMsgType = ENUM_MSG_TYPE_DELETE_FILE_RESPOND;
    if(!dir.exists(strDelPath)) // 路径不存在
    {
        strncpy(resPdu -> caData, PATH_NOT_EXIST, 32);
    }
    else
    {
        bool ret = false;

        QFileInfo fileInfo(strDelPath);
        if(fileInfo.isDir()) // 是文件目录
        {
            dir.setPath(strDelPath);
            ret = dir.removeRecursively();
        }
        else if(fileInfo.isFile())
        {
            ret = dir.remove(strDelPath);
        }
        if(ret)
        {
            strncpy(resPdu -> caData, DELETE_FILE_OK, 32);
        }
        else
        {
            strncpy(resPdu -> caData, DELETE_FILE_FAILED, 32);
        }
    }
    qDebug() << resPdu -> caData;

    return resPdu;
}

// 重命名文件或文件夹请求处理
PDU* handleRenameFileRequest(PDU* pdu)
{
    PDU* resPdu = mkPDU(0);
    char caCurPath[pdu -> uiMsgLen];
    char caOldName[32]; // 旧文件名
    char caNewName[32]; // 新文件名
    memcpy(caCurPath, (char*)pdu -> caMsg, pdu -> uiMsgLen);
    strncpy(caOldName, pdu -> caData, 32);
    strncpy(caNewName, pdu -> caData + 32, 32);
    qDebug() << "重命名文件：" << caCurPath << " " << caOldName << " -> " << caNewName;
    QDir dir;

    resPdu -> uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_RESPOND;
    dir.setPath(caCurPath);
    if(dir.rename(caOldName, caNewName))
    {
        strncpy(resPdu -> caData, RENAME_FILE_OK, 32);
    }
    else
    {
        strncpy(resPdu -> caData, RENAME_FILE_FAILED, 32);
    }
    qDebug() << resPdu -> caData;

    return resPdu;
}

// 进入文件夹请求处理
PDU* handleEntryDirRequest(PDU* pdu)
{
    char strEntryPath[pdu -> uiMsgLen]; // 进入文件夹路径
    memcpy(strEntryPath, (char*)pdu -> caMsg, pdu -> uiMsgLen);
    qDebug() << "进入 " << strEntryPath;
    PDU* resPdu = NULL;
    QDir dir(strEntryPath);

    if(!dir.exists()) // 请求文件夹不存在
    {
        resPdu = mkPDU(0);
        strncpy(resPdu -> caData, PATH_NOT_EXIST, 32);
    }
    else // 存在
    {
        QFileInfo fileInfo(strEntryPath);
        if(!fileInfo.isDir()) // 不是文件夹
        {
            resPdu = mkPDU(0);
            strncpy(resPdu -> caData, ENTRY_DIR_FAILED, 32);
        }
        else
        {
            resPdu = handleFlushDirRequest(pdu); // 通过该函数获取文件夹下内容
        }
    }
    resPdu -> uiMsgType = ENUM_MSG_TYPE_ENTRY_DIR_RESPOND;
    qDebug() << "1 resPdu -> caData ：" << resPdu -> caData;
    if(strcmp(resPdu -> caData, FLUSH_DIR_OK) == 0)
    {
        strncpy(resPdu -> caData, ENTRY_DIR_OK, 32);
        qDebug() << "2 resPdu -> caData ：" << resPdu -> caData;
    }
    else
    {
        strncpy(resPdu -> caData, ENTRY_DIR_FAILED, 32);
        qDebug() << "2 resPdu -> caData ：" << resPdu -> caData;
    }

    return resPdu;
}

// 返回上一目录请求
PDU* handlePreDirRequest(PDU* pdu)
{
    char strPrePath[pdu -> uiMsgLen]; // 进入文件夹路径
    memcpy(strPrePath, (char*)pdu -> caMsg, pdu -> uiMsgLen);
    qDebug() << "上一目录： " << strPrePath;
    PDU* resPdu = NULL;
    QDir dir(strPrePath);

    if(!dir.exists()) // 请求文件夹不存在
    {
        resPdu = mkPDU(0);
        strncpy(resPdu -> caData, PATH_NOT_EXIST, 32);
    }
    else // 存在
    {
        resPdu = handleFlushDirRequest(pdu); // 通过该函数获取文件夹下内容
    }
    resPdu -> uiMsgType = ENUM_MSG_TYPE_PRE_DIR_RESPOND;
    qDebug() << "1 resPdu -> caData ：" << resPdu -> caData;
    if(strcmp(resPdu -> caData, FLUSH_DIR_OK) == 0)
    {
        strncpy(resPdu -> caData, PRE_DIR_OK, 32);
        qDebug() << "2 resPdu -> caData ：" << resPdu -> caData;
    }
    else
    {
        strncpy(resPdu -> caData, PRE_DIR_FAILED, 32);
        qDebug() << "2 resPdu -> caData ：" << resPdu -> caData;
    }

    return resPdu;
}

// 上传文件请求处理
PDU* handleUploadFileRequest(PDU* pdu, TransFile* transFile)
{
    char caCurPath[pdu -> uiMsgLen];
    char caFileName[32] = {'\0'};
    qint64 fileSize = 0;

    strncpy(caCurPath, (char*)pdu -> caMsg, pdu -> uiMsgLen);
    sscanf(pdu -> caData, "%s %lld", caFileName, &fileSize);
    QString strFilePath = QString("%1/%2").arg(caCurPath).arg(caFileName); // 文件路径
    qDebug() << "上传文件路径：" << strFilePath;

    PDU* resPdu = mkPDU(0);
    resPdu -> uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;

    transFile->file.setFileName(strFilePath); // 设置要上传的文件名
    if(transFile->file.open(QIODevice::WriteOnly)) // 以只写的方式打开文件，文件如果不存在会自动创建
    {
        transFile->bTransform = true; // 正在上传文件状态
        transFile->iTotalSize = fileSize;
        transFile->iReceivedSize = 0;

        memcpy(resPdu -> caData, UPLOAD_FILE_START, 32);
    }
    else // 打开文件失败
    {
        memcpy(resPdu -> caData, UPLOAD_FILE_FAILED, 32);
    }

    return resPdu;
}

// 下载文件请求处理
PDU* handleDownloadFileRequest(PDU* pdu, QFile *fDownloadFile, QTimer *pTimer)
{
    char caFileName[32] = {'\0'};
    char caCurPath[pdu -> uiMsgLen];
    memcpy(caFileName, pdu -> caData, 32);
    memcpy(caCurPath, (char*)pdu -> caMsg, pdu -> uiMsgLen);
    QString strDownloadFilePath = QString("%1/%2").arg(caCurPath).arg(caFileName);
    fDownloadFile->setFileName(strDownloadFilePath);

    qDebug() << "下载文件：" << strDownloadFilePath;
    qint64 fileSize = fDownloadFile -> size();

    PDU *resPdu = NULL;
    if(fDownloadFile->open(QIODevice::ReadOnly))
    {
        resPdu = mkPDU(32 + sizeof (qint64) + 5);

        resPdu -> uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;
        strncpy(resPdu -> caData, DOWNLOAD_FILE_START, 32);
        sprintf((char*)resPdu -> caMsg, "%s %lld", caFileName, fileSize);
        pTimer -> start(1000); // 开始计时器1000ms
        qDebug() << (char*)resPdu -> caMsg;
    }
    else // 打开文件失败
    {
        resPdu = mkPDU(0);

        resPdu -> uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;
        strncpy(resPdu -> caData, DOWNLOAD_FILE_FAILED, 32);
    }

    return resPdu;
}

// 移动文件请求处理
PDU* handleMoveFileRequest(PDU* pdu)
{
    char caMoveFileName[32]; // 要移动文件名
    int iOldDirSize = 0;
    int iDesDirSize = 0;
    sscanf(pdu -> caData, "%s %d %d", caMoveFileName, &iDesDirSize, &iOldDirSize);
    char caOldDir[iOldDirSize + 33]; // +33是为了拼接文件名
    char caDesDir[iDesDirSize + 33];
    sscanf((char*)pdu -> caMsg, "%s %s", caDesDir, caOldDir);
    qDebug() << "移动文件：" << caMoveFileName << "从" << caOldDir << "到" << caDesDir;

    QFileInfo fileInfo(caDesDir);
    PDU* resPdu = mkPDU(0);
    resPdu -> uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_RESPOND;

    if(!fileInfo.isDir())
    {
        strncpy(resPdu -> caData, MOVE_FILE_FAILED, 32);
        return resPdu;
    }

    // 拼接文件名
    strcat(caOldDir, "/");
    strcat(caOldDir, caMoveFileName);
    strcat(caDesDir, "/");
    strcat(caDesDir, caMoveFileName);
    if (QFile::rename(caOldDir, caDesDir)) // 移动
    {
        strncpy(resPdu -> caData, MOVE_FILE_OK, 32);
    }
    else
    {
        strncpy(resPdu -> caData, MOVE_FILE_FAILED, 32);
    }

    return resPdu;
}

// 分享文件请求处理
PDU* handleShareFileRequest(PDU* pdu, QString strSouName)
{
    int iUserNum = 0; // 分享好友数
    char caFileName[32]; // 分享的文件名
    sscanf(pdu -> caData, "%s %d", caFileName, &iUserNum);
    qDebug() << "分享文件：" << caFileName << " 人数：" << iUserNum;

    // 转发给被分享的好友分享文件通知
    const int iFilePathLen = pdu->uiMsgLen - iUserNum * 32;
    char caFilePath[iFilePathLen];
    PDU* resPdu = mkPDU(iFilePathLen);
    resPdu -> uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE;
    memcpy(resPdu -> caData, strSouName.toStdString().c_str(), strSouName.size()); // 发送方
    memcpy(resPdu -> caData + 32, caFileName, 32); // 发送文件名
    memcpy(caFilePath, (char*)(pdu -> caMsg) + 32 * iUserNum, iFilePathLen);
    memcpy((char*)resPdu -> caMsg, caFilePath, iFilePathLen); // 发送文件路径
    // 遍历分享所有要接收文件的好友
    char caDesName[32]; // 目标好友名
    for(int i = 0; i < iUserNum; ++ i)
    {
        memcpy(caDesName, (char*)(pdu -> caMsg) + 32 * i, 32);
        MyTcpServer::getInstance().forwardMsg(caDesName, resPdu);
        qDebug() << caDesName;
    }
    free(resPdu);
    resPdu = NULL;

    // 回复发送方消息
    resPdu = mkPDU(0);
    resPdu -> uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
    strncpy(resPdu -> caData, SHARE_FILE_OK, 32);

    return resPdu;
}

// 复制文件夹
bool copyDir(QString strOldPath, QString strNewPath)
{
    int ret = true;
    QDir dir; // 目录操作

    qDebug() << "分享目录：" << strOldPath << " " << strNewPath;
    dir.mkdir(strNewPath); // 新路径创建空目录
    dir.setPath(strOldPath); // 设置为源目录
    QFileInfoList fileInfoList = dir.entryInfoList(); // 获得源目录下文件列表
    // 对源目录下所有文件（分为普通文件、文件夹）进行递归拷贝
    QString strOldFile;
    QString strNewFile;
    for(QFileInfo fileInfo:fileInfoList)
    {
        if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
        { // 注意不要忘记这个判断，"."和".."文件夹不用复制，不然会死循环
            continue;
        }
        strOldFile = QString("%1/%2").arg(strOldPath).arg(fileInfo.fileName());
        strNewFile = QString("%1/%2").arg(strNewPath).arg(fileInfo.fileName());
        if(fileInfo.isFile())
        {
            ret = ret && QFile::copy(strOldFile, strNewFile);
        }
        else if(fileInfo.isDir())
        {
            ret = ret && copyDir(strOldFile, strNewFile);
        }
        qDebug() << strOldFile << " -> " << strNewFile;
    }

    return ret;
}

// 分享文件通知响应处理
PDU* handleShareFileNoteRespond(PDU *pdu)
{
    int iOldPathLen = 0;
    int iNewPathLen = 0;
    sscanf(pdu -> caData, "%d %d", &iOldPathLen, &iNewPathLen);
    char caOldPath[iOldPathLen];
    char caNewDir[iNewPathLen];
    sscanf((char*)pdu -> caMsg, "%s %s", caOldPath, caNewDir);

    // 获得文件新的路径
    char *pIndex = strrchr(caOldPath, '/'); // 获得最右侧的/的指针，找到文件名
    QString strNewPath = QString("%1/%2").arg(caNewDir).arg(pIndex + 1);
    qDebug() << "同意分享文件：" << caOldPath << " " << strNewPath;

    QFileInfo fileInfo(caOldPath);
    bool ret = false;
    if(fileInfo.isFile())
    {
        ret = QFile::copy(caOldPath, strNewPath);
    }
    else if(fileInfo.isDir())
    {
        ret = copyDir(caOldPath, strNewPath);
    }
    else
    {
        ret = false;
    }
    // 回复接收方
    PDU* resPdu = mkPDU(0);
    resPdu -> uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND;
    if(ret)
    {
        memcpy(resPdu -> caData, SHARE_FILE_OK, 32);
    }
    else
    {
        memcpy(resPdu -> caData, SHARE_FILE_FAILED, 32);
    }

    return resPdu;
}









