/*******************************************************************
   *	vIDCs.cpp
   *    DESCRIPTION:vIDC�������ʵ��
   *
   *    AUTHOR:yyc
   *	http://hi.baidu.com/yycblog/home
   *
   *******************************************************************/

#include "../../net4cpp21/include/sysconfig.h"
#include "../../net4cpp21/utils/utils.h"
#include "../../net4cpp21/include/cLogger.h"
#include "vidcs.h"

using namespace std;
using namespace net4cpp21;

vidcsvr :: vidcsvr()
{
	m_bAuthentication=false;
}

vidcsvr :: ~vidcsvr() { Destroy(); }

void vidcsvr :: Destroy()
{
	std::map<long,vidccSession *>::iterator it=m_sessions.begin();
	for(;it!=m_sessions.end();it++)
	{
		vidccSession *pvidcc=(*it).second;
		if(pvidcc!=NULL) pvidcc->Destroy();
		delete pvidcc;
	}
	m_sessions.clear();
}

bool vidcsvr :: DisConnect(long vidccID) //ǿ�ƶϿ�ĳ��vidccc������
{
	std::map<long,vidccSession *>::iterator it=m_sessions.find(vidccID);
	if(it==m_sessions.end()) return false;
	vidccSession *pvidcc=(*it).second;
	//�ر�socket������ onConnect �л��Զ�ɾ����vidccSession
	if(pvidcc) pvidcc->Close(); else m_sessions.erase(it); 
	return true;
}
//���ö�ĳ��vidccӳ��ķ��������־��¼
void vidcsvr :: setLogdatafile(long vidccID,bool b)
{
	std::map<long,vidccSession *>::iterator it=m_sessions.find(vidccID);
	if(it==m_sessions.end()) return;
	vidccSession *pvidcc=(*it).second;
	pvidcc->setIfLogdata(b); 
	return;
}

void vidcsvr :: xml_list_vidcc(cBuffer &buffer)
{
	if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
	if(buffer.str()==NULL) return;
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<vidccs>");
	
	std::map<long,vidccSession *>::iterator it=m_sessions.begin();
	for(;it!=m_sessions.end();it++)
	{
		if(buffer.Space()<128) buffer.Resize(buffer.size()+128);
		if(buffer.str()==NULL) return;
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<vidcc>");
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<vID>%d</vID>",(*it).first);
		vidccSession *pvidcc=(*it).second;
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<vname>%s</vname>",pvidcc->vidccName());
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<vip>%s</vip>",pvidcc->vidccIP());
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"</vidcc>");
	}
		
	if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
	if(buffer.str())
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"</vidccs>");
	return;
}

void vidcsvr :: xml_info_vidcc(cBuffer &buffer,long vidccID)
{
	std::map<long,vidccSession *>::iterator it=m_sessions.find(vidccID);
	if(it==m_sessions.end())
	{
		if(buffer.Space()<48) buffer.Resize(buffer.size()+48);
		if(buffer.str())
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<retmsg>inValid vidccID</retmsg>");
		return;
	}
	vidccSession *pvidcc=(*it).second;
	if(pvidcc==NULL) return;
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<vidcc_status>");
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<vID>%d</vID>",vidccID);
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<vname>%s</vname>",pvidcc->vidccName());
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<vip>%s</vip>",pvidcc->vidccIP());
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<vdesc>%s</vdesc>",pvidcc->vidccDesc());
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<ver>%.1f</ver>",pvidcc->vidccVer()/10.0);
	time_t t=pvidcc->ConnectTime(); struct tm *ltime=localtime(&t);
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<starttime>%04d��%02d��%02d�� %02d:%02d:%02d</starttime>",
			(1900+ltime->tm_year), ltime->tm_mon+1, ltime->tm_mday,ltime->tm_hour, ltime->tm_min, ltime->tm_sec);
	pvidcc->xml_list_mapped(buffer);
	if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
	if(buffer.str())
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"</vidcc_status>");
	return;
}

