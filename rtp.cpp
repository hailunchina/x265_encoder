#include "rtp.h"
#include <QUdpSocket>
#include "cache.h"
#include <QDateTime>
#include <QDataStream>
#include <QThread>
#include <QDebug>
rtp::rtp(QUdpSocket *parent) : QUdpSocket(parent)
{
    QByteArray send_data;
    f=new QFile("./a.hevc");
    if(f->exists()){
        QFile::remove(f->fileName());
    }
    qDebug()<<"...";

//    f->open(QIODevice::ReadWrite);
}
void rtp::send_data(QByteArray data,int nal_type,bool is_at_last_nal,uint32_t time_value){
    //写入文件
        f->open(QIODevice::ReadWrite|QFile::Append);
        f->write(data);
        f->flush();
        f->close();
    //写入文件结束
       QByteArray send_data;
       data=data.remove(0,4);
      //是否需要分包
    for(int i=0;i<client_rtp_rtcp_list.length();i++){
          QHostAddress c_ip(s_s_p.ip.toIPv4Address());
          int mtu=1400-3-12; //mtu 除头之外可存的数据大小
          if(data.size()>mtu){
//              qDebug()<<data.size()<<"开始分包";
              data.remove(0,2);
              QByteArray fu_send_data;
              QByteArray tmp_data=data.left(mtu);  //源数据1398 byte
              data.remove(0,mtu);
              //第一个fu包
               fu_send_data=create_fu_head(1,nal_type);
               fu_send_data.append(tmp_data);
               send_data=create_rtp_head(s_s_p.rtp,false,time_value);
               send_data.append(fu_send_data);
               this->writeDatagram(send_data.data(),send_data.size(),c_ip,s_s_p.rtp);
              //非第一个fu包,并且不是最后一个;
              while(data.size()>mtu){
                  tmp_data.clear();
                  fu_send_data.clear(); //清除fu buff
                  tmp_data=data.left(mtu); //取出指定字节的数据
                  data.remove(0,mtu);       //删除指定字节的数据
                  //添加
                  fu_send_data=create_fu_head(2,nal_type);
                  fu_send_data.append(tmp_data); //fu_head +data;
                  send_data=create_rtp_head(s_s_p.rtp,false,time_value); //rtp_head
                  send_data.append(fu_send_data); //rtp_head+fu_head+data
                  this->writeDatagram(send_data.data(),send_data.size(),c_ip,s_s_p.rtp);
//                  QThread::msleep(3);
              }

              //fu 最后一个包
//             qDebug()<<"最后一个包"<<data.size();
             tmp_data.clear();
             fu_send_data=create_fu_head(3,nal_type);
             tmp_data=data.left(data.size());
             data.remove(0,data.size());
             fu_send_data.append(tmp_data);
             send_data=create_rtp_head(s_s_p.rtp,is_at_last_nal,time_value);
             send_data.append(fu_send_data);
             this->writeDatagram(send_data.data(),send_data.size(),c_ip,s_s_p.rtp);
          }else {
              send_data=create_rtp_head(s_s_p.rtp,is_at_last_nal,time_value);
              send_data.append(data);
              this->writeDatagram(send_data.data(),send_data.size(),c_ip,s_s_p.rtp);
        }
    }
  }
QByteArray rtp::create_rtp_head( int rtp_number,bool is_at_last_nal ,uint32_t time_value){
    uint32_t rtp_head[4]={0x0};
    //头第一个32位
    seq_n++;
    rtp_head[0]=((rtp_head[0]>>30)+0x1)<<31;// v=2 10
    rtp_head[0]=((rtp_head[0]>>29)+0)<<29; //p=0;
    rtp_head[0]=((rtp_head[0]>>28)+0)<<28; //x=0
    rtp_head[0]=((rtp_head[0]>>24)+0x0000)<<24; //cc=0000
    if(is_at_last_nal){
          rtp_head[0]=((rtp_head[0]>>23)+0x1)<<23; //m=1
    }
    rtp_head[0]=((rtp_head[0]>>16)+0x60)<<16; //type
    rtp_head[0]=rtp_head[0]+seq_n; //seq_n++
    //头第二个32位时间戳
    rtp_head[1]=time_value;
    QByteArray rtp_head_byte;
    for (int i=0;i<client_rtp_rtcp_list.length();i++){
      s_s_p=client_rtp_rtcp_list[i];
      rtp_head[2]=rtp_number;
      rtp_head[3]=seq_n;
      QDataStream stream(&rtp_head_byte,QIODevice::ReadWrite);
      for(int y=0;y<3;y++){
          stream<<(uint32_t)rtp_head[y];
      }
    }
//    qDebug()<<rtp_head_byte.toHex();
    return  rtp_head_byte;
};

QByteArray rtp::create_fu_head(int one_noone_end,int nal_type){
    uint8_t playload_fu_head[3]={0};   //playload_head+fu_head
    playload_fu_head[0]=(49<<1);   //playload type 49;
    playload_fu_head[1]=1;
    QByteArray fu_head;
    playload_fu_head[2]=0x0;
    uint8_t buff=0x0;
    if(one_noone_end==1){
        playload_fu_head[2]=(buff+0x1)<<7;
        playload_fu_head[2]=playload_fu_head[2]+nal_type;
    }else if(one_noone_end==2){
        playload_fu_head[2]=playload_fu_head[2]+nal_type;
    }else if (one_noone_end==3) {
        playload_fu_head[2]=(buff+0x1)<<6;
        playload_fu_head[2]=playload_fu_head[2]+nal_type;
    }
    for(int xy=0;xy<3;xy++){
        fu_head.append(playload_fu_head[xy]);
    }
//    qDebug()<<fu_head.toHex();
    return fu_head;
}
