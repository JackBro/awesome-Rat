/*******************************************************************
   *	proxyclnt.cpp
   *    DESCRIPTION:����Э��ͻ���ʵ��
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2005-11-20
   *
   *	net4cpp 2.1
   *	֧��HTTP��socks4��socks5
   *******************************************************************/

#include "../include/sysconfig.h"
#include "../include/proxyclnt.h"
#include "../include/cCoder.h"
#include "../include/cLogger.h"

using namespace std;
using namespace net4cpp21;


socketProxy :: socketProxy()
{
	m_proxytype=PROXY_NONE;
	m_proxyport=0;
	m_dnsType=0;
}
socketProxy :: ~socketProxy()
{
	Close();
	m_thread.join();
}

bool socketProxy :: setProxy(PROXYTYPE proxytype,const char *proxyhost,
							 int proxyport,const char *user,const char *pwd)
{
	m_proxytype=proxytype;
	if(proxyhost) m_proxyhost.assign(proxyhost);
	m_proxyport=proxyport;
	if(user) m_proxyuser.assign(user);
	if(pwd) m_proxypwd.assign(pwd);
	return m_proxytype!=PROXY_NONE;
}

void socketProxy :: setProxy(socketProxy &socks)
{
	m_proxytype=socks.m_proxytype;
	m_proxyhost=socks.m_proxyhost;
	m_proxyport=socks.m_proxyport;
	m_proxyuser=socks.m_proxyuser;
	m_proxypwd=socks.m_proxypwd;
	return;
}

//����ָ����TCP���񡣳ɹ����ر���socket�˿�>0�����򷵻ش���
SOCKSRESULT socketProxy :: Connect(const char *host,int port,time_t lWaitout)
{
	if(host==NULL||host[0]==0||port<=0) return SOCKSERR_HOST;
	if(m_proxytype==PROXY_NONE) //ֱ������ָ������
		return socketTCP::Connect(host,port,lWaitout);
	//���������Ӵ��������
	SOCKSRESULT sr=socketTCP::Connect(m_proxyhost.c_str(),m_proxyport,lWaitout);
	if(sr<0){
		RW_LOG_DEBUG("Failed to connect proxy server(%s:%d),error=%d\r\n"
			,m_proxyhost.c_str(),m_proxyport,sr);
		return sr;//����ʧ��
	}
	SOCKSRESULT sr1=sendReq_Connect(host,port,lWaitout);
	if(sr1==SOCKSERR_OK) return sr;
	this->Close(); 
	return (sr1>0)?SOCKSERR_PROXY_REJECT:sr1;
}
//����socks BIND����ָ����socks�����Ͽ�һ�������˿�
//�ɹ����ؿ����Ķ˿�>0 ����ʧ��
//[out] svrIP/svrPort����socks���񿪵Ķ˿ں�IP��ַ
//[in] svrIP,����socks���񿪵Ķ˿ڵȴ��������Ǹ�ԴIP���ӣ�svrPort������
SOCKSRESULT socketProxy :: Bind(std::string &svrIP,int &svrPort,time_t lWaitout)
{
	if(m_proxytype!=PROXY_SOCKS4 && m_proxytype!=PROXY_SOCKS5) 
		return SOCKSERR_NOTSURPPORT;
	//�����Ӵ��������
	SOCKSRESULT sr=socketTCP::Connect(m_proxyhost.c_str(),m_proxyport,lWaitout);
	if(sr<0){
		RW_LOG_DEBUG("Failed to connect proxy server(%s:%d),error=%d\r\n"
			,m_proxyhost.c_str(),m_proxyport,sr);
		return sr;//����ʧ��
	}
	if( (sr=sendReq_Bind(svrIP,svrPort,lWaitout))!=SOCKSERR_OK || svrPort<=0)
	{
		this->Close();
		return (sr>0)?SOCKSERR_PROXY_REJECT:sr;
	}
	return svrPort;
}
//����UDP����Э������ (����socks5����Э��֧��UDP����)
//�ɹ����ؿ�����UDP�˿�>0 ����ʧ��
//[in] svrIP/svrPort ����UDP�˿ں�IP
//[out] svrIP/svrPort����socks���񿪵Ķ˿ں�IP��ַ
SOCKSRESULT socketProxy :: UdpAssociate(std::string &svrIP,int &svrPort,time_t lWaitout)
{
	if(m_proxytype!=PROXY_SOCKS5) return SOCKSERR_NOTSURPPORT;
	//�����Ӵ��������
	SOCKSRESULT sr=socketTCP::Connect(m_proxyhost.c_str(),m_proxyport,lWaitout);
	if(sr<0){
		RW_LOG_DEBUG("Failed to connect proxy server(%s:%d),error=%d\r\n"
			,m_proxyhost.c_str(),m_proxyport,sr);
		return sr;//����ʧ��
	}
	if( (sr=sendReq_UdpAssociate(svrIP,svrPort,lWaitout))!=SOCKSERR_OK || svrPort<=0)
	{
		this->Close();
		return (sr>0)?SOCKSERR_PROXY_REJECT:sr;
	}
	return svrPort;
}

