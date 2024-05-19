#ifndef OPERATEWIDGET_H
#define OPERATEWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QStackedWidget>//堆栈窗口
#include <QLabel>

#include "friend.h"
#include "book.h"
#include "filesystem.h"


class operateWidget : public QWidget
{
    Q_OBJECT
public:
    //构造函数：
    explicit operateWidget(QWidget *parent = nullptr);

    //使用单例模型：
    static operateWidget &getInstance();

    //得到m_Friend属性函数：
    Friend *getFriend();

    void setuserLable(const char* name);

    fileSystem* getFileSystem() const;
signals:


public slots:


private:
    QLabel *m_UserLable;
    QListWidget *m_listWidget;      //组件列表
    Friend *m_Friend;               //好友页面
    Book *m_Book;                   //书籍
    fileSystem* m_FileSystem;       //文件页面
    QStackedWidget *m_StackWidget;  //容器，每次显示一个页面
};

#endif // OPERATEWIDGET_H
