#include "QtQQ_Server.h"
#include <QSqlDatabase>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QSqlError>
#include <QSqlRecord>
#include <QFileDialog>
const int gTcpPort = 8888;
const int gUdpPort = 6666;
QtQQ_Server::QtQQ_Server(QWidget *parent)
    : QDialog(parent),m_pixPath("")
{
    ui.setupUi(this);
    if (!connectMySql()) {
        QMessageBox::information(this, "Tip", "Failed to connect to the database");
        close();
        return;
    }
    m_queryInfoModel.setQuery(QString("SELECT * FROM tab_employee"));
    ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);//表格只读
    setDepNameMap();
    setOnlineMap();
    setStatusMap();
    initComboBoxData();
    //初始化查询公司群所有员工信息
    m_employeeID = 0;
    m_depID = getCompDepID();
    m_compDepID = m_depID;

    updateTableData();

    //定时更新界面
    m_timer = new QTimer(this);
    m_timer->setInterval(200);
    m_timer->start();
    connect(m_timer, &QTimer::timeout, this, &QtQQ_Server::onRefresh);

    initTcpSocket();
    initUdpSocket();
}

QtQQ_Server::~QtQQ_Server()
{}

bool QtQQ_Server::connectMySql()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName("qtqq");
    db.setHostName("localhost");
    db.setUserName("root");
    db.setPassword("");
    db.setPort(3306);

    if (db.open()) {
        return true;
    }
    else {
        // 打印详细错误
        qDebug() << "数据库打开失败：" << db.lastError().text();
        QMessageBox::critical(this, "错误", db.lastError().text());
        return false;
    }
}

void QtQQ_Server::onRefresh()
{
    updateTableData(m_depID, m_employeeID);
}

void QtQQ_Server::on_queryDepartmentBtn_clicked()
{
    ui.queryIDLineEdit->clear();
    m_employeeID = 0;
    m_depID = ui.departmentBox->currentData().toInt();
    updateTableData(m_depID,m_employeeID);

}

void QtQQ_Server::on_queryIDBtn_clicked()
{
    ui.departmentBox->setCurrentIndex(0);
    m_depID = m_compDepID;
    if (ui.queryIDLineEdit->text().isEmpty()) {
        QMessageBox::information(this, "Tips", "Please enter the user's qq number!");
        ui.queryIDLineEdit->setFocus();
        return;
    }
    //获取用户输入的qq号
    int employeeID = ui.queryIDLineEdit->text().toInt();
    QSqlQuery queryInfo;
    queryInfo.prepare("SELECT * FROM tab_employee WHERE employeeID=?");
    queryInfo.addBindValue(employeeID);
    if (!queryInfo.exec()) {};
    if (queryInfo.next()) {
        m_employeeID = employeeID;
        updateTableData(m_depID, m_compDepID);
    }
    else {
        QMessageBox::information(this, "Tips", "Please enter the correct user qq number!");
        ui.queryIDLineEdit->setFocus();
        return;
    }
    //updateTableData(m_depID, m_employeeID);
}

void QtQQ_Server::on_logoutBtn_clicked()
{
    ui.queryIDLineEdit->clear();
    ui.departmentBox->setCurrentIndex(0);
    //判断是否输入qq号
    if (ui.logoutIDLineEdit->text().isEmpty()) {
        QMessageBox::information(this, "Tips", "Please enter the user's qq number!");
        ui.queryIDLineEdit->setFocus();
        return;
    }

    //获取用户输入的qq号
    int employeeID = ui.logoutIDLineEdit->text().toInt();
    QSqlQuery queryInfo;
    queryInfo.prepare("SELECT * FROM tab_employee WHERE employeeID=?");
    queryInfo.addBindValue(employeeID);
    if (!queryInfo.exec()) {};
    if (!queryInfo.next()) {
        QMessageBox::information(this, "Tips", "Please enter the correct user qq number!");
                ui.logoutIDLineEdit->setFocus();
                return;
    }
    else {
        //注销，更新数据库数据，将状态设置为0
        m_employeeID = employeeID;
        QSqlQuery sqlUpdate;
        sqlUpdate.prepare("UPDATE tab_employee SET status=0 WHERE employeeID=?");
        sqlUpdate.addBindValue(employeeID);
        if(!sqlUpdate.exec()){}
        QMessageBox::information(this, "Tips", QString("The user %1 has successfully logged out!").arg(employeeID));
        updateTableData(m_employeeID, m_depID);
    }


}

