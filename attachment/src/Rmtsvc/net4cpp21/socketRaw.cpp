/*******************************************************************
   *	socketRaw.cpp
   *    DESCRIPTION:Raw socket ���ʵ��
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2005-12-10
   *	
   *	net4cpp 2.1
   *******************************************************************/
#ifdef WIN32 //windowsϵͳƽ̨
/*
 * �����ڰ���windows.h֮ǰ����winsock2.h
 */
	#include <winsock2.h>
/*
 * ws2tcpip.h must be explicitly included after winsock2.h
 */
	#include <ws2tcpip.h>
/*
 * mstcpip.h must be explicitly included after winsock2.h
 */
	#include "mstcpip.h"
#elif defined MAC //��ʱ��֧��
	//....
#else  //unix/linuxƽ̨
	//����������socketΪ����ģʽʱ�õ���ͷ�ļ�
	//SUN unix����if.h�ļ��ж����˽ṹ struct map,���STL��map�����ͻ
	//��������δ�õ�struct map�ṹ������ú꽫�����!!! 
	#define map unix_netmap
	#include <net/if.h>
	#undef map //map
	//���û���ģʽʱ�õ���SIOCSIFFLAGS�Ķ��壬linux��unixҪ������ͬ��ͷ�ļ�
	#include <sys/ioctl.h>
	//Sun OS�������sys/sockio.hͷ�ļ�
	//��linux�������sys/ioctl.hͷ�ļ�
	#ifndef SIOCSIFFLAGS
		    #include <sys/sockio.h>
	#endif
#endif

#include "include/sysconfig.h"
#include "include/socketRaw.h"
#include "include/cLogger.h"

using namespace std;
using namespace net4cpp21;

socketRaw :: socketRaw()
{
	m_TTL=IP_DEF_TTL;
	m_SourceAddress.assign(socketBase::getLocalHostIP());
	//��0
	memset((void *)&m_IpV4Header,0,sizeof(m_IpV4Header));
	memset((void *)&m_ProtocolHeader,0,sizeof(m_ProtocolHeader));
}

