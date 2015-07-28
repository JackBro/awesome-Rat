/*******************************************************************
   *	smtpsvr.cpp
   *    DESCRIPTION:smtpЭ������ʵ��
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2005-11-20
   *
   *	net4cpp 2.1
   *	���ʼ��������(smtp)
   *******************************************************************/

#include "../include/sysconfig.h"
#include "../include/smtpsvr.h"
#include "../include/cCoder.h"
#include "../utils/cTime.h"
#include "../utils/utils.h"
#include "../include/cLogger.h"

using namespace std;
using namespace net4cpp21;


smtpServer :: smtpServer()
{
	m_strSvrname="smtp Server";
	m_authType=SMTPAUTH_NONE;
	m_helloTip="";
}

smtpServer :: ~smtpServer()
{
//������û���̣߳��������������������ǰ����
//	Close();
//	m_threadpool.join();
}


//����һ���µĿͻ����Ӵ˷��񴥷��˺���
void smtpServer :: onAccept(socketTCP *psock)
{
	RW_LOG_DEBUG("%s is connected\r\n",psock->getRemoteIP());
	char buf[SMTP_MAX_PACKAGE_SIZE]; int buflen=0;
	buflen=sprintf(buf,"220 SMTP Server[mailpost1.0] for ready\r\n");
	psock->Send(buflen,buf,-1);
//	ĳЩSMTP�ͻ��˲�֧�ֶ���SMTP��������Ӧ��
/*	buflen=sprintf(buf,"220-SMTP Server[mailpost1.0] for ready\r\n"
					   "220-copyright @yyc 2006. http://yycnet.yeah.net\r\n"
					   "%s",m_helloTip.c_str());
	psock->Send(buflen,buf,-1);
	//��ǰ����������״̬
	struct tm * ltime=localtime(&m_tmOpened);
	buflen=sprintf(buf,"220-��������ʼ���е�ʱ����%04d-%02d-%02d %02d:%02d:%02d\r\n",
				(1900+ltime->tm_year), ltime->tm_mon+1, ltime->tm_mday, 
				ltime->tm_hour, ltime->tm_min, ltime->tm_sec);
	psock->Send(buflen,buf,-1);
*/	
	cSmtpSession clientSession; 
	clientSession.m_tmLogin=time(NULL); //�û�����ʱ��
/*	ltime=localtime(&clientSession.m_tmLogin);
	buflen=sprintf(buf,"220-Ŀǰ���������ڵ�ʱ����%04d-%02d-%02d %02d:%02d:%02d\r\n"
					   "220-��ǰ��½�û����� %d \r\n"
					   "220-������������û��� %d \r\n"
					   "220 SMTP Server for ready...\r\n",
					   (1900+ltime->tm_year), ltime->tm_mon+1, ltime->tm_mday, 
					   ltime->tm_hour, ltime->tm_min, ltime->tm_sec,
					   curConnection(),maxConnection());
	psock->Send(buflen,buf,-1);
*/
	if(this->m_authType==SMTPAUTH_NONE) clientSession.m_bAccess=true;
	buflen=0; //���������ջ���
	while(psock->status()==SOCKS_CONNECTED )
	{
		int iret=psock->checkSocket(SCHECKTIMEOUT,SOCKS_OP_READ);
		if(iret<0) break; //�����¼��ʱ��ر�����
		if(!clientSession.m_bAccess && //�ж��Ƿ��¼��ʱ
			(time(NULL)-clientSession.m_tmLogin)>SMTP_MAX_RESPTIMEOUT) break;
		if(iret==0) continue;
		//���ͻ��˷��͵�����
		iret=psock->Receive(buf+buflen,SMTP_MAX_PACKAGE_SIZE-buflen-1,-1);
		if(iret<0) break; //==0������������������������
		if(iret==0){ cUtils::usleep(MAXRATIOTIMEOUT); continue; }
		buflen+=iret; buf[buflen]=0;
		//����smtp����
		const char *ptrCmd,*ptrBegin=buf;
		while( (ptrCmd=strchr(ptrBegin,'\r')) )
		{
			*(char *)ptrCmd=0;//��ʼ��������
			if(ptrBegin[0]==0) goto NextCMD; //�������������
		
			parseCommand(clientSession,psock,ptrBegin);

NextCMD:	//�ƶ�ptrBegin����һ������������ʼ
			ptrBegin=ptrCmd+1; 
			while(*ptrBegin=='\r' || *ptrBegin=='\n') ptrBegin++; //����\r\n
		}//?while
		//�����δ��������������ƶ�
		if((iret=(ptrBegin-buf))>0 && (buflen-iret)>0)
		{//���ptrBegin-buf==0˵������һ�������������ݰ�
			buflen-=iret;
			memmove((void *)buf,ptrBegin,buflen);
		} else buflen=0;
	}//?while

	RW_LOG_DEBUG("%s is disconnected\r\n",psock->getRemoteIP());
	RW_LOG_FFLUSH()
	return;
}