//��һ���µ����ӽ���
void vidcsvr::onConnect(socketTCP *psock)
{
	socketBase *psocketsvr=psock->parent(); 
	if(psocketsvr==NULL) return; //����vidcs�����socket����
	char buf[VIDC_MAX_COMMAND_SIZE];
	int iret=psock->Receive(buf,VIDC_MAX_COMMAND_SIZE-1,VIDC_MAX_RESPTIMEOUT);
	//����д������ָ����ʱ����û���յ��κ������򷵻ز��Ͽ�����
	if(iret<=0) return; else buf[iret]=0;
	
	RW_LOG_DEBUG("[vidcSvr] c--->s:\r\n\t%s\r\n",buf);
	if(strncmp(buf,"HELO ",5)==0)
	{//ĳ���ͻ������ӵ�½
		//�����µ�vidccSession����ӵ�m_sessions������
		vidccSession *pvidcc=docmd_helo(psock,buf+5);
		if(pvidcc==NULL) return; //��½ʧ�ܹر�����
		char buf[VIDC_MAX_COMMAND_SIZE]; int buflen=0;//���������ѭ��
		time_t tLastReceived=time(NULL);
		while(psocketsvr->status()==SOCKS_LISTEN && 
			  psock->status()==SOCKS_CONNECTED )
		{
			iret=psock->checkSocket(SCHECKTIMEOUT,SOCKS_OP_READ);
			if(iret<0) break;
			if(iret==0)
				if((time(NULL)-tLastReceived)>VIDC_MAX_CLIENT_TIMEOUT) break; else continue;
			//���ͻ��˷��͵�����
			iret=psock->Receive(buf+buflen,VIDC_MAX_COMMAND_SIZE-buflen-1,-1);
			if(iret<0) break; //==0������������������������
			if(iret==0){ cUtils::usleep(MAXRATIOTIMEOUT); continue; }
			tLastReceived=time(NULL); //���һ�ν��յ�����ʱ��
			buflen+=iret; buf[buflen]=0; //����vIDC����
			const char *ptrCmd,*ptrBegin=buf;
			while( (ptrCmd=strchr(ptrBegin,'\r')) )
			{
				*(char *)ptrCmd=0;//��ʼ��������
				if(ptrBegin[0]==0) goto NextCMD; //�������������
				if(strncmp(ptrBegin,"SSLC ",5)==0) //����ĳ��ӳ�����Ŀͻ�����֤֤����Ϣ
				{//��ʽ: SSLC name=<XXX> certlen=<֤���ֽ�> keylen=<˽Կ�ֽ�> pwdlen=<���볤��>\r\n�����ֽ�\r\n
					const char *ptrData=ptrCmd+1;
					if(*ptrData=='\r' || *ptrData=='\n') ptrData++; //����\r\n
					long dataLen=buflen-(ptrData-buf); //�õ����ջ������л�δ�������������
					//docmd_sslc���شӽ��ջ�������ȡ����������ݸ���
					dataLen=pvidcc->docmd_sslc(ptrBegin+5,ptrData,dataLen);
					ptrBegin=ptrData+dataLen; continue; //��������Ĵ���
				}else pvidcc->parseCommand(ptrBegin);

NextCMD:		//�ƶ�ptrBegin����һ������������ʼ
				ptrBegin=ptrCmd+1; 
				while(*ptrBegin=='\r' || *ptrBegin=='\n') ptrBegin++; //����\r\n
			}//?while( (ptrCmd=
			//�����δ��������������ƶ�
			if((iret=(ptrBegin-buf))>0 && (buflen-iret)>0)
			{//���ptrBegin-buf==0˵������һ�������������ݰ�
				buflen-=iret;
				memmove((void *)buf,ptrBegin,buflen);
			} else buflen=0;
		}//?while...
		//��m_sessions������ɾ��vidccSession
		m_mutex.lock();
		std::map<long,vidccSession *>::iterator it=m_sessions.find((long)pvidcc);
		if(it!=m_sessions.end()) m_sessions.erase(it);
		m_mutex.unlock(); 
		pvidcc->Destroy(); delete pvidcc; //Destroy��ȴ����еĹܵ�����
	}//?if(strncmp(buf,"HELO ",5)==0)
	else if(strncmp(buf,"PIPE ",5)==0)
	{//��������ĳ��vIDCc�Ĺܵ�����
		//����һ���ܵ���������onConnect�˳����ͷ�psock����Ӱ�쵽ת���߳�
		socketTCP *pipe=new socketTCP(*psock);
		//���ÿյĸ������Ա�����ȴ��ܵ���
		if(pipe==NULL) return; else pipe->setParent(NULL);
		long vidccID=atol(buf+5);
		vidccSession *pvidcc=AddPipeFromVidcSession(pipe,vidccID);
		if(pvidcc==NULL){ delete pipe; return; }
		
		time_t t=time(NULL);
		while(psocketsvr->status()==SOCKS_LISTEN && 
		//	  pvidcc->isConnected() && //Ҳ��pvidcc�Ѿ����ͷ�ɾ����
			  pipe->status()==SOCKS_CONNECTED )
		{//����ܵ��󶨳ɹ�����pipe��parentָ��󶨵ĶԶ�
			if(pipe->parent()!=NULL) return; //�Ѿ�����ת��socket
			//ָ����ʱ���ڻ�û�б��������
			else if((time(NULL)-t)>VIDC_PIPE_ALIVETIME) break; 
			cUtils::usleep(200000); //�ӳ�200ms
		}//?while
		if(DelPipeFromVidcSession(pipe,vidccID)) delete pipe;
		//����˹ܵ��Ѿ���ȡ��ռ��,��ɾ��
//		else{ pipe->Close(); pipe->setParent(NULL); }
	}//?else if(strncmp(ptrBegin,"PIPE ",5)==0)
	//yyc add 2007-08-21 surpport MakeHole�������TCP����
	else if(strcmp(buf,"HOLE\r\n")==0)
	{
		//�������ӵ�IP��ַ�Ͷ˿�
		iret=sprintf(buf,"200 %s:%d\r\n",psock->getRemoteIP(),psock->getRemotePort());
		iret=psock->Send(iret,buf,-1);
		//ѭ���ȴ��Է��ͷ�����
		while(psocketsvr->status()==SOCKS_LISTEN && 
			  psock->status()==SOCKS_CONNECTED )
		{
			iret=psock->checkSocket(SCHECKTIMEOUT,SOCKS_OP_READ);
			if(iret<0) break;
			if(iret==0) continue;
			//����Է��º͹ر���������checkSocket���==1,��������::recv�᷵��0
			iret=psock->Receive(buf,VIDC_MAX_COMMAND_SIZE-1,-1);
			if(iret<0) break;
		}//?while
	}//?else if(strcmp(buf,"HOLE\r\n")==0)
	//һ���µ����ӹ�������ʼֻ�п��ܷ���HELO��PIPE����,��������������ر�����
}
//�ͻ��˵�¼��֤
//	��ʽ:HELO <SP> <NAME:pswd> <SP> VER <SP> DESC
vidccSession * vidcsvr :: docmd_helo(socketTCP *psock,const char *param)
{
	int len,clntVer=0xff; char buf[64];
	string strUser,strPswd,strDesc;
	const char *ptr1,*ptr=strchr(param,' ');
	if(ptr){ //��ȡName������
		*(char *)ptr=0;
		if( (ptr1=strchr(param,':')) )
		{
			strPswd.assign(ptr1+1);
			strUser.assign(param,ptr1-param);
		}else strUser.assign(param);
		*(char *)ptr=' '; param=ptr+1;
		//��ȡ�汾��
		if( (ptr=strchr(param,' ')) )
		{
			strDesc.assign(ptr+1);
			clntVer=atoi(param);
		}else clntVer=atoi(param);
	}//?if(ptr)
	if(clntVer<VIDCC_MIN_VERSION){
		len=sprintf(buf,msg_err_405,VIDCS_VERSION);
		psock->Send(len,buf,-1); return NULL;
	}
	if(m_bAuthentication && m_strPswd!=strPswd){
		len=sprintf(buf,msg_err_221,VIDCS_VERSION);
		psock->Send(len,buf,-1); return NULL;
	}
	vidccSession *pvidcc=new vidccSession(psock,clntVer,strUser.c_str(),strDesc.c_str());
	if(pvidcc==NULL) return NULL;
	m_mutex.lock();
	m_sessions[(long)pvidcc]=pvidcc;
	m_mutex.unlock();
	len=sprintf(buf,"200 %d %d command success.\r\n",(long)pvidcc,VIDCS_VERSION);
	psock->Send(len,buf,-1); return pvidcc;
}
//vIDC�ͻ����½�����һ���ܵ�
//	��ʽ:PIPE <SP> CLNTID <CRLF>
vidccSession * vidcsvr :: AddPipeFromVidcSession(socketTCP *pipe,long vidccID)
{
	vidccSession *pvidcc=NULL;
	m_mutex.lock();
	std::map<long,vidccSession *>::iterator it=m_sessions.find(vidccID);
	if(it!=m_sessions.end())
	{
		pvidcc=(*it).second;
		if(!pvidcc->AddPipe(pipe)) pvidcc=NULL;
//		RW_LOG_DEBUG("[vidcsvr] new pipe from vidccID %d\r\n",vidccID);
	}
	m_mutex.unlock(); return pvidcc;
}
bool vidcsvr :: DelPipeFromVidcSession(socketTCP *pipe,long vidccID)
{
	bool bret=false; m_mutex.lock();
	std::map<long,vidccSession *>::iterator it=m_sessions.find(vidccID);
	if(it!=m_sessions.end())
	{
		vidccSession *pvidcc=(*it).second;
		if(pvidcc) bret=pvidcc->DelPipe(pipe);
	}
	m_mutex.unlock(); return bret;
}
//------------------------------------------------------------
vidcServer :: vidcServer()
{
	m_strSvrname="vIDC Server";
}
void vidcServer :: Stop()
{
	Close();
	vidcsvr::Destroy();
	m_threadpool.join();
}