//���ؽ���IPV4���ɹ����������û����ݴ�С
unsigned short socketRaw :: dataLen_ipv4()
{ 
	switch(m_IpV4Header.Protocol)
	{
		case IPPROTO_TCP://��ȡTCP�ײ�����
			{
			//��ȡIP�ײ�����
			char ipHeaderLen=m_IpV4Header.get_IpHeaderLen();
			//��ȡTCP�ײ�����
			char tcpHeaderLen=(char)m_ProtocolHeader.m_TcpHeader.get_DataOffset();
			return m_IpV4Header.TotalLength-(ipHeaderLen+tcpHeaderLen)*4;
			}
		case IPPROTO_UDP:
			return m_ProtocolHeader.m_UdpHeader.Length-8;
		default:
			//��ȡIP�ײ�����
			char ipHeaderLen=m_IpV4Header.get_IpHeaderLen();
			return m_IpV4Header.TotalLength-ipHeaderLen*4;
	}//?switch
	return 0;
}
//����IPV4Э���
//tcp/IP����Э��涨��һ��ͳһ�������ֽ�˳������˳�򱻹涨Ϊlittle-endian˳�򡣼����ֽ���ǰ�����ֽ��ں�
char * socketRaw :: decode_ipv4(char *buf,int len)
{
	char *ptr=buf;
	if(ptr!=NULL&&len>=IpV4_Min_Header_Length)
	{
		m_IpV4Header.IHL_Version=*ptr; ptr++;
		m_IpV4Header.TypeOfService=*ptr; ptr++;
		
		m_IpV4Header.TotalLength=*((unsigned short *)ptr); ptr+=2;
		m_IpV4Header.TotalLength=ntohs(m_IpV4Header.TotalLength);
		
		m_IpV4Header.Identification=*((unsigned short *)ptr); ptr+=2;
		m_IpV4Header.Identification=ntohs(m_IpV4Header.Identification);
		
		m_IpV4Header.Frag_and_flags=*((unsigned short *)ptr); ptr+=2;
		m_IpV4Header.Frag_and_flags=ntohs(m_IpV4Header.Frag_and_flags);
		
		m_IpV4Header.TimeToLive=*ptr; ptr++;
		
		m_IpV4Header.Protocol=*ptr; ptr++;
		
		m_IpV4Header.HeaderChecksum=*((unsigned short *)ptr); ptr+=2;
		m_IpV4Header.HeaderChecksum=ntohs(m_IpV4Header.HeaderChecksum);
		
		m_IpV4Header.SourceAddress=*((unsigned long *)ptr); ptr+=4;
		//m_IpV4Header.SourceAddress=ntohl(m_IpV4Header.SourceAddress);
		
		m_IpV4Header.DestinationAddress=*((unsigned long *)ptr); ptr+=4;
		//m_IpV4Header.DestinationAddress=ntohl(m_IpV4Header.DestinationAddress);
		
		//��ȡIP�ײ�����
		char ipHeaderLen=m_IpV4Header.get_IpHeaderLen();
		if(len>=m_IpV4Header.TotalLength)
		{
			//�жϽ������ݳ����Ƿ����=IP������
			//��ȡ�²�Э����׵�ַ
			ptr=buf+ipHeaderLen*4;
			switch(m_IpV4Header.Protocol)
			{
				case IPPROTO_TCP:
				{
					m_ProtocolHeader.m_TcpHeader.SourcePort=*((unsigned short *)ptr); ptr+=2;
					m_ProtocolHeader.m_TcpHeader.SourcePort=ntohs(m_ProtocolHeader.m_TcpHeader.SourcePort);
					
					m_ProtocolHeader.m_TcpHeader.DestinationPort=*((unsigned short *)ptr); ptr+=2;
					m_ProtocolHeader.m_TcpHeader.DestinationPort=ntohs(m_ProtocolHeader.m_TcpHeader.DestinationPort);
					
					m_ProtocolHeader.m_TcpHeader.SequenceNumber=*((unsigned int *)ptr); ptr+=4;
					m_ProtocolHeader.m_TcpHeader.SequenceNumber=ntohl(m_ProtocolHeader.m_TcpHeader.SequenceNumber);
					
					m_ProtocolHeader.m_TcpHeader.AcknowledgementNumber=*((unsigned int *)ptr); ptr+=4;
					m_ProtocolHeader.m_TcpHeader.AcknowledgementNumber=ntohl(m_ProtocolHeader.m_TcpHeader.AcknowledgementNumber);
					
					m_ProtocolHeader.m_TcpHeader.th_flag_res_offset=*((unsigned short *)ptr); ptr+=2;
					//yyc remove �����иߵ��ֽڱ任
//					m_ProtocolHeader.m_TcpHeader.th_flag_res_offset=ntohs(m_ProtocolHeader.m_TcpHeader.th_flag_res_offset);
					
					m_ProtocolHeader.m_TcpHeader.Window=*((unsigned short *)ptr); ptr+=2;
					m_ProtocolHeader.m_TcpHeader.Window=ntohs(m_ProtocolHeader.m_TcpHeader.Window);
					
					m_ProtocolHeader.m_TcpHeader.Checksum=*((unsigned short *)ptr); ptr+=2;
					m_ProtocolHeader.m_TcpHeader.Checksum=ntohs(m_ProtocolHeader.m_TcpHeader.Checksum);
					
					m_ProtocolHeader.m_TcpHeader.UrgentPointer=*((unsigned short *)ptr); ptr+=2;
					m_ProtocolHeader.m_TcpHeader.UrgentPointer=ntohs(m_ProtocolHeader.m_TcpHeader.UrgentPointer);
					
					//��ȡTCP�ײ�����
					char tcpHeaderLen=(char)m_ProtocolHeader.m_TcpHeader.get_DataOffset();
					//��ȡTCPЭ��������׵�ַ ,�û����ݳ���=m_IpV4Header.TotalLength-(ipHeaderLen+tcpHeaderLen)*4
					ptr=buf+ipHeaderLen*4+tcpHeaderLen*4;
					return ptr;
				}
					break;
				case IPPROTO_UDP:
				{
					m_ProtocolHeader.m_UdpHeader.SourcePort=*((unsigned short *)ptr); ptr+=2;
					m_ProtocolHeader.m_UdpHeader.SourcePort=ntohs(m_ProtocolHeader.m_UdpHeader.SourcePort);
					
					m_ProtocolHeader.m_UdpHeader.DestinationPort=*((unsigned short *)ptr); ptr+=2;
					m_ProtocolHeader.m_UdpHeader.DestinationPort=ntohs(m_ProtocolHeader.m_UdpHeader.DestinationPort);
					
					m_ProtocolHeader.m_UdpHeader.Length=*((unsigned short *)ptr); ptr+=2;
					m_ProtocolHeader.m_UdpHeader.Length=ntohs(m_ProtocolHeader.m_UdpHeader.Length);
					
					m_ProtocolHeader.m_UdpHeader.Checksum=*((unsigned short *)ptr); ptr+=2;
					m_ProtocolHeader.m_UdpHeader.Checksum=ntohs(m_ProtocolHeader.m_UdpHeader.Checksum);
					
					//��ȡUDPЭ��������׵�ַ
					//ptr=ptr+0; //�û����ݳ���=m_ProtocolHeader.m_UdpHeader.Length-8
					return ptr;
				}
					break;
				default:
					return ptr; //����Э����ֱ�ӷ���Э��������ʼ
			}
		}//?if(len>=m_IpV4Header.TotalLength)
	}//?if(ptr!=NULL&&len>=IpV4_Min_Header_Length)
	return NULL;
}
//��ָ�����û����ݱ���ΪIPV4���ֽ���,���ر������ֽ�����С
//data --- �û�����ָ��, datalen --- �û����ݳ���
//encodebuf --- ��ű�����ip���ֽ�����һ��һ��ip���ĳ��Ȳ�Ӧ����1500������û�ֻҪ����һ��4096��С�Ŀռ��㹻
int socketRaw :: encode_ipv4(const char *data,int datalen,char *encodebuf)
{
	if(encodebuf==NULL) return 0;
	if(data==NULL) datalen=0;
	char *ptr=encodebuf;
	*ptr=m_IpV4Header.IHL_Version; ptr++;
	*ptr=m_IpV4Header.TypeOfService; ptr++;
	m_IpV4Header.TotalLength=0;
	*((unsigned short *)ptr)=htons(m_IpV4Header.TotalLength); ptr+=2;
	*((unsigned short *)ptr)=htons(m_IpV4Header.Identification); ptr+=2;
	*((unsigned short *)ptr)=htons(m_IpV4Header.Frag_and_flags); ptr+=2;
	*ptr=m_IpV4Header.TimeToLive; ptr++;
	*ptr=m_IpV4Header.Protocol; ptr++;
			
	m_IpV4Header.HeaderChecksum=0;
	*((unsigned short *)ptr)=htons(m_IpV4Header.HeaderChecksum); ptr+=2;	
	*((unsigned int *)ptr)=m_IpV4Header.SourceAddress; ptr+=4; //htonl(m_IpV4Header.SourceAddress); ptr+=4;
	*((unsigned int *)ptr)=m_IpV4Header.DestinationAddress; ptr+=4;//htonl(m_IpV4Header.DestinationAddress); ptr+=4;

	switch(m_IpV4Header.Protocol)
	{
		case IPPROTO_TCP:
		{
			char *pstart_tcp=ptr;//tcp�ײ���ʼָ��
			//��TCP�ײ�������Ϊ5
			m_ProtocolHeader.m_TcpHeader.set_DataOffset(Tcp_Min_Header_Length/4);
			
			//���α�ײ������ڼ���У��ͣ������������ͣ�
			*((unsigned int *)ptr)=htonl(m_IpV4Header.SourceAddress);
			*((unsigned int *)(ptr+4))=htonl(m_IpV4Header.DestinationAddress);
			*(ptr+8)=0;
			*(ptr+9)=IPPROTO_TCP;//Э������
			*((unsigned short *)(ptr+10))=htons((m_ProtocolHeader.m_TcpHeader.th_flag_res_offset & 0x00f0)>>4);//TCP�ײ�����
			
			ptr+=12;//α�ײ�12�ֽڳ�
			*((unsigned short *)ptr)=htons(m_ProtocolHeader.m_TcpHeader.SourcePort); ptr+=2;
			*((unsigned short *)ptr)=htons(m_ProtocolHeader.m_TcpHeader.DestinationPort); ptr+=2;
			
			*((unsigned int *)ptr)=htonl(m_ProtocolHeader.m_TcpHeader.SequenceNumber); ptr+=4;
			*((unsigned int *)ptr)=htonl(m_ProtocolHeader.m_TcpHeader.AcknowledgementNumber); ptr+=4;
			
			//yyc modify
//			*((unsigned short *)ptr)=htons(m_ProtocolHeader.m_TcpHeader.th_flag_res_offset); ptr+=2;
			*((unsigned short *)ptr)=m_ProtocolHeader.m_TcpHeader.th_flag_res_offset; ptr+=2;

			*((unsigned short *)ptr)=htons(m_ProtocolHeader.m_TcpHeader.Window); ptr+=2;
			m_ProtocolHeader.m_TcpHeader.Checksum=0;
			*((unsigned short *)ptr)=htons(m_ProtocolHeader.m_TcpHeader.Checksum); ptr+=2;
			*((unsigned short *)ptr)=htons(m_ProtocolHeader.m_TcpHeader.UrgentPointer); ptr+=2;
			if(datalen>0)
			{//�������
				memcpy(ptr,data,datalen); ptr+=datalen; }
			//����TCPУ���
			m_ProtocolHeader.m_TcpHeader.Checksum=socketRaw::checksum((unsigned short *)pstart_tcp,ptr-pstart_tcp);
			*((unsigned short *)(pstart_tcp+12+16))=htons(m_ProtocolHeader.m_TcpHeader.Checksum);
			//ȥ��α�ײ�
			memmove((void *)pstart_tcp,(void *)(pstart_tcp+12),ptr-pstart_tcp-12);
			m_IpV4Header.TotalLength=ptr-encodebuf-12;//ȥ��һ��α�ײ��ĳ���12
		}
		break;
		case IPPROTO_UDP:
		{
			char *pstart_udp=ptr;
			//UDP����
			m_ProtocolHeader.m_UdpHeader.Length=datalen+8;
			//���α�ײ������ڼ���У��ͣ������������ͣ�
			*((unsigned int *)ptr)=htonl(m_IpV4Header.SourceAddress);
			*((unsigned int *)(ptr+4))=htonl(m_IpV4Header.DestinationAddress);
			*(ptr+8)=0;
			*(ptr+9)=IPPROTO_UDP;//Э������
			*((unsigned short *)(ptr+10))=htons(m_ProtocolHeader.m_UdpHeader.Length);//UDP����
			
			ptr+=12;//α�ײ�12�ֽڳ�
			*((unsigned short *)ptr)=htons(m_ProtocolHeader.m_UdpHeader.SourcePort); ptr+=2;
			*((unsigned short *)ptr)=htons(m_ProtocolHeader.m_UdpHeader.DestinationPort); ptr+=2;
			
			*((unsigned short *)ptr)=htons(m_ProtocolHeader.m_UdpHeader.Length); ptr+=2;
			m_ProtocolHeader.m_UdpHeader.Checksum=0;
			*((unsigned short *)ptr)=htons(m_ProtocolHeader.m_UdpHeader.Checksum); ptr+=2;
			if(datalen>0)
			{//�������
				memcpy(ptr,data,datalen); ptr+=datalen; }
			//����UDPУ���
			m_ProtocolHeader.m_UdpHeader.Checksum=socketRaw::checksum((unsigned short *)pstart_udp,m_ProtocolHeader.m_UdpHeader.Length+12);
			*((unsigned short *)(pstart_udp+12+6))=htons(m_ProtocolHeader.m_UdpHeader.Checksum); 
			//ȥ��α�ײ�
			memmove((void *)pstart_udp,(void *)(pstart_udp+12),m_ProtocolHeader.m_UdpHeader.Length);
			m_IpV4Header.TotalLength=ptr-encodebuf-12;//ȥ��һ��α�ײ��ĳ���12
		}
		break;
		default:
			if(datalen>0)
			{//�������
				memcpy(ptr,data,datalen); ptr+=datalen; }
			m_IpV4Header.TotalLength=ptr-encodebuf;
		break;
	}//?switch
	if(m_IpV4Header.TotalLength>0){
		*((unsigned short *)(encodebuf+2))=htons(m_IpV4Header.TotalLength);
		//����IP�ײ�У���
		m_IpV4Header.HeaderChecksum=socketRaw::checksum((unsigned short *)encodebuf,(m_IpV4Header.IHL_Version & 0x0f)*4);
		*((unsigned short *)(encodebuf+10))=htons(m_IpV4Header.HeaderChecksum);
		if(m_IpV4Header.Protocol==IPPROTO_TCP && datalen==0){
			::memset(encodebuf+m_IpV4Header.TotalLength,0,4); return m_IpV4Header.TotalLength+4;}
	}
	return m_IpV4Header.TotalLength;
}

