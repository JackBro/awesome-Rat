/*******************************************************************
   *	socketBase.cpp
   *    DESCRIPTION:socket�����װ��Ķ���
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2004-10-10
   *	Last modify: 2005-09-01
   *	net4cpp 2.1
   *******************************************************************/

#include "include/sysconfig.h"
#include "include/socketBase.h"
#include "include/cLogger.h"

#include <cstdarg>
#include <cerrno>

using namespace std;
using namespace net4cpp21;

socketBase::socketBase()
{
	m_sockfd=INVALID_SOCKET;
	m_socktype=SOCKS_NONE;
	m_sockstatus=SOCKS_CLOSED;
	m_sockflag=0;
	m_ipv6=false;
	memset((void *)&m_remoteAddr,0,sizeof(SOCKADDR_IN));
	memset((void *)&m_localAddr,0,sizeof(SOCKADDR_IN));
	m_recvBytes=0;
	m_sendBytes=0;
	m_maxSendRatio=m_maxRecvRatio=0;
	m_tmOpened=0;
	m_errcode=SOCKSERR_OK;
	m_parent=NULL;
}

socketBase :: socketBase(socketBase &sockB)
{
	m_sockfd=sockB.m_sockfd;
	m_socktype=sockB.m_socktype;
	m_sockstatus=sockB.m_sockstatus;
	m_sockflag=sockB.m_sockflag;
	m_ipv6=sockB.m_ipv6;
	m_remoteAddr=sockB.m_remoteAddr;
	m_localAddr=sockB.m_localAddr;
	m_recvBytes=sockB.m_recvBytes;
	m_sendBytes=sockB.m_sendBytes;
	m_maxSendRatio=sockB.m_maxSendRatio;
	m_maxRecvRatio=sockB.m_maxRecvRatio;
	m_tmOpened=sockB.m_tmOpened;
	m_errcode=sockB.m_errcode;
	m_parent=sockB.m_parent;
	
	sockB.m_sockfd=INVALID_SOCKET;
	sockB.m_socktype=SOCKS_NONE;
	sockB.m_sockstatus=SOCKS_CLOSED;
	memset((void *)&sockB.m_localAddr,0,sizeof(SOCKADDR_IN));
}

socketBase& socketBase:: operator = (socketBase &sockB)
{
	m_sockfd=sockB.m_sockfd;
	m_socktype=sockB.m_socktype;
	m_sockstatus=sockB.m_sockstatus;
	m_sockflag=sockB.m_sockflag;
	m_ipv6=sockB.m_ipv6;
	m_remoteAddr=sockB.m_remoteAddr;
	m_localAddr=sockB.m_localAddr;
	m_recvBytes=sockB.m_recvBytes;
	m_sendBytes=sockB.m_sendBytes;
	m_maxSendRatio=sockB.m_maxSendRatio;
	m_maxRecvRatio=sockB.m_maxRecvRatio;
	m_tmOpened=sockB.m_tmOpened;
	m_errcode=sockB.m_errcode;
	m_parent=sockB.m_parent;
	
	sockB.m_sockfd=INVALID_SOCKET;
	sockB.m_socktype=SOCKS_NONE;
	sockB.m_sockstatus=SOCKS_CLOSED;
	memset((void *)&sockB.m_localAddr,0,sizeof(SOCKADDR_IN));
	return *this;
}

socketBase::~socketBase()
{
	Close(); 
}

