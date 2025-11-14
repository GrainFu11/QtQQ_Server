#pragma once

#include <QtWidgets/QDialog>
#include "ui_QtQQ_Server.h"
#include "TcpServer.h"
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QTimer>
#include <QUDpSocket>
class QtQQ_Server : public QDialog
{
    Q_OBJECT

public:
    QtQQ_Server(QWidget *parent = nullptr);
    ~QtQQ_Server();
    bool connectMySql();
private slots:
    void onUDPbrodMsg(QByteArray& btData);
    void onRefresh();
    void on_queryDepartmentBtn_clicked();   //点击信号与槽函数自动连接
    void on_queryIDBtn_clicked();           //根据用户qq号查询
    void on_logoutBtn_clicked();            //注销用户号
    void on_selectPictureBtn_clicked();    //选择用户头像
    void on_addBtn_clicked();               //新增用户
private:
    Ui::QtQQ_ServerClass ui;
    TcpServer* m_tcpServer;                 //TCP服务端
    QUdpSocket* m_udpSender;                //UPD广播
    void initUdpSocket();
    void initTcpSocket();
    int getCompDepID();
    void setDepNameMap();
    void setStatusMap();
    void setOnlineMap();
    void initComboBoxData();            //初始化组合框的数据

    QSqlQueryModel m_queryInfoModel;   //查询所有员工的信息模型
    void updateTableData(int depId=0,int employeeID=0);
    int m_compDepID;                   //公司群qq号
    QMap<QString, QString>m_statusMap;
    QMap<QString, QString>m_depNameMap;
    QMap<QString, QString>m_onlineMap;
    QTimer* m_timer;
    int m_depID;            //部门号
    int m_employeeID;       //用户qq号
    QString m_pixPath;      //头像照片路径

};

