#include "operatewidget.h"


operateWidget::operateWidget(QWidget *parent) : QWidget{parent}
{
    m_listWidget = new QListWidget(this);
    m_listWidget->addItem("好友");
    m_listWidget->addItem("图书");

    //下面2个界面，每次只能显示一个：利用堆栈窗口实现功能
    m_Friend = new Friend;
    m_Book = new Book;

    m_StackWidget = new QStackedWidget;
    m_StackWidget->addWidget(m_Friend);
    m_StackWidget->addWidget(m_Book);

    QHBoxLayout *Main = new QHBoxLayout;
    Main->addWidget(m_listWidget);
    Main->addWidget(m_StackWidget);

    setLayout(Main);

    connect(m_listWidget,SIGNAL(currentRowChanged(int)),m_StackWidget,SLOT(setCurrentIndex(int)));

}

operateWidget &operateWidget::getInstance()
{
    static operateWidget instance;
    return instance;
}

Friend *operateWidget::getFriend()
{
    return m_Friend;
}

void operateWidget::setuserLable(const char *name)
{
    m_UserLable->setText(name);
}

fileSystem* operateWidget::getFileSystem() const
{
    return m_FileSystem;
}

