void QtQQ_Server::on_selectPictureBtn_clicked()
{
    m_pixPath = QFileDialog::getOpenFileName(this, QString::fromUtf8("选择头像"), ".", "*.png;;*.jpg");
    if (m_pixPath.isEmpty()) {
        return;
    }
    QPixmap pixmap;
    pixmap.load(m_pixPath);
    
    qreal widthRation = (qreal)ui.headLabel->width()/(qreal)pixmap.width();
    qreal heightRation= (qreal)ui.headLabel->height() / (qreal)pixmap.height();

    QSize size(pixmap.width() * widthRation, pixmap.height() * heightRation);
    ui.headLabel->setPixmap(pixmap.scaled(size));

}

void QtQQ_Server::on_addBtn_clicked()
{
    //检测用户姓名的输入
    QString strName = ui.nameLineEdit->text();
    if (strName.isEmpty()) {
        QMessageBox::information(this, "Tips", "Please enter your name!");
        ui.nameLineEdit->setFocus();
        return;
    }
    //检测头像路径
    if (m_pixPath.isEmpty()) {
        QMessageBox::information(this, "Tips", "Please enter your headLabel!");
        return;
    }
    //图片路径设置为\右斜线
    QString strPixpath = m_pixPath;
    strPixpath.replace("/", "\\");
    //数据库插入新的员工数据
    //获取用户qq号
    QSqlQuery queryMaxEmployeeID;
    queryMaxEmployeeID.prepare("SELECT MAX(employeeID) FROM tab_employee");
    if(!queryMaxEmployeeID.exec()){}
    queryMaxEmployeeID.next();
    int employeeId = queryMaxEmployeeID.value(0).toInt()+1;//新用户qq号

    //部门号
    int depID = ui.employeeDepBox->currentData().toInt();

    QSqlQuery insertSql;
    insertSql.prepare("INSERT INTO tab_employee(departmentID,employeeID,employee_name,picture)\
                       VALUES(?,?,?,?)");
    insertSql.addBindValue(depID);
    insertSql.addBindValue(employeeId);
    insertSql.addBindValue(strName);
    insertSql.addBindValue(strPixpath);
    if (!insertSql.exec()) {}
    else {
        QMessageBox::information(this, "Tips", QString("Added user %1 successfully!").arg(strName));
        ui.headLabel->setText("用户头像");
        ui.nameLineEdit->clear();
    }
}

void QtQQ_Server::initUdpSocket()
{
    m_udpSender = new QUdpSocket(this);
}

