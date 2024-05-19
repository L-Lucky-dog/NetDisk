#ifndef ONLINE_H
#define ONLINE_H

#include <QWidget>
#include "protocol.h"


namespace Ui {
class Online;
}

class Online : public QWidget
{
    Q_OBJECT

public:
    //构造函数：
    explicit Online(QWidget *parent = nullptr);

    //析构函数：
    ~Online();

    //显示用户函数：
    void showUser(PDU *pdu);

private slots:
    //槽函数：添加好友
    void on_addFriend_pb_clicked();

private:
    Ui::Online *ui;     //online.ui界面
};

#endif // ONLINE_H
