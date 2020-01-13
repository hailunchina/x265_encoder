#ifndef RTP_H
#define RTP_H

#include <QObject>
#include <QUdpSocket>
#include <QFile>;
class rtp : public QUdpSocket
{
    Q_OBJECT
public:
    explicit rtp(QUdpSocket *parent = nullptr);

private:
signals:
private:
    uint16_t seq_n=0;
    QFile *f;
public slots:
    void send_data(QByteArray data,int nal_type,bool is_at_last_nal,uint32_t time_value);
    QByteArray create_rtp_head( int rtp_number,bool is_at_last_nal,uint32_t time_value);
    QByteArray create_fu_head(int one_noone_end,int nal_type);
};

#endif // RTP_H