void socketBase :: Close()
{
	if(m_socktype==SOCKS_NONE) return;
	m_socktype=SOCKS_NONE;
	m_sockflag=0;
	if ( m_sockfd!=INVALID_SOCKET ){
//��������SO_LINGER���༴linger�ṹ�е�l_onoff����Ϊ���㣬�μ�2.4��4.1.7��4.1.21���ڣ������������㳬ʱ�����
//��closesocket()������������ִ�У������Ƿ����Ŷ�����δ���ͻ�δ��ȷ�ϡ����ֹرշ�ʽ��Ϊ��ǿ�ơ���ʧЧ���رգ�
//��Ϊ�׽ӿڵ����·��������λ���Ҷ�ʧ��δ���͵����ݡ���Զ�˵�recv()���ý���WSAECONNRESET����
//��������SO_LINGER��ȷ���˷���ĳ�ʱ�������closesocket()�����������̣�ֱ����ʣ���ݷ�����ϻ�ʱ��
//���ֹرճ�Ϊ�����ŵġ��رա���ע������׽ӿ���Ϊ��������SO_LINGER��Ϊ���㳬ʱ����closesocket()���ý�
//��WSAEWOULDBLOCK���󷵻ء�
//windowsϵͳĬ��SO_DONTLINGER�������º͹ر�
//		::shutdown(m_sockfd,SD_BOTH); 
		::closesocket(m_sockfd);
		m_sockfd=INVALID_SOCKET;
	}//?if ( m_sockfd!=INVALID_SOCKET ){
	m_sockstatus=SOCKS_CLOSED;
	m_localAddr.sin_port=0;
	m_localAddr.sin_addr.s_addr=INADDR_ANY;
	m_recvBytes=m_sendBytes=0;
	//yyc remove 2006-02-15,�رյ�ʱ�����Զ�̵�ַ��Ϣ
	//����ͨ��setRemoteInfo����,��ֹcreateʱ���
//	m_remoteAddr.sin_port=0;
//	m_remoteAddr.sin_addr.s_addr=INADDR_ANY;
	return;
}

//����Ҫ���ӻ�UPD���͵�Զ��������Ϣ
SOCKSRESULT socketBase :: setRemoteInfo(const char *host,int port)
{
	if(host==NULL || port<=0) return SOCKSERR_PARAM;
	//�ж��Ƿ�Ϊһ����Ч��IP
	unsigned long ipAddr=socketBase::Host2IP(host);
	if(ipAddr==INADDR_NONE) return SOCKSERR_HOST;
	m_remoteAddr.sin_port=htons(port);
	m_remoteAddr.sin_addr.s_addr=ipAddr;
	return SOCKSERR_OK;
}

//���ط������ݵĴ�С�����<0��������
SOCKSRESULT socketBase :: Send(LPCTSTR fmt,...)
{
	if(m_sockstatus<SOCKS_CONNECTED) return SOCKSERR_INVALID;
	TCHAR buf[2048];
	va_list args;
	va_start(args,fmt);
	int len=vsnprintf(buf,sizeof(buf)-1,fmt,args);
	va_end(args);
	if(len==-1) return SOCKSERR_BUFFER; //len=sizeof(buf)-1; 
	buf[len]=0;
	return (len>0)?_Send((const char *)buf,len*sizeof(TCHAR),-1):0;
}
SOCKSRESULT socketBase :: Send(size_t buflen,const char * buf,time_t lWaitout)
{
	if(m_sockstatus<SOCKS_CONNECTED) return SOCKSERR_INVALID;
	if(buf==NULL) return SOCKSERR_PARAM;
	if(buflen==0) if( (buflen=strlen(buf))==0) return SOCKSERR_PARAM;
	return (buflen>0)?_Send((const char *)buf,buflen,lWaitout):0;
}
//���ʹ�������
SOCKSRESULT socketBase :: SendOOB(size_t buflen,const char *buf)
{
	if(m_sockstatus!=SOCKS_CONNECTED) return SOCKSERR_INVALID;
	if(buf==NULL) return SOCKSERR_PARAM;
	if(buflen==0) if( (buflen=strlen(buf))==0) return SOCKSERR_PARAM;
	buflen=::send(m_sockfd,buf,buflen,MSG_NOSIGNAL|MSG_OOB);
	if(buflen>=0)  return buflen; 
	
	m_errcode=SOCK_M_GETERROR;
	return SOCKSERR_ERROR;//����ϵͳ����ͨ��SOCK_M_GETERROR��ô������
}

//--------------------------static function---------------------------------------