//���ʹ���BIND���󣬳ɹ�����SOCKSERR_OK(0)
//��SOCKS Server�Ͽ�һ����������,svrIP/svrPort����socks���񿪵Ķ˿ں�IP��ַ
//[in] svrIP,����socks���񿪵Ķ˿ڵȴ��������Ǹ�ԴIP���ӣ�svrPort������
SOCKSRESULT socketProxy :: sendReq_Bind(std::string &svrIP,int &svrPort,time_t lWaitout)
{
	SOCKSRESULT sr=SOCKSERR_OK;
	unsigned long ipAddr=INADDR_NONE;
	if(!m_ipv6) ipAddr=socketBase::Host2IP(svrIP.c_str());
	if(ipAddr==INADDR_NONE) return SOCKSERR_HOST;//��������Ч��IPv4��ַ
	char buf[256]; int buflen=0;
	if(m_proxytype==PROXY_SOCKS4)
	{//���sock4req�ṹ
		buf[0]=0x04; buf[1]=0x02; //BIND
		unsigned short rp=htons((unsigned short)svrPort);
		::memcpy((void *)(buf+2),(const void *)&rp,sizeof(rp));
		buflen=2+sizeof(rp);
		::memcpy((void *)(buf+buflen),(const void *)&ipAddr,sizeof(ipAddr));
		buflen+=sizeof(ipAddr);
		buf[buflen++]=0;
	}
	else if(m_proxytype==PROXY_SOCKS5)
	{
		if( !socks5_Negotiation(lWaitout) ) return SOCKSERR_PROXY_AUTH;
		buf[0]=0x05; buf[1]=0x02; //���sock5req�ṹ BIND
		buf[2]=0x0; buf[3]=0x1; //ָ����ipv4�ĵ�ַ
		::memcpy((void *)(buf+4),(const void *)&ipAddr,sizeof(ipAddr));
		buflen=4+sizeof(ipAddr);
		unsigned short rp=htons((unsigned short)svrPort);
		::memcpy((void *)(buf+buflen),(const void *)&rp,sizeof(rp));
		buflen+=sizeof(rp);
	}
	else return SOCKSERR_NOTSURPPORT;
	if( (sr=this->Send(buflen,(const char *)buf,-1))<0) return sr;
	if( (sr=this->Receive(buf,256,lWaitout))<0 ) return sr;
	
	if(m_proxytype==PROXY_SOCKS4)
	{
		if(buf[1]==90) //==0x5A
		{
			sr=SOCKSERR_OK;
			//��ȡ���񷵻صĿ����������˿ں�IP
			svrPort=htons(*(unsigned short *)(buf+2));
			ipAddr=*(unsigned long *)(buf+2+sizeof(unsigned short));
			if(ipAddr==INADDR_ANY) ipAddr=this->m_remoteAddr.sin_addr.s_addr;
			svrIP.assign(socketBase::IP2A(ipAddr));
		}
		else sr=buf[1];
	}//?if(m_proxytype==PROXY_SOCKS4)
	else //m_proxytype==PROXY_SOCKS5
	{
		if(buf[1]==0x0) 
		{
			sr=SOCKSERR_OK;
			char atyp=buf[3];
			if(atyp!=0x01) return SOCKSERR_PROXY_ATYP;
			//��ȡ���񷵻صĿ����������˿ں�IP
			ipAddr=*(unsigned long *)(buf+4);
			if(ipAddr==INADDR_ANY) ipAddr=this->m_remoteAddr.sin_addr.s_addr;
			svrIP.assign(socketBase::IP2A(ipAddr));
			svrPort=htons( *(unsigned short *)(buf+4+sizeof(ipAddr)) );
		}
		else sr=buf[1];
	}
	return sr;
}
//����socks Bind�����ȴ����ӵ���(���ڶ���Bind��Ӧ)
//bool BindedEvent�Ƿ����onBinded�¼�֪ͨ��ʽ
bool socketProxy :: WaitForBinded(time_t lWaitout,bool BindedEvent)
{
//	if(BindedEvent) //�����ȴ������߳�
//		return m_thread.start((THREAD_CALLBACK *)&socketProxy::bindThread,(void *)this);
	
	bool bAccept=false;
	if(this->m_proxytype==PROXY_SOCKS4)
	{
		char buf[8];
		if( this->Receive(buf,8,lWaitout)==8 && buf[1]==0x5a ) 
			bAccept=true;
	}
	if(this->m_proxytype==PROXY_SOCKS5)
	{
		char buf[262];
		if( this->Receive(buf,5,lWaitout)==5 && buf[1]==0)
		{//�Ƚ���5�ֽ��жϵ�ַ����
			int len=0;
			if(buf[3]=0x01) //ipv4
				len=10; //4+4+2
			else if(buf[3]==0x04) //ipv6
				len=22; //4+16+2
			else if(buf[3]==0x03) //ȫ������
				len=7+(unsigned char)buf[4];//4+1+buf[4]+2;
			//ʵ�ʻ�δ��������ֽ�
			if( (len-=5)>0 && this->Receive(buf+5,len,-1)>0)
				bAccept=true;
		}
	}
	return bAccept;
}

