/*******************************************************************
   *	msnlib.cpp
   *    DESCRIPTION:msnЭ�鴦����ʵ��
   *
   *    AUTHOR:yyc
   *
   *    HISTORY:
   *
   *    DATE:2005-06-28
   *	
   *******************************************************************/

#include "../../include/sysconfig.h"
#include "../../include/cCoder.h"
#include "../../utils/utils.h"
#include "../../include/cLogger.h"
#include "msnlib.h"

using namespace std;
using namespace net4cpp21;

//�����ͻ��˺�NS�������������
void msnMessager :: receiveThread(msnMessager *pmsnmessager) 
{
	if(pmsnmessager==NULL) return;
	RW_LOG_PRINT(LOGLEVEL_WARN,0,"main-thread of msnMessager has been started.\r\n");
	socketTCP *psock=(socketTCP *)&pmsnmessager->m_curAccount.m_chatSock;

	time_t tStart=time(NULL);
	char buffer[MSN_MAX_NS_COMMAND_SIZE]; //���տͻ�������
	int buflen=0; //���������ջ���
	while( psock->status()==SOCKS_CONNECTED )
	{
		int iret=psock->checkSocket(SCHECKTIMEOUT,SOCKS_OP_READ);
		if(iret<0) break; 
		if(iret==0){
			if( (time(NULL)-tStart)>=20/*MSN_MAX_RESPTIMEOUT*/ ){ 
				psock->Send(5,"PNG\r\n",-1); 
				tStart=time(NULL); 
			}
			continue; //û������
		}//?if(iret==0)
		//���ͻ��˷��͵�����
		iret=psock->Receive(buffer+buflen,MSN_MAX_NS_COMMAND_SIZE-buflen-1,-1);
		if(iret<0) break; //==0������������������������
		if(iret==0){ cUtils::usleep(MAXRATIOTIMEOUT); continue; }
		buflen+=iret; buffer[buflen]=0;
		//����msn����
		unsigned long trID=0; //NS������ӦID
		char *tmpptr,*ptrCmd,*ptrBegin=buffer;
		while( (ptrCmd=strchr(ptrBegin,'\r')) )
		{
			*(char *)ptrCmd=0;//��ʼ��������
			if(ptrBegin[0]==0) goto NextCMD; //�������������

//����MSN NS���������� ---- begin-------------------------------------------
			//ע��:�յ����������Ƚ���utf8����Ȼ�����mime�����
//			RW_LOG_DEBUG("[msnlib] recevied Command from NS: %s.\r\n",ptrBegin);
			if( strncmp(ptrBegin,"MSG ",4)==0 || 
				strncmp(ptrBegin,"GCF ",4)==0 ||
				strncmp(ptrBegin,"UBX ",4)==0 )
			{//���������Ϣ,��ʽ����
				//MSG Hotmail Hotmail 514  ����514ָ����Ϣ�ĺ��������յ��ֽ���
				//GCF 9 Shields.xml 159 ����159ָ����Ϣ�ĺ��������յ��ֽ���(ΪShields.xml�ļ�����)
				//UBX idazhi@hotmail.com 60 ����60ָ����Ϣ�ĺ��������յ��ֽ���
				if( (tmpptr=strrchr(ptrBegin,' ')) )
				{//��ȡ��Ϣ���� len
					int len=atoi(tmpptr+1); 
					//�ж�����Ľ���������\r����\r\n
					int nSkip=(*(ptrCmd+1)=='\n')?2:1;
					if( (buflen-(ptrCmd-buffer+nSkip))>=len) 
					{ //��Ϣ�Ѿ�ȫ���������
						char c=*(ptrCmd+len+nSkip); 
						*(ptrCmd+len+nSkip)=0;//����Ҫ������ַ����н�������

						//������Ϣ //+nSkip����\r\n
						if(ptrBegin[0]=='M')
							pmsnmessager->nscmd_msg(psock,ptrCmd+nSkip);
						else if(ptrBegin[0]=='U')
						{
							*(char *)tmpptr=0; //��ȡemail��Ϣ
							pmsnmessager->nscmd_ubx(psock,ptrBegin+4,ptrCmd+nSkip);
						}
//						else if(ptrBegin[0]=='G')
//							pmsnmessager->nscmd_gcf(psock,ptrCmd+nSkip); //������GCF��Ϣ

						*(ptrCmd+len+nSkip)=c;
						//����������Ϣ,ע��whileѭ����ptrBegin=ptrCmd+1;���Ҫ-1
						ptrCmd+=(len+nSkip-1);
					}
					else{ *ptrCmd='\r'; break; }//��������ʣ�����Ϣ
				}//?if( (tmpptr=strrchr(ptrBegin,' ')) )
			}//?strncmp(ptrBegin,"MSG ",4)
			else if( atol(ptrBegin)!=0 ) 
			{//ĳ������Ĵ��󷵻� ��ʽ: ������ trID ˵��\r\n
				if( (tmpptr=strchr(ptrBegin,' ')) ) 
					trID=(unsigned long)atol(tmpptr+1);
			}//?if( atol(ptrBegin)!=0 )
			else if(strncmp(ptrBegin,"SBS ",4)==0)
				pmsnmessager->nscmd_sbs(psock,ptrBegin);
			else if(strncmp(ptrBegin,"XFR ",4)==0)
				trID=(unsigned long)atol(ptrBegin+4);
			else if(strncmp(ptrBegin,"SYN ",4)==0)
				pmsnmessager->nscmd_syn(psock,ptrBegin);
			else if(strncmp(ptrBegin,"LSG ",4)==0)
				pmsnmessager->nscmd_lsg(psock,ptrBegin);
			else if(strncmp(ptrBegin,"LST ",4)==0)
				pmsnmessager->nscmd_lst(psock,ptrBegin);
			else if(strncmp(ptrBegin,"CHL ",4)==0)
				pmsnmessager->nscmd_chl(psock,ptrBegin);
			else if(strncmp(ptrBegin,"ILN ",4)==0)
				pmsnmessager->nscmd_iln(psock,ptrBegin);
			else if(strncmp(ptrBegin,"FLN ",4)==0)
				pmsnmessager->nscmd_fln(psock,ptrBegin);
			else if(strncmp(ptrBegin,"NLN ",4)==0)
				pmsnmessager->nscmd_nln(psock,ptrBegin);
			else if(strncmp(ptrBegin,"REM ",4)==0)
				trID=pmsnmessager->nscmd_rem(psock,ptrBegin);
			else if(strncmp(ptrBegin,"ADC ",4)==0)
				trID=pmsnmessager->nscmd_adc(psock,ptrBegin);
			else if(strncmp(ptrBegin,"RNG ",4)==0)
				trID=pmsnmessager->nscmd_rng(psock,ptrBegin);
			else if(strncmp(ptrBegin,"PRP ",4)==0)
				trID=pmsnmessager->nscmd_prp(psock,ptrBegin);
			else
				RW_LOG_DEBUG("[msnlib] recevied Command from NS: %s.\r\n",ptrBegin);

			if(trID!=0){
				pmsnmessager->m_mutex.lock();
				std::map<unsigned long,cCond *>::iterator it=pmsnmessager->m_conds.find(trID);
				if(it!=pmsnmessager->m_conds.end()){ 
					char *respbuf=(char *)((*it).second->getArgs());
					if(respbuf) strcpy(respbuf,ptrBegin);
					(*it).second->active(); 
				}
				pmsnmessager->m_mutex.unlock(); trID=0; 
			}//?if(trID!=0)
//����MSN NS���������� ----  end -------------------------------------------
			

NextCMD:	//�ƶ�ptrBegin����һ������������ʼ
			ptrBegin=ptrCmd+1; 
			while(*ptrBegin=='\r' || *ptrBegin=='\n') ptrBegin++; //����\r\n
		}//?while
		//�����δ��������������ƶ�
		if((iret=(ptrBegin-buffer))>0 && (buflen-iret)>0)
		{//���ptrBegin-buf==0˵������һ�������������ݰ�
			buflen-=iret;
			memmove((void *)buffer,ptrBegin,buflen);
		} else buflen=0;
	}//?while
	
	psock->Close(); pmsnmessager->onSIGNOUT();
	RW_LOG_PRINT(LOGLEVEL_WARN,0,"main-thread of msnMessager has been ended\r\n");
	return;

}