//�õ�����IP�����صõ�����IP�ĸ���
long socketBase :: getLocalHostIP(vector<string> &vec)
{
	char buf[64];
	gethostname(buf,sizeof(buf));
	struct hostent * p=gethostbyname(buf);
	if(p==NULL) return 0;
	for(int i=0;p->h_addr_list[i];i++)
		vec.push_back( (char *)inet_ntoa(*((struct in_addr *)p->h_addr_list[i])) );
	return i;
}
const char *socketBase :: getLocalHostIP()
{
	char buf[64];
	gethostname(buf,sizeof(buf));
	struct hostent * p=gethostbyname(buf);
	if(p==NULL) return 0;
	return inet_ntoa(*((struct in_addr *)p->h_addr_list[0]));
}

//����ָ��������,only for IPV4
unsigned long socketBase :: Host2IP(const char *host)
{
	unsigned long ipAddr=inet_addr(host);
	if(ipAddr!=INADDR_NONE) return ipAddr;
	//ָ���Ĳ���һ����Ч��ip��ַ������һ����������
	struct hostent * p=gethostbyname(host);
	if(p==NULL) return INADDR_NONE;
	ipAddr=(*((struct in_addr *)p->h_addr)).s_addr;
	return ipAddr;
}

//windows��Ĭ�ϵ�FD_SETSIZEֻ��64��С!!!ע��checkSocket�д�����socket���ܳ�����ֵ���趨
//!!!!!#define FD_SETSIZE      64 
//���ָ����sockets�Ƿ�ɶ����д
//sockfds --- Ҫ����socket������飬len-sockets����ĸ���
//wait_usec --- ��鳬ʱ�ȴ�ʱ�䣬΢��
//opmode --- ���socket�Ƿ�ɶ�/д/��OOB��������
//���� --- ���ؿɶ�/��д��socket�������
//	--- �������С��0��˵�������˴���
//	�˺������дsockfds�����е�ֵ�����ĳ��socket����ɶ�/��д����1������0
//windows/linux,unix��FD_SET��ʵ�ֲ��
//windows��Ĭ�ϵ�FD_SETSIZEΪ64��FD_SET�����SOCKETs���飬�������
//��fd�����SOCKETs���Ѿ����ڣ����˳�������fd���뵽������棬ͬʱ����++
//linux,unix��Ĭ�ϵ�FD_SETSIZEΪ65535��FD_SET�꽫socket�ľ��ֵ���뵽��fdֵΪ�±�
//��SOCKETs������λ�á����linux,unix�µ���FD_SET��ʱҪ����fd<FD_SETSIZE��顣
//��linux,unix��socket�ľ��fd��ֵ������ڣ�FD_SETSIZE����Ч��
int socketBase :: checkSocket(int *sockfds,size_t len,time_t wait_usec,SOCKETOPMODE opmode)
{
	int i,retv=0;
	struct timeval to;
	fd_set fds; FD_ZERO(&fds);
	for(i=0;i<len;i++)
	{
		FD_SET(sockfds[i], &fds);
	}
	if ( wait_usec>0)
    {
		to.tv_sec =wait_usec/1000000L;
		to.tv_usec =wait_usec%1000000L;
    }
	else
	{
		to.tv_sec = 0;
		to.tv_usec = 0;
	}
	if(opmode<=SOCKS_OP_READ) //�ж��Ƿ������ݿɶ�
		retv = select(FD_SETSIZE, &fds, NULL, NULL, &to);
	else if(opmode==SOCKS_OP_WRITE) //�ж��Ƿ��д
		retv = select(FD_SETSIZE, NULL, &fds, NULL, &to);
	else if(opmode==SOCKS_OP_ROOB) //�Ƿ���OOB���ݿɶ�
		retv = select(FD_SETSIZE, NULL, NULL, &fds, &to);
	else return 1; //����������ԶΪ�棬����д��������!!!!!
	if(retv==0) return 0; // timeout occured 
	if(retv==SOCKET_ERROR){ //�д�����
		if(SOCK_M_GETERROR==EINPROGRESS) return 0;//���ڴ�������У��������
	}
	//���������Ĵ�����ֱ�ӷ���,�����û������ж�
	//�о���ɶ����д
	for(i=0;i<len;i++)
	{
		sockfds[i]=(FD_ISSET(sockfds[i], &fds))?1:0;
	}
	return retv;
}

