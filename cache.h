#ifndef CACHE_H
#define CACHE_H
#include <QList>
#include <QHostAddress>
#include "rtp.h"
struct rtp_rtcp_struct{
  int rtp;
  int rtcp;
  QHostAddress ip;
};
extern rtp_rtcp_struct s_s_p;
extern QList<rtp_rtcp_struct> client_rtp_rtcp_list;
extern rtp rtp_server;
class cache
{
public:
    cache();
};

#endif // CACHE_H