//�����ͻ��˺�SS�������������
//ÿ��sessionThread��һ��chat session
void msnMessager :: sessionThread(void *param) 
{
	std::pair<msnMessager *,cContactor *> *pp=(std::pair<msnMessager *,cContactor *> *)param;
	msnMessager *pmsnmessager=pp->first;
	cContactor *pcon=pp->second; delete pp;
	if(pmsnmessager==NULL || pcon==NULL) return;
	srand(clock()); //��֤���̵߳ĵ�rand�����������ÿ���̶߳�����Ĭ�ϵ����Ӳ��������������ÿ��
	//������̲߳������������ͬ 2005-07-18 yyc comment
	socketProxy *pchatsock=&pcon->m_chatSock;
	RW_LOG_PRINT(LOGLEVEL_WARN,"session-thread(%s) of msnMessager has been started.\r\n",
		pcon->m_email.c_str());
	pmsnmessager->onChatSession((HCHATSESSION)pcon,MSN_CHATSESSION_CREATE,
		pcon->m_email.c_str(),0);
	
	pcon->m_chat_contacts=0;//����˴ν�̸�ĻỰ����ϵ�˸���
	time_t tStart=time(NULL);//�Ự��ʼʱ��
	time_t tTimeout=60;//10*MSN_MAX_RESPTIMEOUT;
	char buffer[MSN_MAX_SS_COMMAND_SIZE]; //���տͻ�������
	int buflen=0;//������������ĳ���
	while(pchatsock->status()==SOCKS_CONNECTED)
	{
		int iret=pchatsock->checkSocket(SCHECKTIMEOUT,SOCKS_OP_READ);
		if(iret<0) break; //��socket��������
		if(iret==0){
			if(pcon->m_chat_contacts>0 && pcon->m_vecMessage.size()>0) 
			{//yyc add 2006-03-13
				int iSize=pcon->m_vecMessage.size();
				for(int i=0;i<iSize;i++)
				{
					std::pair<char *,long> &p=pcon->m_vecMessage[i];
					pchatsock->Send(p.second,p.first,-1);
					delete[] p.first;
				}
				pcon->m_vecMessage.clear();
			}//yyc add 2006-03-13 end
			continue; //û������
		}
		//�����ݵ���,��������
		iret=pchatsock->Receive(buffer+buflen,MSN_MAX_SS_COMMAND_SIZE-buflen-1,-1);
		if(iret<0)  break;  //==0������������������������
		if(iret==0){ cUtils::usleep(MAXRATIOTIMEOUT); continue; }
		buflen+=iret; buffer[buflen]=0;
		//����MSN SS����������
		unsigned long trID=0; //SS������ӦID
		char *tmpptr,*ptrCmd,*ptrBegin=buffer;
//		RW_LOG_DEBUG("[msnlibXXXX] recevied Command from SS: %s.\r\n",ptrBegin);
		while( (ptrCmd=strchr(ptrBegin,'\r')) )
		{
			*(char *)ptrCmd=0;//��ʼ��������
			if(ptrBegin[0]==0) goto NextCMD; //�������������

//����MSN SS���������� ---- begin-------------------------------------------
//			RW_LOG_DEBUG("[msnlib] recevied Command from SS: %s.\r\n",ptrBegin);
			if( strncmp(ptrBegin,"MSG ",4)==0 )
			{//���������Ϣ,��ʽ����
				//MSG yycnet@hotmail.com yyc:) 91  ����91ָ����Ϣ�ĺ��������յ��ֽ���
				if( (tmpptr=strrchr(ptrBegin,' ')) )
				{//��ȡ��Ϣ���� len
					int len=atoi(tmpptr+1); 
					//�ж�����Ľ���������\r����\r\n
					int nSkip=(*(ptrCmd+1)=='\n')?2:1;
					if( (buflen-(ptrCmd-buffer+nSkip))>=len) 
					{ //��Ϣ�Ѿ�ȫ���������
						//��ptrBegin+4ָ��MSG��Ϣ�ķ�����Email��ȥ���������������
						if( (tmpptr=strchr(ptrBegin+4,' ')) ) *tmpptr=0;

						char c=*(ptrCmd+len+nSkip); 
						*(ptrCmd+len+nSkip)=0;//����Ҫ������ַ����н�������
						//������Ϣ //+nSkip����\r\n
						if(ptrBegin[0]=='M')
							pmsnmessager->sscmd_msg(pcon,ptrBegin+4,ptrCmd+nSkip,len); 

						*(ptrCmd+len+nSkip)=c;
						//����������Ϣ,ע��whileѭ����ptrBegin=ptrCmd+1;���Ҫ-1
						ptrCmd+=(len+nSkip-1);
					}
					else{ *ptrCmd='\r'; break; }//��������ʣ�����Ϣ
				}//?if( (tmpptr=strrchr(ptrBegin,' ')) )
			}//?strncmp(ptrBegin,"MSG ",4)
			else if( atol(ptrBegin)!=0 ) 
			{//ĳ������Ĵ��󷵻� ��ʽ: ������ trID ˵��\r\n
				if( (tmpptr=strchr(ptrBegin,' ')) ) 
					trID=(unsigned long)atol(tmpptr+1);
			}//?if( atol(ptrBegin)!=0 )
			else if(strncmp(ptrBegin,"IRO ",4)==0)
			{//��������ҷ�������죬�������˽������յ�IRO
				//IRO <trID> 1 1 yycnet@hotmail.com yyc:)\r\n
				//������email
				char *ptrStart=NULL,*ptr=ptrBegin+4;
				int icount=0;
				while(*ptr){
					if(*ptr==' '){
						icount++;
						if(icount==3) 
							ptrStart=ptr+1;
						else if(icount==4){ *ptr=0; break; }
					}//?if(*ptr==' ')
					ptr++;
				}//?while
				if(ptrStart!=NULL)
					pmsnmessager->onChatSession((HCHATSESSION)pcon,MSN_CHATSESSION_JOIN,
							ptrStart,pcon->m_chat_contacts);
				pcon->m_chat_contacts++;
			}//?else if(strncmp(pcmdbuf,"IRO ",4)==0)
			else if(strncmp(ptrBegin,"JOI ",4)==0)
			{//������ҷ�������죬�������˽������յ�JOI
				//JOI yycnet@hotmail.com yyc:)\r\n
				if( (tmpptr=strchr(ptrBegin+4,' ')) ) *tmpptr=0;
				pmsnmessager->onChatSession((HCHATSESSION)pcon,MSN_CHATSESSION_JOIN,
					ptrBegin+4,pcon->m_chat_contacts);
				pcon->m_chat_contacts++;
			}//?else if(strncmp(pcmdbuf,"JOI ",4)==0)
			else if(strncmp(ptrBegin,"BYE ",4)==0)
			{//ĳ���û��˳�������
				pmsnmessager->onChatSession((HCHATSESSION)pcon,MSN_CHATSESSION_BYE,
					ptrBegin+4,pcon->m_chat_contacts);
				if( --pcon->m_chat_contacts<=0 ) goto EXIT1;
			}//?else if(strncmp(pcmdbuf,"BYE ",4)==0)
			else if(strncmp(ptrBegin,"ACK ",4)==0)
			{ //ACK trID\r\n
				trID=(unsigned long)atol(ptrBegin+4);
			}
			else
				RW_LOG_DEBUG("[msnlib] recevied Command from SS: %s.\r\n",ptrBegin);

			if(trID!=0){
				pmsnmessager->m_mutex.lock();
				std::map<unsigned long,cCond *>::iterator it=pmsnmessager->m_conds.find(trID);
				if(it!=pmsnmessager->m_conds.end()){ 
					char *respbuf=(char *)((*it).second->getArgs());
					if(respbuf) strcpy(respbuf,ptrBegin);
					(*it).second->active(); 
				}
				pmsnmessager->m_mutex.unlock(); trID=0;
			}//?if(trID!=0)
//����MSN SS���������� ----  end -------------------------------------------
			

NextCMD:	//�ƶ�ptrBegin����һ������������ʼ
			ptrBegin=ptrCmd+1; 
			while(*ptrBegin=='\r' || *ptrBegin=='\n') ptrBegin++; //����\r\n
		}//?while
		
		//�ж��Ƿ���δ�����������
		if( (iret=(ptrBegin-buffer))> 0)
		{
			if((buflen-iret)>0) //������δ������
			{
				buflen-=iret;
				memmove((void *)buffer,ptrBegin,buflen);
			}
			else buflen=0; //������ȫ��������
		}
		//���ptrBegin-buf==0˵������һ��δ������İ�
/***********yyc remove 2006-09-01*********************************
	//�����δ��������������ƶ�
		if((iret=(ptrBegin-buffer))>0 && (buflen-iret)>0)
		{//���ptrBegin-buf==0˵������һ�������������ݰ� �˾�����MSG����
			buflen-=iret;
			memmove((void *)buffer,ptrBegin,buflen);
		} else buflen=0;
*****************************************************************/
	}//?while
EXIT1:
	pchatsock->Send(5,"OUT\r\n",-1);
	pchatsock->Close(); pcon->m_shell.destroy();
	pmsnmessager->onChatSession((HCHATSESSION)pcon,MSN_CHATSESSION_DESTROY,NULL,0);
	if(pcon->m_vecMessage.size()>0) //yyc add 2006-03-13
	{
		std::vector<std::pair<char *,long> >::iterator it=pcon->m_vecMessage.begin();
		for(;it!=pcon->m_vecMessage.end();it++) delete[] (*it).first;
		pcon->m_vecMessage.clear();
	}
	pcon->m_chat_contacts=0;
	std::map<std::string,cMsncx *>::iterator it=pcon->m_msncxMaps.begin();
	for(;it!=pcon->m_msncxMaps.end();it++) delete (*it).second; 
	pcon->m_msncxMaps.clear(); pcon->m_msnc1Maps.clear();

	RW_LOG_PRINT(LOGLEVEL_WARN,"session-thread(%s) of msnMessager has been ended\r\n",
		pcon->m_email.c_str());
	return;
}
//��cmd shell�������߳�
//��ȡcmd shell����������͵�chat�ͻ���
void msnMessager :: shellThread(void *param)
{
	std::pair<msnMessager *,cContactor *> *pp=(std::pair<msnMessager *,cContactor *> *)param;
	msnMessager *pmsnmessager=pp->first;
	cContactor *pcon=pp->second; delete pp;
	if(pmsnmessager==NULL || pcon==NULL) return;
	RW_LOG_PRINT(LOGLEVEL_DEBUG,0,"shell-thread of msnMessager has been started.\r\n");
	

	char msgHeader[512]; //����56�ֽڵĿռ�����д��MSG����ͷ
	int headerlen=pmsnmessager->encodeChatMsgHead(msgHeader+56,512-57,NULL,NULL);
	socketProxy *pchatsock=&pcon->m_chatSock;
	char Buf[2048]; int bufLen=0; 
	bool bSend=false;
	time_t tSend=time(NULL);
	while(pchatsock->status()==SOCKS_CONNECTED)
	{
		cUtils::usleep(SCHECKTIMEOUT); //����200ms
		int iret=pcon->m_shell.Read(Buf+bufLen,2048-bufLen-1);
		if(iret<0) break; 
		bufLen+=iret; Buf[bufLen]=0;
		if(iret==0){
			if((time(NULL)-tSend)>=MSN_MAX_RESPTIMEOUT)
				bSend=true; //��ֹchatSessionû�н������ر�
			else if(bufLen>0) bSend=true;//������������
		}//?if(iret==0)
		else if(bufLen>=1500) bSend=true; //������������
		if(bSend){
			if(bufLen<=0)
				pmsnmessager->sendcmd_SS_Typing(pcon,NULL);
			else if( !pmsnmessager->sendcmd_SS_chatMsg(pcon,msgHeader,headerlen,Buf,bufLen) ) 
				break;
			bufLen=0; bSend=false;
			tSend=time(NULL);
		}
	}//?while

	RW_LOG_PRINT(LOGLEVEL_DEBUG,0,"shell-thread of msnMessager has been ended\r\n");
	return;
}
