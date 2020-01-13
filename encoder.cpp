#include "encoder.h"
#include <QGuiApplication>
#include <x265.h>
#include <QPixmap>
#include <QScreen>
#include <QDebug>
#include <QUdpSocket>
#include <QFile>
#include <QImage>
#include <QDataStream>
#include <QDateTime>
#include <QFile>
#include <string>
#include "cache.h"
encoder::encoder(QThread *parent) : QThread(parent)
{
    width=268;
    height=268;
}
QList<QByteArray> encoder::returnyuv(){
    QScreen *screen= QGuiApplication::primaryScreen();
    QPixmap disktop_img= screen->grabWindow(0).scaled(width,height,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    QImage disktop=disktop_img.toImage();
    disktop= disktop.convertToFormat(QImage::Format_RGB888);
    //rgba to yuv
    QByteArray y_l,u_l,v_l,yuv;
    uchar * d=disktop.bits();
    //循环变量
    int i,j;
    int width=disktop.width();
    int height=disktop.height();
    uint8_t r,g,b,y,u,v;
    for(i=0;i<height;i++){
        for(j=0;j<width;j++){
            int pos=i*(width*3)+j*3;
            r=d[pos+2];
            g=d[pos+1];
            b=d[pos];
            y=0.299*r+0.587*g+0.114*b;
            y_l.append(y);
            if(i%2==0 and j%2==0){ //偶数行并且偶数像素
                u=-0.169*r-0.331*g+0.5*b+128;
                           u_l.append(u);
                v=0.5*r-0.419*g-0.081*b+128;
                           v_l.append(v);
              }
        }
    }
    QList<QByteArray> yuv_byte;
    yuv_byte.append(y_l);
    yuv_byte.append(v_l);
    yuv_byte.append(u_l);
    return yuv_byte;
}
void encoder::run(){
    int status;
    x265_param *param=x265_param_alloc();
    status=x265_param_default_preset(param,"ultrafast","zerolatency");
//    status=x265_param_default_preset(param,"faster","fastdecode");
//    faster
    if(status!=0){
        qDebug()<<"快速，发送！";
            x265_param_free(param);
            return;
    }
    param->sourceWidth=width; //宽
    param->sourceHeight=height; //高
    param->frameNumThreads=1; //并发编码帧数，0为自动检测（默认值），在2到6直接会导致运动搜索（比如6个B帧）
    param->numaPools="none" ; //""|"*"所有的numa节点都用于线程池
                           //"none" 没有创建线程池只能进行帧并行编码。
                           //numa理解为服务器各CPU自己的内存区域。因此在多CPu编码并行时，因为各个帧要依赖于上一帧，所以可能导致在多个cpu内存来回取数据（个人理解）
    param->fpsNum=10; //帧率的分子
    param->fpsDenom=1;  //帧率的分母
    param->interlaceMode=0; //源图片的隔行类型：0：渐进图片（默认）
                                        // 1：顶场优先
                                        // 2：底场优先
    param->keyframeMax=10; //I帧的间隔
//    param->bframes=3;
//    param->intr
    param->interRefine=1;  //启用当前编码中的中间块的优化
//                            启用当前编码中的中间块的优化。
//                            级别0-从保存编码强制模式和深度。
//                            级别1-当当前块大小比min-cu-size大1时，评估当前深度（n）和深度（n + 1）的所有帧间模式。强制使用较大块的模式。
//                            级别2-除级别1的功能外，还限制了当保存编码将特定模式确定为最佳模式时评估的模式。
//                            保存编码中的2nx2n-禁用对rect和amp的重新评估。
//                            跳过保存编码-仅重新评估跳过，合并和2nx2n模式。
//                            级别3-在重用保存编码中的深度时执行帧间模式分析。
//                            默认值0。
    param->internalCsp=X265_CSP_I420;
//    param->bField=true; //字段编码
//    param->maxCUSize=16;
    param->minCUSize=32;  //最小CU块
//    param->bOpenGOP=false;
    //rc结构体
    param->rc.rateControlMode=X265_RC_CRF; //（默认 CRF）码率控制模式
                // X265_RC_ABR, //指定平均码率， x264的一位主要开发者说你应该永远不要使用它，由于编码器无法提前知道要编码视频的情况，它将不得不猜测如何达到比特率。
                // X265_RC_CQP,  //恒定QP
                   param->rc.qp=51; //0表示无损 范围0-51,值越大表示越大的量化步长;
                // X265_RC_CRF   //恒定质量因子，对于运动或细节丰富的场景会增大量化失真，对静止或平坦区域则减少量化失真;
                param->rc.rfConstantMax=51; //crf最大码率，0-51 值越大视频质量越低，压缩率越高
                param->rc.rfConstantMin=0;  //crf最小码率
                param->rc.rfConstant=0;    //常量码率
    param->bRepeatHeaders=true; //是否命令行输出 vps sps pps 头标志
    param->bAnnexB=true;    //是否生成NUAL起始标志 0000001;
QByteArray data;
x265_encoder *encoder;
encoder=x265_encoder_open(param);
x265_encoder_reconfig(encoder,param);
//x265_encoder_open(param); //创建一个编码器指针
x265_nal *pp_nal;
uint32_t pi_nal;
x265_picture *picture;
int frame=0;
//int frame_ms=1000/param->fpsNum;
//    QList<QByteArray> yuv=returnyuv(); //截屏转yuv4:2:0
while (true){
    data.clear();
    QDateTime start_jieping=QDateTime::currentDateTime();
    QList<QByteArray> yuv=returnyuv(); //截屏转yuv4:2:0
    picture = x265_picture_alloc();
    x265_picture_init(param,picture);
    picture->planes[0]=yuv[0].data(); //y
    picture->planes[1]=yuv[1].data();//u
    picture->planes[2]=yuv[2].data(); //v
    picture->stride[0]=param->sourceWidth;
    picture->stride[1]=param->sourceWidth/2;
    picture->stride[2]=param->sourceWidth/2;
    frame++;
    x265_encoder_encode(encoder,&pp_nal,&pi_nal,picture,NULL); //编码
    int data_byte_size=0;
    for(int i=0;i<pi_nal;i++){ //写入数据长度
        data_byte_size+=pp_nal[i].sizeBytes;
        }
     char data_char[data_byte_size];
     memcpy(&data_char, pp_nal[0].payload,data_byte_size);
     data=QByteArray(data_char,data_byte_size);
     QDateTime end_jieping=QDateTime::currentDateTime();
     int sleep_ms= start_jieping.msecsTo(end_jieping);
     uint32_t time_value=sleep_ms*(9000/param->fpsNum/(1000/30))+frame*(90000/param->fpsNum);
     rtp_server.send_data(data,pp_nal->type,true,time_value);
     rtp_server.flush();
     if(sleep_ms<(1000/param->fpsNum)){
         QThread::msleep((1000/param->fpsNum)-sleep_ms);
     }
}
x265_param_free(param);
x265_picture_free(picture);
x265_encoder_close(encoder);
}