//--------------------------private function---------------------------------------

inline int socketBase :: getAF()
{
	int af=AF_INET;
#ifdef IPPROTO_IPV6
	if(m_ipv6) af=AF_INET6;
#endif
	return af;
}

//��ȡ��socket�󶨵ı���ip�Ͷ˿ڣ��ɹ����ذ󶨵ı��ض˿�
//����<=0��������
int socketBase :: getSocketInfo()
{
//	if(m_sockfd==INVALID_SOCKET) return SOCKSERR_INVALID;
	//getsockname()�������ڻ�ȡһ���׽ӿڵ����֡�������һ����������������׽ӿ�s�����ص�ַ�������ء�
	//�������ر����������������δ����bind()�͵�����connect()��
	//��ʱΨ��getsockname()���ÿ��Ի�֪ϵͳ�ڶ��ı��ص�ַ��
	//�ڷ���ʱ��namelen�������������ֵ�ʵ���ֽ�����
	//��һ���׽ӿ���INADDR_ANY����Ҳ����˵���׽ӿڿ��������������ĵ�ַ��
	//��ʱ���ǵ���connect()��accept()�����ӣ�����getsockname()�����᷵������IP��ַ���κ���Ϣ��
	//�����׽ӿڱ����ӣ�WINDOWS�׽ӿ�Ӧ�ó���Ӧ����IP��ַ���INADDR_ANY���������ַ��
	//������Ϊ���ڶ�����������£������׽ӿڱ����ӣ�������׽ӿ����õ�IP��ַ�ǲ���֪�ġ�
	int addr_len=sizeof(m_localAddr);
	m_localAddr.sin_family=getAF();
	m_localAddr.sin_port=0; m_localAddr.sin_addr.s_addr=0;
	getsockname(m_sockfd,(struct sockaddr *) &m_localAddr,(socklen_t *)&addr_len);
	return ntohs(m_localAddr.sin_port);
}

