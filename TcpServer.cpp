#include "TcpServer.h"
#include <QDebug>
//#include <QTcpSocket>
#include "TcpSocket.h"
TcpServer::TcpServer(int port):m_port(port)
{
}

TcpServer::~TcpServer()
{}

bool TcpServer::run() {
	
	if (this->listen(QHostAddress::AnyIPv4, m_port)) {
		qDebug() << QString::fromUtf8("服务端监听端口 %1 成功！").arg(m_port);
		return true;
	}
	else {
		qDebug()<< QString::fromUtf8("服务端监听端口 %1 失败！").arg(m_port);
		return false;
	}
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
	qDebug() << QString::fromUtf8("新的连接:") << socketDescriptor << Qt::endl;

	TcpSocket* tcpsocket = new TcpSocket();
	tcpsocket->setSocketDescriptor(socketDescriptor);
	tcpsocket->run();
	
	connect(tcpsocket, &TcpSocket::signalGetDataFromClient, this, &TcpServer::SocketDataProcessing);
	connect(tcpsocket, &TcpSocket::signalClientDisconnect, this, &TcpServer::SocketDisconnected);

	m_tcpSocketConnectList.append(tcpsocket);
}

void TcpServer::SocketDisconnected(int descriptor)
{
	for (int i = 0; i < m_tcpSocketConnectList.count(); i++) {
		QTcpSocket* item = m_tcpSocketConnectList.at(i);
		if (item->socketDescriptor() == descriptor||item->socketDescriptor()==-1) {
			m_tcpSocketConnectList.removeAt(i);
			item->deleteLater();
			qDebug() << QString::fromUtf8("TcpSocket断开连接: ") << descriptor << Qt::endl;
			return;
		}
	}
}

void TcpServer::SocketDataProcessing(QByteArray& SendData, int descriptor) {
	for (int i = 0; i < m_tcpSocketConnectList.count(); i++) {
		QTcpSocket* item = m_tcpSocketConnectList.at(i);
		if (item->socketDescriptor() == descriptor) {
			qDebug() << QString::fromUtf8("来自IP: ") << item->peerAddress().toString()
				<< QString::fromUtf8("接受到的数据: ") << QString(SendData);
			emit signalTcpMsgComes(SendData);
		}
	}
}