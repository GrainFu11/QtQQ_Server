#include "TcpSocket.h"

TcpSocket::TcpSocket()
{
}

TcpSocket::~TcpSocket()
{}

void TcpSocket::run() {
	//设置描述符
	m_descriptor = this->socketDescriptor();
	connect(this, &QTcpSocket::readyRead, this, &TcpSocket::onReceiveData);
	connect(this, &QTcpSocket::disconnected, this, &TcpSocket::onClientDisconnect);

}

void TcpSocket::onReceiveData() {
	QByteArray buffer=this->readAll();
	if (!buffer.isEmpty()) {
		QString strData = QString::fromUtf8(buffer);
		//发射接受客户端信息信号
		emit signalGetDataFromClient(buffer, m_descriptor);

	}
}

void TcpSocket::onClientDisconnect() {
	emit signalClientDisconnect(m_descriptor);
}