//��ָ���Ķ˿ں�IP
//bReuse -- ָ���Ƿ�����ö˿� bindip --- ָ��Ҫ�󶨵�ip��ַ�����ΪNULL/""������е�
//�ɹ�����ʵ�ʰ󶨵Ķ˿� >0
SOCKSRESULT socketBase :: Bind(int port,BOOL bReuseAddr,const char *bindip)
{
	if(m_sockfd==INVALID_SOCKET) return SOCKSERR_INVALID;
	if(bReuseAddr==SO_REUSEADDR){//���԰��������õĶ˿�
		int on=1;
		setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &on,sizeof(on));
	}else if(bReuseAddr==SO_EXCLUSIVEADDRUSE){ //�󶨵Ķ˿ڽ�ֹ����
		int on=1;
		setsockopt(m_sockfd, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char *) &on,sizeof(on));
	}//���򲻿��԰��������õĶ˿�,���ΰ󶨵Ķ˿���������

	SOCKADDR_IN addr; memset(&addr, 0, sizeof(addr));
	addr.sin_family = getAF();
	addr.sin_port =(port<=0)?0:htons(port);
	if(bindip && bindip[0]!=0) addr.sin_addr.s_addr=inet_addr(bindip);
	//���� �����󶨱����κε�ַINADDR_ANY

	if (bind(m_sockfd, (struct sockaddr *) &addr, sizeof(addr)) == SOCKET_ERROR ) 
	{
		m_errcode=SOCK_M_GETERROR;
		return SOCKSERR_BIND;
	}
	return getSocketInfo();
}
//��ָ���Ķ˿ڣ��˿���[startport,endport]֮�����ѡ��
//startport>=0, endport<=65535.���endport<=0��=65535
//����ʵ�ʰ󶨵Ķ˿�
SOCKSRESULT socketBase :: Bind(int startport,int endport,BOOL bReuseAddr,const char *bindip)
{
	if(m_sockfd==INVALID_SOCKET) return SOCKSERR_INVALID;
	if(bReuseAddr==SO_REUSEADDR){//���԰��������õĶ˿�
		int on=1;
		setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &on,sizeof(on));
	}else if(bReuseAddr==SO_EXCLUSIVEADDRUSE){ //�󶨵Ķ˿ڽ�ֹ����
		int on=1;
		setsockopt(m_sockfd, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char *) &on,sizeof(on));
	}//���򲻿��԰��������õĶ˿�,���ΰ󶨵Ķ˿���������
	
	SOCKADDR_IN addr; memset(&addr, 0, sizeof(addr));
	addr.sin_family = getAF();
	if(bindip && bindip[0]!=0) addr.sin_addr.s_addr=inet_addr(bindip);
	//���� �����󶨱����κε�ַINADDR_ANY
	if(startport<1) startport=1;
	if(endport<1 || endport>65535) endport=65535;
	if(endport<startport){ int iswap=startport; startport=endport; endport=iswap; }
	
	int icount=endport-startport;
	if(icount<10)
	{
		for(int i=0;i<=icount;i++)
		{
			addr.sin_port=htons(startport+i);
			if (bind(m_sockfd, (struct sockaddr *) &addr, sizeof(addr)) != SOCKET_ERROR )
				break;
		}
		if(i>icount) return SOCKSERR_BIND;
	}
	else
	{
		for(int i=1;i<=10;i++)
		{
			int port=startport+rand()*icount/RAND_MAX;
			addr.sin_port=htons(port);
			if (bind(m_sockfd, (struct sockaddr *) &addr, sizeof(addr)) != SOCKET_ERROR )
				break;
		}
		if(i>10) return SOCKSERR_BIND;
	}
	return getSocketInfo();
}
//ʹ��/��ֹSO_LINGER�����ʹ��SO_LINGER����ر�socket���ʱ����ȴ������������ݷ������
//iTimeout ��--- ���ʹ��SO_LINGER����ָ������೤ʱ���ڻ��������������û�з��������ǿ�йر�
SOCKSRESULT socketBase::setLinger(bool bEnabled,time_t iTimeout)
{
	if(m_sockfd==INVALID_SOCKET) return SOCKSERR_INVALID;
	if(m_sockstatus<SOCKS_CONNECTED) return SOCKSERR_OK;
	int sr=SOCKSERR_OK;
	//ʹ��SO_LINGER
	//When  enabled,  a  close(2) or shutdown(2) will not return until all queued messages for the socket have
    //      been successfully sent or the linger timeout has been reached. Otherwise, the call  returns  immediately
    //     and  the  closing  is  done  in the background.  When the socket is closed as part of exit(2), it always
    //      lingers in the background.
	struct linger lg;
	if(bEnabled)
		lg.l_onoff=1;
	else
		lg.l_onoff=0;
	lg.l_linger=iTimeout;//���ó�ʱ���ʱ��s
	if(setsockopt(m_sockfd,SOL_SOCKET,SO_LINGER,(const char *)&lg,sizeof(lg))!=0)
		return SOCKSERR_SETOPT;
	return SOCKSERR_OK;
}
//����socket����/������
//�ɹ�����SOCKSERR_OK
SOCKSRESULT socketBase::setNonblocking(bool bNb)
{
	if(m_sockfd==INVALID_SOCKET) return SOCKSERR_INVALID;
	int sr=SOCKSERR_OK;
#ifdef WIN32
	unsigned long l = bNb ? 1 : 0; //1 -- ������
	sr=ioctlsocket(m_sockfd,FIONBIO,&l); //����socketΪ����/��������ʽ
	if(sr!=0) sr=SOCKSERR_ERROR;
#else
	if (bNb)
	{
		sr=fcntl(s, F_SETFL, O_NONBLOCK);
	}
	else
	{
		sr=fcntl(s, F_SETFL, 0);
	}
#endif
	//����ϵͳ����ͨ��SOCK_M_GETERROR��ô������
	if(sr==SOCKSERR_ERROR) m_errcode=SOCK_M_GETERROR;
	return sr;
}

