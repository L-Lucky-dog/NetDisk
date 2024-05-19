#ifndef OPERATEDB_H
#define OPERATEDB_H

#include <QObject>
#include <QSqlDatabase>//连接数据库
#include <QSqlQuery>//查询数据库
#include <QStringList>//字符串列表

class operateDB : public QObject
{
    Q_OBJECT
public:
    explicit operateDB(QObject *parent = nullptr);

    //静态函数：
    static operateDB &getInstance();

    //数据库初始化函数：
    void init();

    //处理注册的函数:
    bool handleRegist(const char* Name, const char* Pwd);

    //处理登录的函数:
    bool handleLogin(const char* Name, const char* Pwd);

    //处理下线的函数:
    void handleOffline(const char* Name);

    //处理查找的函数:
    int handleSearchUser(const char* Name);

    //处理添加好友的函数:
    int handleAddFriend(const char* PerName, const char* LoginName);

    //处理刷新好友列表的函数：
    QStringList handleFlushFriend(const char* name);

    //处理所有在线用户的函数：
    QStringList handleAllOnline();

    //处理好友删除的函数：
    bool handleDeleteFriend(const char* deleteName,const char* sourceName);

    //析构函数：
    ~operateDB();

signals:






public slots:






private:

    QSqlDatabase m_db;




};

#endif // OPERATEDB_H
