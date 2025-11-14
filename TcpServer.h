#pragma once

#include <QTcpServer>

class TcpServer  : public QTcpServer
{
	Q_OBJECT

public:
	TcpServer(int port);
	~TcpServer();
	bool run();				//服务端监听指定端口
protected:
	//客户端有新的连接时
	void incomingConnection(qintptr socketDescriptor);

private slots:
	//处理数据
	void SocketDataProcessing(QByteArray& SendData,int descriptor);
	//断开连接处理
	void SocketDisconnected(int descriptor);
signals:
	void signalTcpMsgComes(QByteArray&);
private:
	int m_port;			//端口号
	QList<QTcpSocket*>m_tcpSocketConnectList;

};

