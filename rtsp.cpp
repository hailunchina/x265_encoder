#include "rtsp.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QDebug>
#include <QDateTime>
#include <QByteArray>
#include "cache.h"
rtsp::rtsp(int rtp,int rtcp,QTcpServer *parent) : QTcpServer(parent)
{
    server_rtp=rtp;
    server_rtcp=rtcp;
}
void rtsp::incomingConnection(qintptr socket_number){
    connect_socket *client_socket=new connect_socket(server_rtp,server_rtcp,socket_number);
    QThread *client_socket_thread=new QThread();
    client_socket->moveToThread(client_socket_thread);
    client_socket_thread->start();
}
connect_socket::connect_socket(int rtp,int rtcp,qintptr socket_number,QTcpSocket *parent):QTcpSocket (parent){
    this->setSocketDescriptor(socket_number);
    server_rtp_port=rtp; //server rtp_port
    server_rtcp_port=rtcp; //server rtcp_port
    connect(this,SIGNAL(readyRead()),this,SLOT(read_data()));
};
void connect_socket::read_data(){
    QString tmp_data;
    tmp_data=this->readAll();
    if(tmp_data.size()>50){
        QStringList request_body=tmp_data.split(" ");
//        qDebug()<<tmp_data;
        router(request_body);
    }
}
int connect_socket::router(QStringList head){
    return_error error_return;
    error_return=no_error; //正常返回0;
    QString methon=head[0];

    cseq=head[3].split("\r\n")[0].toInt();
    if(methon=="OPTIONS"){ //本版获取
        error_return=no_vision;
            if(head[2].contains("\r\n")){
                QStringList verison_list=head[2].split("\r\n");
                //rtsp获取版本号
                if(verison_list[0].contains("/")){verison=verison_list[0].split("/")[0].toInt();}else {
                    return  error_return;
                    };
                send_methon(); //发送支持的方法给客户端;
            }else {
                //无版本号返回-1;
                error_return=no_vision;
                return error_return;
            }
    }else if (methon=="SETUP") { //setup请求
        error_return=no_rtp_rtcp_port;
        //从请求中取出rtp rtcp端口号
        if(head[head.length()-1].contains("\r\n\r\n")){
            int client_port_index=0;
            for(int i=0;i<head.length();i++){
                if(head[i].contains("=") and head[i].contains("-")){
                    client_port_index=i;
                }
            }
            QStringList port_list=head[client_port_index].split("\r\n\r\n")[0].split("=")[1].split("-");
            client_rtp_port=port_list[0].toInt();
            client_rtcp_port=port_list[1].toInt();
                        //添加客户端端口号到列表
            s_s_p.rtp=client_rtp_port;
            s_s_p.rtcp=client_rtcp_port;
            s_s_p.ip= this->peerAddress();
            client_rtp_rtcp_list.append(s_s_p);
                        //返回会话数据
            send_rtp_rtcp_session();
            }else {
                        return error_return;
                    }
    }else if (methon=="PLAY") {
        QString url;
        if(head.size()>=1){
            url=head[1];
        }
        QByteArray data;
        data.append("RTSP/1.0 200 OK\r\nCSeq: "+QString::number(cseq)+"\r\nSession: "+SSRC+"\r\n"+"RTP-Info: url="+url
                    +";seq=9810092;rtptime=0\r\n\r\n");
        this->write(data);
//        qDebug()<<data;
        this->waitForBytesWritten(3000);
    }else if (methon=="DESCRIBE") {
            send_sdp();
    }else{
    }
    return  error_return;
};
void connect_socket::send_methon(){ //发送支持的方法
    QByteArray data;
    data.append("RTSP/1.0 200 OK\r\n CSeq:"+QString::number(cseq)+"\r\nPublic:DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE\r\n\r\n");
    this->write(data);
    this->waitForBytesWritten(3000);
}
void connect_socket::send_rtp_rtcp_session(){ //发送会话数据
    SSRC+=QString::number(client_rtp_port)+"_"+QString::number(client_rtcp_port);
    QByteArray data;
    data.append("RTSP/1.0 200 OK\r\nCSeq:"+QString::number(cseq)+"\r\nTransport: RTP/AVP;unicast;client_port="+
            QString::number(client_rtp_port)+"-"+QString::number(client_rtcp_port)+";"
            +"server_port="+QString::number(server_rtp_port)+"-"+QString::number(server_rtcp_port)+";"
            +"ssrc="+SSRC+"\r\nSession:"+SSRC+"\r\n\r\n");
//    qDebug()<<data;
    this->write(data);
    this->waitForBytesWritten(3000);
}
void connect_socket::send_sdp(){
    QByteArray tmp_data;
    QByteArray data;
    tmp_data.append("c=IN IP4 "+this->peerAddress().toString().split(":")[3]+"\r\nm=video 0 RTP/AVP 96\r\na=rtpmap:96 H265/90000\r\na=fmtp:96 profile-space=0;profile-id=0;tier-flag=0;level-id=0\r\n\r\n");
    data.append( "RTSP/1.0 200 OK\r\nCSeq:"+QString::number(cseq)+"\r\nContent-Type: application/sdp\r\nContent-Length:"
                 +QString::number(tmp_data.size())+"\r\n\r\n");
    data.append(tmp_data);
//    qDebug()<<tmp_data;
    this->write(data);
    this->waitForBytesWritten(3000);
}