//����socks Bind����������ĵȴ����ӵ�����߳�(���ڶ���Bind��Ӧ)
//����Bind����������һ���Ϸ��û����ӵ�socks�����������onBinded�¼�
//��ʱ�ɽ������ݽ������ͺ����û�ֱ���������ط���Ķ˿��϶�����socks���񿪵Ķ˿���һ��
//bAccept --- bind����ĵڶ�����Ӧ�Ǿܾ�����ͬ��
//void socketProxy :: bindThread(socketProxy *psock)
//{
//	if(psock==NULL) return;
//	psock->onBinded(psock->WaitForBinded(-1,false));
//	return;
//}

//���ʹ����������󣬳ɹ�����SOCKSERR_OK(0)
SOCKSRESULT socketProxy :: sendReq_Connect(const char *host,int port,time_t lWaitout)
{
	SOCKSRESULT sr=SOCKSERR_OK;
	char buf[256]; int buflen=0;
	if(m_proxytype==PROXY_SOCKS5)
	{
		unsigned long ipAddr=INADDR_NONE;
		if(!m_ipv6) ipAddr=socketBase::Host2IP(host);
		//���Ҫ�󱾵ؽ���
		if(ipAddr==INADDR_NONE && m_dnsType==1 ) return SOCKSERR_HOST;
		if( !socks5_Negotiation(lWaitout) ) return SOCKSERR_PROXY_AUTH;
		buf[0]=0x05; buf[1]=0x01;
		buf[2]=0x00; buflen=3;
		if(ipAddr==INADDR_NONE){
			buf[3]=0x03;
			buf[4]=sprintf(buf+5,"%s",host)+1; //����NULL��������
			buflen=5+buf[4]; buf[buflen-1]=0;
		}
		else{
			buf[3]=0x01;
			::memcpy((void *)(buf+4),(const void *)&ipAddr,sizeof(ipAddr));
			buflen=4+sizeof(ipAddr);
		}
		unsigned short rp=htons((unsigned short)port);
		::memcpy((void *)(buf+buflen),(const void *)&rp,sizeof(rp));
		buflen+=sizeof(rp);
	}//?if(m_proxytype==PROXY_SOCKS5)
	else if(m_proxytype==PROXY_HTTPS)
	{
		string strAuth;
		if(m_proxyuser!=""){//��֤��Ϣ
			strAuth=m_proxyuser+string(":")+m_proxypwd;
			if(cCoder::Base64EncodeSize(strAuth.length())>=256)
			{
				strAuth="";
				RW_LOG_PRINT(LOGLEVEL_ERROR,0,"���������ʺ��޷�base64���룬��������������\r\n");
			}
			else
			{
				buflen=cCoder::base64_encode((char *)strAuth.c_str(),strAuth.length(),buf);
				strAuth.assign("Proxy-Authorization: Basic ");
				strAuth.append(buf,buflen); strAuth.append("\r\n");
			}
		}//?if(m_proxyuser!=""){//��֤��Ϣ
		buflen=sprintf(buf,"CONNECT %s:%d HTTP/1.1\r\nHost: %s:%d\r\n%sUser-Agent: yyproxy\r\n\r\n",
			host,port,host,port,strAuth.c_str());
	}//?else if(m_proxytype==PROXY_HTTPS)
	else //PROXY_SOCKS4
	{
		//����socks4 CONNECT������������ָ��������
		//socks4AЭ�����ָ��Զ������������������������ip
		//��ʽ���£�(ԭsock4Э��IP��ַ��������0.0.0.x),��buf[4]=buf[5]=buf[6]=0,buf[7]=x
		//[�汾] [����] [�˿�] [0.0.0.1] [0] [hostdomain] [0]
		
		buf[0]=0x04; buf[1]=0x01; //���sock4req�ṹ
		unsigned short rp=htons((unsigned short)port);
		::memcpy((void *)(buf+2),(const void *)&rp,sizeof(rp));
		buflen=2+sizeof(rp);
		unsigned long ipAddr=INADDR_NONE;
		if(!m_ipv6) ipAddr=socketBase::Host2IP(host);
		if(ipAddr==INADDR_NONE) //Ҳ�����������Ч�������������˽���
		{
			ipAddr=0x01000000;
			::memcpy((void *)(buf+buflen),(const void *)&ipAddr,sizeof(ipAddr));
			buflen+=sizeof(ipAddr); buf[buflen++]=0;
			buflen+=sprintf(buf+buflen,"%s",host);buf[buflen++]=0;
		}
		else
		{
			::memcpy((void *)(buf+buflen),(const void *)&ipAddr,sizeof(ipAddr));
			buflen+=sizeof(ipAddr); buf[buflen++]=0;
		}
	}//?else //PROXY_SOCKS4
	if( (sr=this->Send(buflen,(const char *)buf,-1))<0) return sr;
	if( (sr=this->Receive(buf,256,lWaitout))<0 ) return sr;
	
	if(m_proxytype==PROXY_SOCKS5)
		sr=buf[1]; //������سɹ���buf[1]ӦΪ0
	else if(m_proxytype==PROXY_HTTPS)
	{
		if(strncmp(buf,"HTTP/1.",7)==0 && strncmp(buf+9,"200",3)==0) 
			sr=SOCKSERR_OK;
		else
			sr=SOCKSERR_PROXY_REJECT;
	}
	else //PROXY_SOCKS4
		sr=(buf[1]==90)?SOCKSERR_OK:buf[1];
	return sr;
}