//����ָ�����͵�socket���
//�ɹ���������򷵻ؼ�
bool socketBase :: create(SOCKETTYPE socktype) 
{
	if( type()!=SOCKS_NONE ) Close();
	m_localAddr.sin_family=getAF();
	m_remoteAddr.sin_family=getAF();
	int af=m_localAddr.sin_family;
	switch(socktype)
	{
		case SOCKS_TCP:
			if( (m_sockfd=::socket(af,SOCK_STREAM,0))!=INVALID_SOCKET )
				m_socktype=SOCKS_TCP;
			break;
		case SOCKS_UDP:
			if( (m_sockfd=::socket(af,SOCK_DGRAM,0))!=INVALID_SOCKET )
			{
				m_socktype=SOCKS_UDP;
				int on=1; //Allows transmission of broadcast messages on the socket.
						// ֧�ֹ㲥����,Ŀ�ĵ�ַ��255.255.255.255,��ζ�����������л�������
				setsockopt(m_sockfd, SOL_SOCKET, SO_BROADCAST, (char *) &on,sizeof(on));
			}
			break;
		case SOCKS_RAW:
			break;
	}//?switch(socktype)
	if(m_socktype==SOCKS_NONE) return false;
	m_tmOpened=time(NULL);
	return true;
}
//ע��:����������Ϣsocket(message-oriented) һ�η��͵����ݳ��Ȳ��ܴ���socket�ķ��ͻ������Ĵ�С�����򷵻�SOCKET_ERROR
//һ��ϵͳĬ�ϵ�socket�ķ��ͻ�������SO_MAX_MSG_SIZE 
/*inline size_t socketBase :: v_write(const char *buf,size_t buflen)
{
	size_t len=0;
	if(m_socktype==SOCKS_TCP)
		len=::send(m_sockfd,buf,buflen,MSG_NOSIGNAL);
	else
		len=::sendto(m_sockfd,buf,buflen,MSG_NOSIGNAL,(struct sockaddr *)&m_remoteAddr,sizeof(SOCKADDR_IN));
	return len;
}*/
inline size_t socketBase :: v_write(const char *buf,size_t buflen)
{
	return ::send(m_sockfd,buf,buflen,MSG_NOSIGNAL);
}
inline size_t socketBase :: v_writeto(const char *buf,size_t buflen,SOCKADDR_IN &addr)
{
	return ::sendto(m_sockfd,buf,buflen,MSG_NOSIGNAL,(struct sockaddr *)&addr,sizeof(SOCKADDR_IN));
}

inline size_t socketBase :: v_read(char *buf,size_t buflen)
{
	size_t len=0;
	if(m_socktype==SOCKS_TCP)
		len=::recv(m_sockfd,buf,buflen,MSG_NOSIGNAL);
	else
	{
		m_remoteAddr.sin_addr.S_un.S_addr=INADDR_ANY;
		m_remoteAddr.sin_port=0;
		int addrlen=sizeof(m_remoteAddr);
		len=::recvfrom(m_sockfd,buf,buflen,MSG_NOSIGNAL,
			(struct sockaddr *) &m_remoteAddr, (socklen_t *)&addrlen);
	}	
	return len;
}
inline size_t socketBase :: v_peek(char *buf,size_t buflen)
{
	size_t len=0;
	if(m_socktype==SOCKS_TCP)
		len=::recv(m_sockfd,buf,buflen,MSG_NOSIGNAL|MSG_PEEK);
	else
	{
		m_remoteAddr.sin_addr.S_un.S_addr=INADDR_ANY;
		m_remoteAddr.sin_port=0;
		int addrlen=sizeof(m_remoteAddr);
		len=::recvfrom(m_sockfd,buf,buflen,MSG_NOSIGNAL|MSG_PEEK,
			(struct sockaddr *) &m_remoteAddr, (socklen_t *)&addrlen);
	}
		
	return len;
}