void QtQQ_Server::initTcpSocket()
{
    m_tcpServer = new TcpServer(gTcpPort);
    m_tcpServer->run();

    //收到tcp客户端发来的信息后进行udp广播
    connect(m_tcpServer, &TcpServer::signalTcpMsgComes, this, &QtQQ_Server::onUDPbrodMsg);
    
}
int QtQQ_Server::getCompDepID()
{
    QSqlQuery queryCompDepID;
    queryCompDepID.prepare("SELECT departmentID FROM tab_department WHERE department_name=?");
    queryCompDepID.addBindValue(QString("公司群"));
    if (!queryCompDepID.exec()){};
    queryCompDepID.next();
    return queryCompDepID.value(0).toInt();
}
void QtQQ_Server::setDepNameMap()
{
    m_depNameMap.insert("2001", "人事群");
    m_depNameMap.insert("2002", "研发群");
    m_depNameMap.insert("2003", "市场群");
}
void QtQQ_Server::setStatusMap()
{
    m_statusMap.insert("1", "有效");
    m_statusMap.insert("0", "注销");
}
void QtQQ_Server::setOnlineMap()
{
    m_onlineMap.insert("1", "离线");
    m_onlineMap.insert("2", "在线");
    m_onlineMap.insert("3", "隐身");
}
void QtQQ_Server::initComboBoxData()
{
    QString itemText;   //组合框项目文本
    //获取公司总的部门数
    QSqlQueryModel queryDepModel;
    queryDepModel.setQuery(QString("SELECT * FROM tab_department"));
    int depCounts = queryDepModel.rowCount() - 1;

    for (int i = 0; i < depCounts; i++) {
        itemText = ui.employeeDepBox->itemText(i);

        QSqlQuery queryDepID;
        queryDepID.prepare("SELECT departmentID FROM tab_department WHERE department_name=?");
        queryDepID.addBindValue(itemText);
        if(!queryDepID.exec()){}
        queryDepID.next();

        //设置用户所属部门组合框的数据为相应部门号
        ui.employeeDepBox->setItemData(i, queryDepID.value(0).toInt()); 
        queryDepID.finish();
    }
    //算上了公司群
    for (int i = 0; i < depCounts + 1; i++) {
        itemText = ui.departmentBox->itemText(i);
        QSqlQuery queryDepID;
        queryDepID.prepare("SELECT departmentID FROM tab_department WHERE department_name=?");
        queryDepID.addBindValue(itemText);
        if (!queryDepID.exec()) {}
        queryDepID.first();

        //设置部门组合框的数据为相应的部门号
        ui.departmentBox->setItemData(i, queryDepID.value(0).toInt());
        queryDepID.finish();
    }

}
void QtQQ_Server::updateTableData(int depId, int employeeID)
{
    ui.tableWidget->clear();

    if (depId&&depId!=m_compDepID) {
        m_queryInfoModel.setQuery(QString("SELECT * FROM tab_employee WHERE departmentID=%1").arg(depId));
    }
    else if (employeeID) {
        m_queryInfoModel.setQuery(QString("SELECT * FROM tab_employee WHERE employeeID=%1").arg(employeeID));
    }
    else{
        m_queryInfoModel.setQuery(QString("SELECT * FROM tab_employee"));
    }

    int rows = m_queryInfoModel.rowCount();
    int columns = m_queryInfoModel.columnCount();
    QModelIndex index;
    ui.tableWidget->setRowCount(rows);
    ui.tableWidget->setColumnCount(columns);

    //设置表头
    QStringList headers;
    headers << QStringLiteral("部门")
        << QStringLiteral("QQ号")
        << QStringLiteral("用户姓名")
        << QStringLiteral("用户签名")
        << QStringLiteral("用户状态")
        << QStringLiteral("用户头像")
        << QStringLiteral("在线状态");
    ui.tableWidget->setHorizontalHeaderLabels(headers);

    //设置列等宽
    ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            index = m_queryInfoModel.index(i, j);
            QString strData = m_queryInfoModel.data(index).toString();
            //获取字段名称
            QSqlRecord record = m_queryInfoModel.record(i);     //当前行的记录
            QString strRecordName = record.fieldName(j);        //列
            if (strRecordName == QStringLiteral("departmentID")) {
              
                ui.tableWidget->setItem(i, j, new QTableWidgetItem(m_depNameMap.value(strData)));
            }
            else if (strRecordName == QStringLiteral("status")) {
                ui.tableWidget->setItem(i, j, new QTableWidgetItem(m_statusMap.value(strData)));
            }
            else if (strRecordName == QStringLiteral("online")) {
                ui.tableWidget->setItem(i, j, new QTableWidgetItem(m_onlineMap.value(strData)));
            }
            else ui.tableWidget->setItem(i, j, new QTableWidgetItem(strData));
        }
    }
    

}
void QtQQ_Server::onUDPbrodMsg(QByteArray& btData) {
    for (quint16 port = gUdpPort; port < gUdpPort+200; port++) {
        m_udpSender->writeDatagram(btData, btData.size(),QHostAddress::Broadcast,port);
    }

}