//socks5Э����֤,�ɹ���������򷵻ؼ�
bool socketProxy :: socks5_Negotiation(time_t lWaitout)
{
	char buf[128];
	buf[0]=0x05; buf[1]=0x02;
	buf[2]=0x0; buf[3]=0x02;
	SOCKSRESULT sr=this->Send(4,(const char *)buf,-1);
	if(sr<0) return false;
	//����2�ֽڵ���Ӧ����
	sr=this->Receive(buf,2,lWaitout);
	if(sr!=2) return false;
	if(buf[1]==0x02)
	{//����Ҫ����֤
		int len=m_proxyuser.length()+m_proxypwd.length()+3;
		if(len>=128) return false; //�����˷��ͻ�������С
		buf[0]=0x01; buf[1]=(unsigned char)m_proxyuser.length();
		strncpy((buf+2),m_proxyuser.c_str(),m_proxyuser.length());
		
		buf[2+m_proxyuser.length()]=(unsigned char)m_proxypwd.length();//���볤��
		strncpy( (buf+3+m_proxyuser.length()),m_proxypwd.c_str(),m_proxypwd.length() );
		if( (sr=this->Send(len,(const char *)buf,-1))<0) return false;
		//����2�ֽڵ���Ӧ����
		sr=this->Receive(buf,2,lWaitout);
		if(sr!=2) return false;
		if(buf[1]!=0) return false; //��֤ʧ��
	}//?if(buf[1]==0x02)
	else if(buf[1]!=0) //��֧��Ҫ�����֤��ʽ
	{
		RW_LOG_PRINT(LOGLEVEL_WARN,"not surpport Authentication (%d).\r\n",buf[1]);
		return false;
	} //buf[1]==0��Ҫ����֤
	return true;
}

