#ifndef ENCODER_H
#define ENCODER_H

#include <QObject>
#include <QThread>
class encoder : public QThread
{
    Q_OBJECT
public:
    explicit encoder(QThread *parent = nullptr);
    QList<QByteArray> returnyuv();
    void run();
signals:
//    void send_data(QByteArray data,int nal_type,bool is_at_last_nal);
public slots:
//    void send_data();
private:
    int width;
    int height;
};

#endif // ENCODER_H