//�����ǰ���������ڵ�ǰ�趨������������򴥷����¼�
void smtpServer :: onTooMany(socketTCP *psock)
{
	char resp[]="220 access denied, Too many users.\r\n";
	psock->Send(sizeof(resp)-1,resp,-1);
	return;
}

//-------------------SMTP ����������� begin-----------------------------------
void smtpServer :: parseCommand(cSmtpSession &clientSession,socketTCP *psock,const char *ptrCommand)
{
	RW_LOG_DEBUG("[smtpsvr] c--->s:\r\n\t%s\r\n",ptrCommand);
	if(strncasecmp(ptrCommand,"EHLO ",5)==0)
		docmd_ehlo(clientSession,psock,ptrCommand+5);
	else if(strncasecmp(ptrCommand,"HELO ",5)==0)
		docmd_ehlo(clientSession,psock,ptrCommand+5);
	else if(strncasecmp(ptrCommand,"AUTH ",5)==0)
		docmd_auth(clientSession,psock,ptrCommand+5);
	else if(strncasecmp(ptrCommand,"MAIL FROM:",9)==0)
		docmd_mailfrom(clientSession,psock,ptrCommand+9);
	else if(strncasecmp(ptrCommand,"RCPT TO:",7)==0)
		docmd_rcptto(clientSession,psock,ptrCommand+7);
	else if(strcasecmp(ptrCommand,"Data")==0)
		docmd_data(clientSession,psock);
	else if(strcasecmp(ptrCommand,"quit")==0)
		docmd_quit(psock);
	else if(strcasecmp(ptrCommand,"rset")==0)
		resp_OK(psock);
	else 
		resp_unknowed(psock);
	return;
}

inline SOCKSRESULT response(socketTCP *psock,const char *buf,int buflen)
{
	RW_LOG_DEBUG("[smtpsvr] s--->c:\r\n\t%s",buf);
	return psock->Send(buflen,buf,-1);
}

inline void smtpServer :: resp_OK(socketTCP *psock)
{
	char resp[]="250 OK\r\n";
	response(psock,resp,sizeof(resp)-1);
	return;
}
inline void smtpServer :: resp_unknowed(socketTCP *psock)
{
	char resp[]="500 command unrecognized.\r\n";
	response(psock,resp,sizeof(resp)-1);
	return;
}
//ע��smtp����Ķ������ݵ���Ӧ������������һ������Ӧ�е���Ӧ�������֮��Ҫ��-����
void smtpServer :: docmd_ehlo(cSmtpSession &clientSession,socketTCP *psock,const char *strParam)
{
	//ȥ�����������ǰ���ո�
	while(*strParam==' ') strParam++;
	clientSession.m_ehlo.assign(strParam);
	const char resp[]="250-PIPELINING\r\n"
					  "250-AUTH LOGIN\r\n"
					  "250-AUTH=LOGIN\r\n"
					  "250-8BITMIME\r\n"
					  "250 OK\r\n";
	response(psock,resp,sizeof(resp)-1);
	return;
}

