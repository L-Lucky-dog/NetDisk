#include "operatedb.h"
#include <QMessageBox>
#include <QDebug>
#include <QWidget>


operateDB::operateDB(QObject *parent) : QObject{parent}
{
    m_db = QSqlDatabase::addDatabase("QMYSQL");
}

operateDB &operateDB::getInstance()
{
    static operateDB instance;
    return instance;
}

void operateDB::init()
{
    m_db.setHostName("localhost");//数据库服务器IP
    m_db.setUserName("root");//数据库用户名
    m_db.setPassword("root");//数据库密码
    m_db.setDatabaseName("D:\QT WorkSpace\TcpServer\cloud.sql");

    if(m_db.open())//数据库是否打开？
    {
        QSqlQuery query;
        query.exec("select * from userInfo");
        while(query.next())
        {
            QString data = QString("%1,%2,%3").arg(query.value(0).toString()).arg(query.value(1).toString()).arg(query.value(2).toString());
            qDebug() << data;
        }
    }
    else
    {
        QMessageBox::critical(NULL,"打开数据库","打开数据库失败");
    }
}

bool operateDB::handleRegist(const char *Name, const char *Pwd)
{
    //考虑形参有效性：
    if(NULL == Name || NULL == Pwd)
    {
        return false;
    }

    QString data = QString("INSERT INTO userInfo(Name,Pwd) VALUES('%1','%2')").arg(Name).arg(Pwd);
    QSqlQuery query;
    return query.exec(data);//要执行的语句
}

bool operateDB::handleLogin(const char *Name, const char *Pwd)
{
    //考虑形参有效性：
    if(NULL == Name || NULL == Pwd)
    {
        return false;
    }

    QString data = QString("SELECT * FROM userInfo WHERE Name=\'%1\' and Pwd=\'%2\' and online").arg(Name).arg(Pwd);
    qDebug() << data;
    QSqlQuery query;
    query.exec(data);//要执行的语句

    if(query.next())
    {
        //将online定为1:表示已经登录了
        data = QString("UPDATE userInfo SET online=1 WHERE Name=\'%1\' and Pwd=\'%2\' and online").arg(Name).arg(Pwd);
        qDebug() << data;
        QSqlQuery query;
        query.exec(data);//要执行的语句
        return true;
    }
    else
    {
        return false;
    }
}

void operateDB::handleOffline(const char *Name)
{
    //考虑形参有效性：
    if(NULL == Name)
    {
        return;
    }

    //状态设置为0
    QString data = QString("update userInfo set online=0 where Name=\'%1\'").arg(Name);

    QSqlQuery query;
    query.exec(data);//要执行的语句


}

int operateDB::handleSearchUser(const char *Name)
{
    if(NULL == Name)
    {
        return -1;
    }

    //确认用户在线的条件: online = 1
    QString data = QString("select Online from userInfo where name=\'%1\'").arg(Name);

    //数据库查询语句:
    QSqlQuery query;

    //要执行的语句:
    query.exec(data);

    if(query.next())
    {
        int ret = query.value(0).toInt();
        if(1 == ret)
        {
            return 1;
        }
        else if(0 == ret)
        {
            return 0;
        }
    }
    else
    {
        return -1;
    }
}

int operateDB::handleAddFriend(const char *PerName, const char *LoginName)
{
    if(NULL == PerName || NULL == LoginName){
        return -1;
    }
    QString data = QString("select * from friend where(id=(select id from userInfo where name = \'%1\') and frinedID = (select id from userInfo where name = \'%2\') or id=(select id from userInfo where name = \'%3\') and frinedID = (select id from userInfo where name = \'%4\'))").arg(PerName).arg(LoginName);

}

QStringList operateDB::handleAllOnline()
{
    //确认用户在线的条件: online = 1
    QString data = QString("select Name from userInfo where online=1");

    QSqlQuery query;//查询语句

    query.exec(data);//要执行的语句
    QStringList result;//声明一个字符串列表
    result.clear();

    //将所有数据库内online = 1的用户名拷贝到result上, 返回result
    while(query.next())
    {
        result.append(query.value(0).toString());
    }
    return result;
}

bool operateDB::handleDeleteFriend(const char *deleteName, const char *sourceName)
{
    if(deleteName == NULL || sourceName == NULL)
    {
        return false;
    }

    //先查出deleteName与sourceName对应的id
    int iDelID = -1;
    int iSouID = -1;
    QString strQuery = QString("select id from userInfo where name in (\'%1\', \'%2\')").arg(deleteName).arg(sourceName);
    QSqlQuery query;

    query.exec(strQuery);
    if(query.next())
    {
        iDelID = query.value(0).toInt();
        iSouID = query.value(0).toInt();
    }

    //删除好友信息表中两个id之间的好友关系
    strQuery = QString("delete from friendInfo where id in (\'%1\',\'%2\') and friendID in (\'%3\',\'%4\')").arg(iDelID).arg(iSouID).arg(iDelID).arg(iSouID);
    return query.exec(strQuery);
}

QStringList operateDB::handleFlushFriend(const char *name)
{
    QStringList friendList;
    friendList.clear();
    if(NULL == name)
    {
        return friendList;
    }
    QString data = QString("select name from userInfo where online=1 and ID=(select id from friend where friendID=(select id from userInfo where name=\'%1\'))").arg(name);
    QSqlQuery query;
    query.exec(data);
    while(query.next())
    {
        friendList.append(query.value(0).toString());
    }
    //return friendList;

    data = QString("select name from userInfo where online=1 and ID=(select friendID from friend where id=(select id from userInfo where name=\'%1\'))").arg(name);
    query.exec(data);
    while(query.next())
    {
        friendList.append(query.value(0).toString());
    }
    return friendList;
}

operateDB::~operateDB()
{
    m_db.close();
}

