void socketRaw :: ConstructIPV4Header(unsigned char  ucProtocol,unsigned char  ucHeaderLength)
{
	memset((void *)&m_IpV4Header,0,sizeof(IpV4Header));
	m_IpV4Header.IHL_Version=ucHeaderLength/4 + IpVersion*16;
	m_IpV4Header.Protocol=ucProtocol;
	m_IpV4Header.Frag_and_flags=0;//usFragmentationFlags;
	m_IpV4Header.Identification=rand();//usIdentification;
	m_IpV4Header.TypeOfService=IpService_ROUTINE;

	m_IpV4Header.TimeToLive=m_TTL;
	m_IpV4Header.SourceAddress=socketBase::Host2IP(m_SourceAddress.c_str());
	return;
}

//size --Ϊ�ֽڵĴ�С��������unsigned short����
unsigned short socketRaw :: checksum(unsigned short *buffer, int size)
{
	unsigned long cksum=0;
	while (size>1) 
	{
		cksum += *buffer++;
		size -= sizeof(unsigned short);
	}
	if(size) //�������ֽ�
	{
		cksum += *(unsigned char*)buffer;
	}
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >>16);
	return (unsigned short)(~cksum);
}
//bindip==NULL ������IP��==""�󶨱�����һ��ip
//�����ָ����ip
bool socketRaw :: create(int IPPROTO_type,const char *bindip)
{
	socketBase::create(SOCKS_RAW);
	int af=m_localAddr.sin_family;
	//linux ��ֻ��rootȨ�޲��ܴ���RAW_SOCKET
#ifndef WIN32   //You must be root user to create RAW_SOCKET
	if(geteuid() != 0) return false;//return SOCKSERR_NOROOT;
#endif
	if( (m_sockfd=::socket(af,SOCK_RAW ,IPPROTO_type))!=INVALID_SOCKET )
	{
		if(bindip){
			if(bindip[0]!=0) m_SourceAddress.assign(bindip);
			bindip=m_SourceAddress.c_str();
		}
		if(this->Bind(0,FALSE,bindip)>=0)
		{
			int on=1; //Set that the application will send the IP header
			if(setsockopt(m_sockfd, IPPROTO_IP, IP_HDRINCL, (char *) &on,sizeof(on))!=SOCKET_ERROR)
			{
				m_socktype=SOCKS_RAW;
				m_sockstatus=SOCKS_OPENED;
				return true;
			}
			RW_LOG_DEBUG(0,"Failed to setsockopt(...IPPROTO_IP, IP_HDRINCL...)\r\n");
		}
		else
			RW_LOG_DEBUG("Failed to Bind %s\r\n",bindip);
	}
	else 
		RW_LOG_DEBUG("Failed to create %d IPPROTO.\r\n",IPPROTO_type);
	return false;
/*  ����Э�����ƻ�ȡЭ���
	struct protoent *p = NULL; //yyc add 2005-08-24
	if(protocol!=NULL && protocol[0]!=0)
	{
		if( (p = getprotobyname( protocol))==NULL )
			RW_LOG_PRINT(LOGLEVEL_ERROR,"failed to getprotobyname(%s)\r\n",protocol);
	}
	int protno = p ? p -> p_proto : 0;//IPPROTO_RAW;//0(IPPROTO_IP);
*/
}