//�������ݰ�
//���ؽ������ݵĳ��ȣ����<0��������
//�������==0���ʾ�����ٶȳ������趨���������ƣ����ݻ�����
//bPeek --- �Ƿ�ֻPEEK���ݣ������ӽ��ջ��������Ƴ�
//==-1˵������ϵͳ����ͨ��SOCK_M_GETERROR��ô������ SOCKETOPMODE
SOCKSRESULT socketBase :: _Receive(char *buf,size_t buflen,time_t lWaitout,SOCKETOPMODE opmode)
{
	if(m_sockstatus<SOCKS_CONNECTED) return SOCKSERR_INVALID;
	if(buf==NULL || buflen==0) return SOCKSERR_PARAM;
	int iret=1; //�Ƿ������ݿɶ�
	if(lWaitout>=0)
	{
		time_t t=time(NULL);
		while( (iret=checkSocket(SCHECKTIMEOUT,opmode))== 0 )
		{//������Ƿ�ɶ�
			if( (time(NULL)-t)>lWaitout ) break; //����Ƿ�ʱ
			if(m_parent && m_parent->m_sockstatus<=SOCKS_CLOSED) 
				return SOCKSERR_PARENT;
		}//?while
	}//?if(lWaitout>=0)
	if(iret!=1)
	{
		if(iret==-1) {m_errcode=SOCK_M_GETERROR; return SOCKSERR_ERROR;}
		return SOCKSERR_TIMEOUT; //��ʱ
	}
	//���Է�gracefully closedʱ,recvfrom��������0�����ܻ��������Ƿ�������
	//recv�������ȡ�������е����ݣ�����������������ݵĻ�������Ҳ����0
	if(opmode==SOCKS_OP_PEEK)
		iret=v_peek(buf,buflen);
	else if(opmode==SOCKS_OP_READ)
	{
		if(m_maxRecvRatio!=0) //��������˽������������ж��Ƿ񳬹�����
		{
			time_t tNow=time(NULL);
			if( (m_recvBytes/(tNow-m_tmOpened+1))>m_maxRecvRatio) return 0; //����������
		}//?if(m_maxRecvRatio!=0)
		if( (iret=v_read(buf,buflen))>0 ) m_recvBytes+=iret;
	}
	else if(opmode==SOCKS_OP_ROOB && m_socktype==SOCKS_TCP) 
	{//��ȡ�������� ,only for TCP
		iret=::recv(m_sockfd,buf,buflen,MSG_NOSIGNAL|MSG_OOB);
	}
	else return 0;
	if(iret>0) return iret;

	if(iret==0) return SOCKSERR_CLOSED; //�������0��˵����socket�����Ѿ����Է��ر�(only for TCP)
	m_errcode=SOCK_M_GETERROR;
	return SOCKSERR_ERROR;//����ϵͳ����ͨ��SOCK_M_GETERROR��ô������ 
}

