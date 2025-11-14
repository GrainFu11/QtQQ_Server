#pragma once

#include <QTcpSocket>

class TcpSocket  : public QTcpSocket
{
	Q_OBJECT

public:
	TcpSocket();
	~TcpSocket();
	void run();

private slots:
	void onReceiveData();		//处理readyRead信号读取的数据
	void onClientDisconnect();	//处理客户端断开连接
signals:
	void signalGetDataFromClient(QByteArray&,int);//从客户端收到数据后发射信号
	void signalClientDisconnect(int);			  //告知server有客户端断开连接

private:
	int m_descriptor;	//描述符，唯一标识
};