void smtpServer :: docmd_auth(cSmtpSession &clientSession,socketTCP *psock,const char *strParam)
{
	//ȥ�����������ǰ���ո�
	while(*strParam==' ') strParam++;
	int authType=SMTPAUTH_NONE;
	if(strcasecmp(strParam,"LOGIN")==0)
	{
		authType=SMTPAUTH_LOGIN;
		const char resp[]="334 VXNlcm5hbWU6\r\n"; //VXNlcm5hbWU6ΪUsername:��base64����
		response(psock,resp,sizeof(resp)-1);
	}
	else if(strcasecmp(strParam,"PLAIN")==0) //���Ĵ���
	{
		authType=SMTPAUTH_PLAIN;
		const char resp[]="334 Username:\r\n";
		response(psock,resp,sizeof(resp)-1);
	}
	else if(strcasecmp(strParam,"8BITMIME")==0)
	{
		authType=SMTPAUTH_8BITMIME;
		const char resp[]="334 Username%3A\r\n"; //Username%3AΪUsername:��Mime����
		response(psock,resp,sizeof(resp)-1);
	}
	else
	{
		const char resp[]="504-�����������ʵ��\r\n"
						  "504 only Surpport LOGIN,8BITMIME\r\n";
		response(psock,resp,sizeof(resp)-1);
		psock->Close(); return;
	}
	//�ȴ��û�������֤�ʺź�����
	std::string strAccount,strPwd;
	char buf[SMTP_MAX_PACKAGE_SIZE];
	int iret=psock->Receive(buf,SMTP_MAX_PACKAGE_SIZE-1,SMTP_MAX_RESPTIMEOUT);
	if(iret<=0){ psock->Close(); return; }
	buf[iret]=0; strAccount.assign(buf);
	if(authType==SMTPAUTH_LOGIN)
	{//����Base64����
		iret=cCoder::base64_decode((char *)strAccount.c_str(),strAccount.length(),buf);
		if(iret>=0) buf[iret]=0; 
		strAccount.assign(buf);
		const char resp[]="334 UGFzc3dvcmQ6\r\n";
		response(psock,resp,sizeof(resp)-1);
	}
	else if(authType==SMTPAUTH_8BITMIME)
	{
		iret=cCoder::mime_decode(strAccount.c_str(),strAccount.length(),buf);
		if(iret>=0) buf[iret]=0; 
		strAccount.assign(buf);
		const char resp[]="334 Password%3A\r\n";
		response(psock,resp,sizeof(resp)-1);
	}
	else
	{
		const char resp[]="334 Password:\r\n";
		response(psock,resp,sizeof(resp)-1);
	}
	//��ȡ����
	iret=psock->Receive(buf,SMTP_MAX_PACKAGE_SIZE-1,SMTP_MAX_RESPTIMEOUT);
	if(iret<=0){ psock->Close(); return; }
	buf[iret]=0; strPwd.assign(buf);
	if(authType==SMTPAUTH_LOGIN)
	{//����Base64����
		iret=cCoder::base64_decode((char *)strPwd.c_str(),strPwd.length(),buf);
		if(iret>=0) buf[iret]=0; 
		strPwd.assign(buf);	
	}
	else if(authType==SMTPAUTH_8BITMIME)
	{
		iret=cCoder::mime_decode(strPwd.c_str(),strPwd.length(),buf);
		if(iret>=0) buf[iret]=0; 
		strPwd.assign(buf);	
	}
	
	//��֤�ʺź�����
	if(!onAccess(strAccount.c_str(),strPwd.c_str()))
	{
		const char resp[]="551 Authentication unsuccessful\r\n";
		response(psock,resp,sizeof(resp)-1);
		psock->Close(); //��֤ʧ��
	}
	else
	{
		clientSession.m_bAccess=true; //��֤�ɹ�
		const char resp[]="235 Authentication successful\r\n";
		response(psock,resp,sizeof(resp)-1);
	}
	return;
}

void smtpServer :: docmd_mailfrom(cSmtpSession &clientSession,socketTCP *psock,const char *strParam)
{
	//ȥ�����������ǰ���ո�
	while(*strParam==' ') strParam++;
	const char *ptrS=strchr(strParam,'<');
	if(ptrS)
	{	ptrS++;
		const char *ptrE=strchr(ptrS,'>');
		if(ptrE) *(char *)ptrE=0;
		clientSession.m_fromemail.assign(ptrS);
	}
	else
		clientSession.m_fromemail.assign(strParam);
	resp_OK(psock);
	return;
}