//����/���ָ��fd�Ļ���ģʽ
//b=true--set b=false---clear
//�ɹ�����SOCKSERR_OK(0)�������
//!!������û���ģʽ����socketRaw��������һ��IP
SOCKSRESULT socketRaw :: Set_Promisc(bool b)
{
	if(m_socktype!=SOCKS_RAW) return SOCKSERR_INVALID;
	
	SOCKSRESULT sr=SOCKSERR_OK;
#ifdef WIN32
	//����SOCK_RAWΪSIO_RCVALL���Ա�������е�IP��		
	DWORD dwValue =(b)?RCVALL_ON:RCVALL_OFF;
	sr=ioctlsocket(m_sockfd, SIO_RCVALL, &dwValue);
#else
	struct ifreq ifr;
	strncpy(ifr.ifr_name,"",1);//����Ҫ����Ϊ����ģʽ�������豸����!!!!!!!
	sr=ioctlsocket(m_sockfd,SIOCGIFFLAGS,&ifr);
	if(sr>=0)
	{
	//��Ϊ����ģʽ
		if(b)
			ifr.ifr_flags |= IFF_PROMISC;
		else
			ifr.ifr_flags &= ~IFF_PROMISC;
		if( (sr=ioctlsocket(m_sockfd, SIOCSIFFLAGS, &ifr))>=0 ) sr=SOCKSERR_OK;	
	}
#endif
	if(sr==SOCKET_ERROR) m_errcode=SOCK_M_GETERROR;
	return sr;
}
