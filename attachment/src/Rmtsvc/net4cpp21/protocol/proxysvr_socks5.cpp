/*******************************************************************
   *	proxysvr.cp
   *    DESCRIPTION:��������ʵ��
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2006-08-24
   *
   *	net4cpp 2.1
   *	
   *******************************************************************/

#include "../include/sysconfig.h"
#include "../include/proxyclnt.h"
#include "../include/proxysvr.h"
#include "../include/socketUdp.h"
#include "../utils/utils.h"
#include "../include/cLogger.h"

using namespace std;
using namespace net4cpp21;


#define SOCKS5_MAX_PACKAGE_SIZE 1500
void transData_UDP(socketTCP *psock,socketUdp &sockUdp,unsigned long clntIP,int clntPort);
void cProxysvr :: doSock5req(socketTCP *psock)
{
	char buf[SOCKS5_MAX_PACKAGE_SIZE];
	int iret,recvlen=0; int istep=0; //socks5Э�̲���
	PROXYACCOUNT *ptr_proa=NULL;
	while( (iret=psock->Receive(buf+recvlen,SOCKS5_MAX_PACKAGE_SIZE-recvlen,PROXY_MAX_RESPTIMEOUT))>0 )
	{
		if( (recvlen+=iret)<2 ) continue; //����δ������
		if(istep==0) //��һ��socks5Э������
		{
			sock5req *psock5req=(sock5req *)buf;
			if(recvlen<(psock5req->nMethods+2)) continue; //����δ������
			//�������,��ʼ������Ϣ
			sock5ans ans; ans.Ver =5;
			ans.Method=(m_bProxyAuthentication)?2:0;//2Ҫ����֤,�ȴ�������֤��Ϣ����Ҫ����֤
			psock->Send(2 /*sizeof(sock5ans)*/,(const char *)&ans,-1);
			istep=(m_bProxyAuthentication)?1:2; //����1�ȴ�������֤
			recvlen=0; continue; //��ս��յ�����,����
		}
		else if(istep==1) //�ڶ���socks5������֤
		{
			authreq *pauthreq=(authreq *)buf;
			if(pauthreq->Ver!=1) break; //�����Э������
			if( recvlen<4 ) continue; //����δ������
			if(recvlen<(pauthreq->Ulen+3) ) continue; //����δ������
			unsigned char plen=*(unsigned char *)(buf+2+pauthreq->Ulen); //��ȡ���볤��
			if(recvlen<(pauthreq->Ulen+plen+3) ) continue; //����δ������
			//�������,��ʼ������Ϣ
			pauthreq->Name[pauthreq->Ulen]=0;
			char *strPass=(char *)(buf+2+pauthreq->Ulen+1);
			strPass[plen]=0;
//			pauthreq->Pass[pauthreq->PLen]=0; //error yyc modify 2007-01-10
			authans auans;auans.Ver=1;
			ptr_proa=ifAccess(psock,pauthreq->Name,strPass,NULL);
			if(ptr_proa) //��֤ͨ��
				auans.Status=0;  //�ɹ� //��֤ͨ��
			else auans.Status=1; //��֤��ͨ��
			psock->Send(2 /*sizeof(authans)*/,(const char *)&auans,-1);
			if(auans.Status!=0) break;
			istep=2; //����2
			recvlen=0; continue; //��ս��յ�����,����
		}//?else if(istep==1)
		else if(istep==2) //������������������
		{
			sock5req1 *psock5req1=(sock5req1 *)buf;
			char *hostip; int hostport=0;
			unsigned long IPAddr=INADDR_NONE;
			sock5ans1 ans1;ans1.Ver=5;ans1.Rsv=0;ans1.Atyp=1;
			if( recvlen<10 ) continue; //����δ������
			if(psock5req1->Atyp==1) //IP address
			{
				IPAddr =*((unsigned long *)psock5req1->other);
				hostport=ntohs(*((unsigned short *)(psock5req1->other+4)));
				hostip=NULL;
			}
			else if(psock5req1->Atyp==3) //domain name
			{
				int datalen=6+psock5req1->other[0]+1;
				if( recvlen<datalen ) continue; //����δ������
				hostport=ntohs(*((unsigned short *)(psock5req1->other+psock5req1->other[0]+1)));
				hostip=psock5req1->other+1; hostip[ psock5req1->other[0] ]=0;
			}
			else //��֧�ֵĵ�ַ����
			{
				ans1.Rep=8;//Address type not supported
				psock->Send(10 /*sizeof(sock5ans1)*/,(const char *)&ans1,-1);
				break; 
			}
			socketProxy peer; peer.setParent(psock);
			if(m_bCascade){ //�����˶�������
				PROXYTYPE ptype=PROXY_SOCKS5;
				if((m_casProxytype & PROXY_SOCKS5)==0) //��������֧��SOCKS5����
				{
					if(m_casProxytype & PROXY_HTTPS)
						ptype=PROXY_HTTPS;
					else if(m_casProxytype & PROXY_SOCKS4)
						ptype=PROXY_SOCKS4;
				}
				std::pair<std::string,int> * p=GetCassvr(); //��ȡ���������������
				if(m_casProxyAuthentication)
					peer.setProxy(ptype,p->first.c_str(),p->second,m_casAccessAuth.first.c_str(),m_casAccessAuth.second.c_str());
				else
					peer.setProxy(ptype,p->first.c_str(),p->second,"","");
			}//?if(m_bCascade)
			if(psock5req1->Cmd ==1) //tcp connect
			{//����ָ����Զ������,����ɹ���������ת����
				bool bAccessDest=true; //�Ƿ��������Ŀ�ķ���
				if(ptr_proa && ptr_proa->m_dstRules.rules()>0)
				{
					if(hostip) IPAddr=socketBase::Host2IP(hostip);
					bAccessDest=ptr_proa->m_dstRules.check(IPAddr,hostport,RULETYPE_TCP);
				}
				if(bAccessDest)
				{
//					RW_LOG_DEBUG("[ProxySvr] SOCKS5 - Connecting %s:%d ... \r\n",((hostip)?hostip:socketBase::IP2A(IPAddr)),hostport);

					if(m_bCascade) //�����˶�������
					{//ͨ��������������ָ����Ӧ�÷���
						if(hostip==NULL){
							strcpy(buf,socketBase::IP2A(IPAddr));
							hostip=buf; }
						peer.Connect(hostip,hostport,PROXY_MAX_RESPTIMEOUT); 
					}else{//����/IP��ַ����
						if(hostip) IPAddr=socketBase::Host2IP(hostip);
						peer.SetRemoteInfo(IPAddr,hostport);
						if( IPAddr!=INADDR_NONE) 
							peer.socketTCP::Connect( NULL,0,PROXY_MAX_RESPTIMEOUT);
					}//?if(m_bCascade)...else...
				}
				if(peer.status()==SOCKS_CONNECTED){
					ans1.Rep=0;
					ans1.IPAddr=peer.getRemoteip();
					ans1.Port=htons(peer.getRemotePort());
				}else ans1.Rep=5;
			}//?if(psock5req1->Cmd ==1)
			else if(psock5req1->Cmd ==2) //tcp bind
			{//�ͻ���������socks ����˽���һ����ʱ�������񣬵ȴ�ָ��hostip�����ӵ���
				if(m_bCascade) //�����˶�������
				{
					std::string svrip; int svrport=hostport;
					if(hostip) svrip.assign(hostip); 
					else svrip.assign(socketBase::IP2A(IPAddr));
					if( peer.Bind(svrip,svrport,PROXY_MAX_RESPTIMEOUT)>0 )
					{
						ans1.Rep=0;  ans1.Port=htons(svrport);
						ans1.IPAddr=peer.getRemoteip();//...//���ͳɹ���Ӧ 
						psock->Send(10 /*sizeof(sock5ans1)*/,(const char *)&ans1,-1);
						ans1.Port=0; ans1.IPAddr=0;
						ans1.Rep=1; //�ȴ��Է����ӣ������͵ڶ�����Ӧ
						if(peer.WaitForBinded(PROXY_MAX_RESPTIMEOUT,false))
						{
							ans1.Port=htons(peer.getRemotePort()); //...����Ӧ��Ϊʵ�ʵĶ˿�/IP???
							ans1.IPAddr=peer.getRemoteip(); //...
							ans1.Rep=0; //���ӳɹ�
						}
					}else ans1.Rep=1;
				}else if( (iret=peer.ListenX(0,FALSE,NULL)) > 0)
				{
					ans1.Rep=0;  ans1.Port=htons(iret);
					ans1.IPAddr=0; //psock->getLocalip();//���ͳɹ���Ӧ 
					psock->Send(10 /*sizeof(sock5ans1)*/,(const char *)&ans1,-1);
					ans1.Port=0; ans1.IPAddr=0;
					ans1.Rep=1; //�ȴ��Է����ӣ������͵ڶ�����Ӧ
					if(peer.Accept(PROXY_MAX_RESPTIMEOUT,NULL)>0)
					{
						ans1.Port=htons(peer.getRemotePort());
						ans1.IPAddr=peer.getRemoteip();
						ans1.Rep=0; //���ӳɹ�
					}
				}else ans1.Rep=1; //һ���Դ���
			}//?else if(psock5req1->Cmd ==2)
			else if(psock5req1->Cmd ==3) //udp����
			{
				socketUdp sockUdp; //��һ��UDP�˿���������ת������
				SOCKSRESULT sr=sockUdp.Open(0,false,NULL);
				if(sr>0){ //UDP�˿ڴ򿪳ɹ�
					std::string svrip; int svrport=sr; //���������صĴ���UDP�˿ں�IP
					if(!m_bCascade || peer.UdpAssociate(svrip,svrport,PROXY_MAX_RESPTIMEOUT)>0)
					{//���û�����ö����������������������UDP��������ɹ�
						ans1.Rep=0; ans1.Port=htons(sr); //�ɹ�
						ans1.IPAddr=psock->getLocalip();
						psock->Send(10 /*sizeof(sock5ans1)*/,(const char *)&ans1,-1);
						//��ȡUDP����ͻ���IP�Ͷ˿ڣ���clntIP,clntPort
						if(hostip) IPAddr=socketBase::Host2IP(hostip);
						if(IPAddr==INADDR_ANY || IPAddr==INADDR_NONE) IPAddr=psock->getRemoteip();
						if(m_bCascade){
							peer.setRemoteInfo(svrip.c_str(),svrport);
							sockUdp.setParent(&peer);
						}
						transData_UDP(psock,sockUdp,IPAddr,hostport); //��ʼUDP���ݴ���ת��
						break;
					}//?if(!m_bCascade ||
				}//?if(sr>0)
				ans1.Rep=1; //������Socks���� //ans1.Rep=7;//��ʱ�Ȳ�֧��
			}//?else if(psock5req1->Cmd ==3) //udp����
			else ans1.Rep=7;//��֧�ֵ�����
			psock->Send(10 /*sizeof(sock5ans1)*/,(const char *)&ans1,-1);
			if(ans1.Rep==0) //SOCKS5����ɹ�,����ת����
				transData(psock,&peer,NULL,0);
			else RW_LOG_PRINT(LOGLEVEL_WARN,"[ProxySvr] SOCKS5 failed - Cmd=%d, Atyp=%d, Rep=%d, host:port=%s:%d\r\n",
					psock5req1->Cmd,psock5req1->Atyp,ans1.Rep,((hostip)?hostip:peer.getRemoteIP()),hostport);
			break;//����ѭ��,��������
		}//?else if(istep==2)
		else break; //����Ĳ���
	}//?while

	if(ptr_proa) ptr_proa->m_loginusers--;
}
//yyc 2007-02-12 ����socks5 UDP����Ĵ���
//UDP����ת��
#define UDPPACKAGESIZE 8192
#define SOCKS5UDP_SIZE 10  //sizeof(sock5udp)==12
void transData_UDP(socketTCP *psock,socketUdp &sockUdp,unsigned long clntIP,int clntPort)
{
	if(psock==NULL) return;
	socketTCP *psvr=(socketTCP *)psock->parent();
	if(psvr==NULL) return;
	//�Ƿ�ָ���˶�������,peerΪ���Ӷ��������socket
	socketProxy *peer=(socketProxy *)sockUdp.parent();
	
	char *buffer=new char[SOCKS5UDP_SIZE+UDPPACKAGESIZE];
	if(buffer==NULL) return;
	
	//��ʼUDP���ݵĴ���ת��
	RW_LOG_DEBUG(0,"[ProxySvr] socks5-UDP has been started\r\n");
	if(peer) //ָ���˶�������
	{   //��ȡ����������UDP����˿�
		int casUdpPort=peer->getRemotePort();
		unsigned long casUdpIP=peer->getRemoteip();
		RW_LOG_DEBUG("[ProxySvr] socks5-UDP : Cascade UDP %s:%d\r\n",socketBase::IP2A(casUdpIP),casUdpPort);
		while(psvr->status()==SOCKS_LISTEN)
		{
			//checkSocket���ж����Ӷ�������������Ƿ��쳣
			int iret=sockUdp.checkSocket(SCHECKTIMEOUT,SOCKS_OP_READ);
			if(iret==0){
				if(psock->checkSocket(0,SOCKS_OP_READ)<0) break;
				else if(peer->checkSocket(0,SOCKS_OP_READ)<0) break;
				else continue;
			}
			if(iret<0) break; //��������
			//���ͻ��˷��͵�����
			iret=sockUdp.Receive(buffer,UDPPACKAGESIZE,-1);
			if(iret<=0){//�������󣬽�����ӡ������Ϣ
				RW_LOG_DEBUG("[ProxySvr] socks5-UDP : Failed to Receive,(%d - %d)\r\n",iret,sockUdp.getErrcode());
				continue; 
			}
			//����ǴӴ���ͻ��˹�����UDP������ת������������
			if(sockUdp.getRemoteip()==clntIP && sockUdp.getRemotePort()==clntPort)
				sockUdp.SetRemoteInfo(casUdpIP,casUdpPort);
			else //����ת��������ͻ���
				sockUdp.SetRemoteInfo(clntIP,clntPort);
			iret=sockUdp.Send(iret,buffer,-1);

//			RW_LOG_DEBUG("CasCade - len=%d data:\r\n:%s.\r\n",iret,buffer+sizeof(socks5udp));
		}//?while
	}
	else //û��ָ����������
	{
		char *buf=buffer+SOCKS5UDP_SIZE;
		socks5udp *pudp_pack=(socks5udp *)buffer;
		pudp_pack->Rsv=0; pudp_pack->Frag=0;
		pudp_pack->Atyp=0x01; //IPV4
		while(psvr->status()==SOCKS_LISTEN)
		{
			int iret=sockUdp.checkSocket(SCHECKTIMEOUT,SOCKS_OP_READ);
			if(iret==0){
				if(psock->checkSocket(0,SOCKS_OP_READ)<0) break;
				else continue;
			}
			if(iret<0){
				RW_LOG_DEBUG("[ProxySvr] socks5-UDP : checkSocket() return %d\r\n",iret);
				break; //��������
			}
			//���ͻ��˷��͵�����
			iret=sockUdp.Receive(buf,UDPPACKAGESIZE,-1);
			if(iret<=0){//�������󣬽�����ӡ������Ϣ
				RW_LOG_DEBUG("[ProxySvr] socks5-UDP : Failed to Receive,(%d - %d)\r\n",iret,sockUdp.getErrcode());
				continue; 
			}
			//����ǴӴ���ͻ��˹�����UDP������ת������������
			if(sockUdp.getRemoteip()==clntIP && sockUdp.getRemotePort()==clntPort)
			{//��UDP���ݰ��н�����ʵ�ʵ�IP��ַ�Ͷ˿�
				unsigned long IPAddr=INADDR_NONE; int hostport=0;
				socks5udp *pudp=(socks5udp *)buf; long udppackLen;
				if(pudp->Frag!=0) //�Ƕ���UDP���������Ƭ����
				{//��ʱ��ʵ����Ƭ���鹦�ܣ������˰�
					RW_LOG_DEBUG(0,"[ProxySvr] socks5-UDP : Received UDP frag,discard it\r\n");
					continue;
				}
				if(pudp->Atyp==1) //IP address
				{
					IPAddr =pudp->IPAddr;
					hostport=ntohs(pudp->Port);
					udppackLen=SOCKS5UDP_SIZE;
				}
				else if(pudp->Atyp==3) //domain name
				{
					char *other=(char *)&pudp->IPAddr;
					udppackLen=7+other[0]; //4+1+other[0]+2;
					hostport=ntohs(*((unsigned short *)(other+other[0]+1)));
					other[other[0]+1]='\0'; other++; //otherָ������
					IPAddr=socketBase::Host2IP(other);
				}else{
					RW_LOG_DEBUG(0,"[ProxySvr] socks5-UDP : Not surpport Address Type\r\n");
					break; //��֧�ֵĵ�ַ����
				}
				sockUdp.SetRemoteInfo(IPAddr,hostport);
				iret=sockUdp.Send(iret-udppackLen,buf+udppackLen,-1);
//				RW_LOG_DEBUG("Client->UDP(%s:%d) - len=%d data:\r\n%s.\r\n",
//					socketBase::IP2A(IPAddr),hostport,iret-udppackLen,buf+udppackLen);
			}else{ //�������UDP���Ȼ��ת��������ͻ���
				pudp_pack->IPAddr=sockUdp.getRemoteip();
				pudp_pack->Port=htons(sockUdp.getRemotePort());
				sockUdp.SetRemoteInfo(clntIP,clntPort);
				sockUdp.Send(iret+SOCKS5UDP_SIZE,buffer,-1);
//				RW_LOG_DEBUG("UDP->Client(%s:%d) - len=%d+%d data:\r\n%s.\r\n",
//					socketBase::IP2A(clntIP),clntPort,SOCKS5UDP_SIZE,iret,buf);
			}
		}//?while
	}

	sockUdp.Close(); delete[] buffer; 
	RW_LOG_DEBUG(0,"[ProxySvr] socks5-UDP has been ended\r\n");
	return;
}