void smtpServer :: docmd_rcptto(cSmtpSession &clientSession,socketTCP *psock,const char *strParam)
{
	//ȥ�����������ǰ���ո�
	while(*strParam==' ') strParam++;
	const char *ptrS=strchr(strParam,'<');
	if(ptrS)
	{	ptrS++;
		const char *ptrE=strchr(ptrS,'>');
		if(ptrE) *(char *)ptrE=0;
		clientSession.m_recp.push_back(ptrS);
	}
	else
		clientSession.m_recp.push_back(strParam);
	resp_OK(psock);
	return;
}

void smtpServer :: docmd_data(cSmtpSession &clientSession,socketTCP *psock)
{

	const char resp[]="354 End data with <CR><LF>.<CR><LF>\r\n";
	response(psock,resp,sizeof(resp)-1);
	//���濪ʼ������������,ֱ���յ�<CR><LF>.<CR><LF>
	char buf[4096]; int buflen=0;
	std::string emlfile;//������ʱ�ļ���
	time_t tNow=time(NULL);
	srand( (unsigned)clock() );
	struct tm * ltime=localtime(&tNow);
	buflen=sprintf(buf,"%s%04d%02d%02d%02d%02d%02d_%d.!em",m_receivedpath.c_str(),
		(1900+ltime->tm_year), ltime->tm_mon+1, ltime->tm_mday, 
		ltime->tm_hour, ltime->tm_min, ltime->tm_sec,rand());
	emlfile.assign(buf);
	FILE *fp=::fopen(emlfile.c_str(),"wb");
	if(fp==NULL){psock->Close(); return; }
	::fprintf(fp,"Email body is base64 encoded\r\nFROM: %s\r\nTO: ",clientSession.m_fromemail.c_str());
	for(int i=0;i<clientSession.m_recp.size();i++)
		::fprintf(fp,"%s,",clientSession.m_recp[i].c_str());
	::fprintf(fp,"\r\n\r\n");
	cTime t=cTime::GetCurrentTime();
	int tmlen=t.Format(buf,256,"%a, %d %b %Y %H:%M:%S %Z"); buf[tmlen]=0;
	::fprintf(fp,"Received: from %s [%s]\r\n\tby mailpost1.0 with SMTP\r\n"
				 "\t%s\r\n",clientSession.m_ehlo.c_str(),psock->getRemoteIP(),
				 buf);

	bool bRecvALL=false; //�����Ƿ���������
	while( psock->status()==SOCKS_CONNECTED )
	{
		//���ͻ��˷��͵�����
		//�������SMTP_MAX_RESPTIMEOUT��û�յ����ݿ���Ϊ�ͻ����쳣
		buflen=psock->Receive(buf,4095,SMTP_MAX_RESPTIMEOUT);
		if(buflen<0){
			RW_LOG_PRINT(LOGLEVEL_WARN,"[smtpsvr] Failed to receive mail DATA,error=%d\r\n",buflen);
			break; 
		}
		if(buflen==0){ cUtils::usleep(SCHECKTIMEOUT); continue; }//==0������������������������
		buf[buflen]=0;
		if(buflen>=5 && strcmp(buf+buflen-5,"\r\n.\r\n")==0)
		{ 
			if( (buflen-=5)>0 ) ::fwrite(buf,sizeof(char),buflen,fp);
			bRecvALL=true; break;
		}
		else
			::fwrite(buf,sizeof(char),buflen,fp);
	}//?while
	::fclose(fp);
	if(bRecvALL)
	{
		buflen=sprintf(buf,"%s",emlfile.c_str());
		//����չ����Ϊeml
		emlfile[buflen-3]='e';emlfile[buflen-2]='m';
		emlfile[buflen-1]='l';emlfile[buflen]=0;
		FILEIO::fileio_rename(buf,emlfile.c_str());
		const char resp[]="250 Ok: Success to receive Data.\r\n";
		response(psock,resp,sizeof(resp)-1);
		onReceive(emlfile.c_str(),clientSession);
	}
	else
	{
		const char resp[]="451 Error: Failed to receive Data.\r\n";
		response(psock,resp,sizeof(resp)-1);
		FILEIO::fileio_deleteFile(emlfile.c_str());
	}
	return;
}

inline void smtpServer :: docmd_quit(socketTCP *psock)
{
	const char resp[]="221 Bye\r\n";
	response(psock,resp,sizeof(resp)-1);
	psock->Close(); return;
}

