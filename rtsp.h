#ifndef RTSP_H
#define RTSP_H
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
class rtsp : public QTcpServer
{
    Q_OBJECT
public:
    explicit rtsp(int rtp,int rtcp,QTcpServer *parent = nullptr);
    void incomingConnection(qintptr socket_number);
signals:
public slots:
private:
    int server_rtp;
    int server_rtcp;
};
class connect_socket :public QTcpSocket{
    Q_OBJECT
public:
    explicit connect_socket(int rtp,int rtcp,qintptr socket_number,QTcpSocket *parent=nullptr);
    enum return_error{
        no_error=0,
        no_vision= -1,
        no_rtp_rtcp_port=-2,
    };
signals:
private:
    int router(QStringList head);
    void send_methon();
    void send_rtp_rtcp_session();
    void send_sdp();
    int verison;
    int client_rtp_port;
    int client_rtcp_port;
    int server_rtp_port;
    int server_rtcp_port;
    int cseq=0;
    QByteArray SSRC="Wang_qin_feng";
public slots:
    void read_data();
};
#endif // RTSP_H