//��Ŀ�ķ�������
//���ط������ݵĴ�С�����<0��������
//�������==0���ʾ�����ٶȳ������趨���������ƣ����ݻ�����
//���ڷ�tcp���ӣ�����ǰҪ����setRemoteInfo���÷��͵�Ŀ�������Ͷ˿ڡ�
//ע��:���ڷ�tcp���ӣ�����Receiveʱ���Ὣ�������ݵ�Դ����ip�Ͷ˿�д��Ŀ�������Ͷ˿ڡ�
//
//�����øú���ʱ��send�ȱȽϴ��������ݵĳ���len���׽���s�ķ��ͻ������ĳ��ȣ�
//���len����s�ķ��ͻ������ĳ��ȣ��ú�������SOCKET_ERROR�����lenС�ڻ��ߵ���s�ķ��ͻ�����
//�ĳ��ȣ���ôsend�ȼ��Э���Ƿ����ڷ���s�ķ��ͻ����е����ݣ�����Ǿ͵ȴ�Э������ݷ����꣬
//���Э�黹û�п�ʼ����s�ķ��ͻ����е����ݻ���s�ķ��ͻ�����û�����ݣ���ôsend�ͱȽ�s�ķ���
//��������ʣ��ռ��len�����len����ʣ��ռ��Сsend��һֱ�ȴ�Э���s�ķ��ͻ����е����ݷ���
//�꣬���lenС��ʣ��ռ��Сsend�ͽ�����buf�е�����copy��ʣ��ռ��ע�Ⲣ����send��s�ķ�
//�ͻ����е����ݴ������ӵ���һ�˵ģ�����Э�鴫�ģ�send�����ǰ�buf�е�����copy��s�ķ��ͻ���
//����ʣ��ռ�������send����copy���ݳɹ����ͷ���ʵ��copy���ֽ��������send��copy����ʱ
//���ִ�����ôsend�ͷ���SOCKET_ERROR�����send�ڵȴ�Э�鴫������ʱ����Ͽ��Ļ�����ôsend
//����Ҳ����SOCKET_ERROR��Ҫע��send������buf�е����ݳɹ�copy��s�ķ��ͻ����ʣ��ռ������
//�ͷ����ˣ����Ǵ�ʱ��Щ���ݲ���һ�����ϱ��������ӵ���һ�ˡ�
//ע�⣺��Unixϵͳ�£����send�ڵȴ�Э�鴫������ʱ����Ͽ��Ļ�������send�Ľ��̻���յ�һ��
//SIGPIPE�źţ����̶Ը��źŵ�Ĭ�ϴ����ǽ�����ֹ��
inline SOCKSRESULT socketBase :: _Send(const char *buf,size_t buflen,time_t lWaitout)
{
	SOCKADDR_IN addr; memcpy((void *)&addr,(const void *)&m_remoteAddr,sizeof(addr));
	//���UDPû��ָ�����͵�Ŀ���򷵻ش���
//	if(m_socktype==SOCKS_UDP && m_remoteAddr.sin_port==0) return SOCKSERR_HOST;
	
	time_t tNow=time(NULL);
	if(m_maxSendRatio!=0) //��������˷������������ж��Ƿ񳬹�����
		if( (m_sendBytes/(tNow-m_tmOpened+1))>m_maxSendRatio) return 0; //����������
	while(lWaitout>=0)
	{
		int iret=checkSocket(SCHECKTIMEOUT,SOCKS_OP_WRITE);
		if(iret<0) { m_errcode=SOCK_M_GETERROR; return SOCKSERR_ERROR; }
		if(iret>0) break; //socket��д
		if(m_parent && m_parent->m_sockstatus<=SOCKS_CLOSED) 
			return SOCKSERR_PARENT;
		if( (time(NULL)-tNow)>lWaitout ) return SOCKSERR_TIMEOUT; //��ʱ
	}//?while
	
	int iret=(m_socktype==SOCKS_UDP)?v_writeto(buf,buflen,addr):v_write(buf,buflen);
	if(iret>=0) { m_sendBytes+=iret; return iret; }//����ʵ�ʷ��͵��ֽ���
	
	m_errcode=SOCK_M_GETERROR;
	if(iret==SOCKET_ERROR && m_errcode==WSAEACCES) 
		return SOCKSERR_EACCES; //ָ���ĵ�ַ��һ���㲥��ַ����û�����ù㲥��־
	//����iretӦ�õ���-1,��ʱ����send/sendto���÷����˴���
	return SOCKSERR_ERROR;//����ϵͳ����ͨ��SOCK_M_GETERROR��ô������ 
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
NetEnv netEnv;

NetEnv::NetEnv()
{
	m_bState=true;
#ifdef WIN32
	if(WSAStartup(MAKEWORD(2,2),&m_wsadata)!=0)
		m_bState=false;
#endif
}

NetEnv::~NetEnv()
{
#ifdef WIN32
	if(m_bState) WSACleanup();
#endif
}

NetEnv &getInstance(){ return netEnv; }
