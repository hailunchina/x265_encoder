#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDebug>
#include "encoder.h"
#include "cache.h"
#include "rtsp.h"
#include "rtp.h"
//#define vcss
int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    int rtp_port=1130;
    int rtcp_port=1131;
    //截屏编码
    encoder *en_coder=new encoder();
    en_coder->start();
    //rtsp服务
    rtsp server_rtsp(rtp_port,rtcp_port);
    server_rtsp.listen(QHostAddress::Any,1133);
    //rtp服务
    rtp_server.bind(QHostAddress::Any,rtp_port);
//    QObject::connect(en_coder,SIGNAL(send_data(QByteArray,int,bool)),&server_rtp,SLOT(send_data(QByteArray,int,bool)));
    rtp_server.bind(QHostAddress::Any,rtp_port);
    //rtcp服务
    rtp server_rtcp;
    server_rtcp.bind(QHostAddress::Any,rtcp_port);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
