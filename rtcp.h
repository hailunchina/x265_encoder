#ifndef RTCP_H
#define RTCP_H

#include <QObject>
#include <QUdpSocket>
class rtcp : public QUdpSocket
{
    Q_OBJECT
public:
    explicit rtcp(QUdpSocket *parent = nullptr);

signals:

public slots:
    void read_data();
};

#endif // RTCP_H