//����UDP�������󣬳ɹ�����SOCKSERR_OK(0) (����socks5����Э��֧��UDP����)
//��SOCKS Server�Ͽ�һ����������,svrIP/svrPort����socks���񿪵Ķ˿ں�IP��ַ
//[in] svrIP/svrPort ����UDP�˿ں�IP
SOCKSRESULT socketProxy :: sendReq_UdpAssociate(std::string &svrIP,int &svrPort,time_t lWaitout)
{
	SOCKSRESULT sr=SOCKSERR_OK;
//	if(m_proxytype!=PROXY_SOCKS5) return SOCKSERR_NOTSURPPORT;
	if( !socks5_Negotiation(lWaitout) ) return SOCKSERR_PROXY_AUTH;
	char buf[256]; int buflen=10;
	buf[0]=0x05; buf[1]=0x03; //���sock5req�ṹ BIND
	buf[2]=0x0; buf[3]=0x1; //ָ����ipv4�ĵ�ַ
	unsigned long ipAddr=INADDR_NONE;
	if(svrIP!="") ipAddr=socketBase::Host2IP(svrIP.c_str());
	if(ipAddr==INADDR_NONE) ipAddr=INADDR_ANY;
	*(unsigned long *)(buf+4)=ipAddr; //buf[4]=buf[5]=buf[6]=buf[7]=0;
	if(svrPort<0) svrPort=0;  //buf[8]=buf[9]=0;
	*(unsigned short *)(buf+8)=htons(svrPort);
	
	if( (sr=this->Send(buflen,(const char *)buf,-1))<0) return sr;
	if( (sr=this->Receive(buf,256,lWaitout))<0 ) return sr;
	
	if(buf[1]==0x0) 
	{
		sr=SOCKSERR_OK;
		char atyp=buf[3];
		if(atyp!=0x01) return SOCKSERR_PROXY_ATYP; //��֧�ֵĵ�ַ����
		//��ȡ���񷵻صĿ����������˿ں�IP
		unsigned long ipAddr=*(unsigned long *)(buf+4);
		if(ipAddr==INADDR_ANY) ipAddr=this->m_remoteAddr.sin_addr.s_addr;
		svrIP.assign(socketBase::IP2A(ipAddr));
		svrPort=htons( *(unsigned short *)(buf+4+sizeof(ipAddr)) );
	}
	else sr=buf[1];
	return sr;
}

