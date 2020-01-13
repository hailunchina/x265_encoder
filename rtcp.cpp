#include "rtcp.h"
#include <QUdpSocket>
#include <QNetworkDatagram>
rtcp::rtcp(QUdpSocket *parent) : QUdpSocket(parent)
{
    connect(this,SIGNAL(readyRead()),this,SLOT(read_data()));
}
void rtcp::read_data(){
    QNetworkDatagram datagram = this->receiveDatagram();
       qDebug()<< datagram.senderAddress()<<datagram.senderPort();
       qDebug()<<datagram.destinationAddress()<<datagram.destinationPort();
}
