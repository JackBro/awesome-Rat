/*******************************************************************
   *	NS_sendcmd.cpp
   *    DESCRIPTION:��NS���������͵�����
   *
   *    AUTHOR:yyc
   *
   *    HISTORY:
   *
   *    DATE:2005-06-28
   *	net4cpp 2.0
   *******************************************************************/

#include "../../include/sysconfig.h"
#include "../../include/cCoder.h"
#include "../../include/cLogger.h"
#include "msnlib.h"

using namespace std;
using namespace net4cpp21;
int splitstring(const char *str,char delm,std::vector<std::string> &vec,int maxSplit=0);

bool msnMessager :: sendcmd_VER()
{
	socketProxy &m_nsSocket=m_curAccount.m_chatSock;
	unsigned long trID=msgID();
	char buf[64]; 
	int iret=sprintf(buf,"VER %d MSNP11 MSNP10 CVR0\r\n",trID);
	buf[iret]=0; RW_LOG_PRINT(LOGLEVEL_DEBUG,"[msnlib] ---> %s",buf);
	if( m_nsSocket.Send(iret,buf,-1)<0 ) return false;
	iret=m_nsSocket.Receive(buf,63,MSN_MAX_RESPTIMEOUT);
	if(iret<0){ 
		iret=sprintf(buf,"[msnlib] failed to receive answer of VER %d,err=%d\r\n",trID,iret);
		RW_LOG_PRINT(LOGLEVEL_DEBUG,iret,buf); return false;
	} else buf[iret]=0;
	RW_LOG_PRINT(LOGLEVEL_DEBUG,"[msnlib] <--- %s",buf);
	return (strncmp(buf,"VER ",4)!=0)?false:true;
}
bool msnMessager :: sendcmd_CVR(const char *strAccount)
{
	socketProxy &m_nsSocket=m_curAccount.m_chatSock;
	unsigned long trID=msgID();
	char buf[256]; 
	int iret=sprintf(buf,"CVR %d 0x0804 winnt 5.0 i386 MSNMSGR 7.0.0813 msmsgs %s\r\n",trID,strAccount);
	buf[iret]=0; 
	RW_LOG_PRINT(LOGLEVEL_DEBUG,"[msnlib] ---> %s",buf);
	if( m_nsSocket.Send(iret,buf,-1)<0 ) return false;
	iret=m_nsSocket.Receive(buf,255,MSN_MAX_RESPTIMEOUT);
	if(iret<0){ 
		iret=sprintf(buf,"[msnlib] failed to receive answer of CVR %d,err=%d\r\n",trID,iret);
		RW_LOG_PRINT(LOGLEVEL_DEBUG,iret,buf); return false;
	} else buf[iret]=0;
	RW_LOG_PRINT(LOGLEVEL_DEBUG,"[msnlib] <--- %s",buf);
	return (strncmp(buf,"CVR ",4)!=0)?false:true;
}
//���������ȡNS�������ĵ�ַ
//�ɹ�����0,���򷵻ش�����
int msnMessager :: sendcmd_USR(const char *strAccount,std::string &strNShost,int &iNSport)
{
	socketProxy &m_nsSocket=m_curAccount.m_chatSock;
	unsigned long trID=msgID();
	char buf[128]; 
	int iret=sprintf(buf,"USR %d TWN I %s\r\n",trID,strAccount);
	buf[iret]=0; 
	RW_LOG_PRINT(LOGLEVEL_DEBUG,"[msnlib] ---> %s",buf);
	if( m_nsSocket.Send(iret,buf,-1)<0 ) 
		return SOCKSERR_MSN_UNKNOWED;
	iret=m_nsSocket.Receive(buf,127,MSN_MAX_RESPTIMEOUT);
	if(iret<0){ 
		iret=sprintf(buf,"[msnlib] failed to receive answer of USR %d,err=%d\r\n",trID,iret);
		RW_LOG_PRINT(LOGLEVEL_DEBUG,iret,buf); return SOCKSERR_TIMEOUT;
	} else buf[iret]=0;
	RW_LOG_PRINT(LOGLEVEL_DEBUG,"[msnlib] <--- %s",buf);
	//��ȷ�ķ��ظ�ʽ XFR 3 NS 207.46.0.21:1863 0 65.54.239.140:1863
	//�������Ч���ʺŽ����� 911 trID
	if(strncmp(buf,"XFR ",4)!=0) return SOCKSERR_MSN_EMAIL;
	const char *ptr1,*ptr=strstr(buf+4,"NS ");
	if(ptr) ptr1=strchr(ptr+3,':');
	if(ptr==NULL || ptr1==NULL) return SOCKSERR_MSN_GETNS;
	strNShost.assign(ptr+3,ptr1-ptr-3); iNSport=atoi(ptr1+1);
	return MSN_ERR_OK;
}
//���������ȡhttps hashkey
bool msnMessager :: sendcmd_USR(const char *strAccount,std::string &hashkey)
{
	socketProxy &m_nsSocket=m_curAccount.m_chatSock;
	unsigned long trID=msgID();
	char buf[256]; 
	int iret=sprintf(buf,"USR %d TWN I %s\r\n",trID,strAccount);
	buf[iret]=0; 
	RW_LOG_PRINT(LOGLEVEL_DEBUG,"[msnlib] ---> %s",buf);
	if( m_nsSocket.Send(iret,buf,-1)<0 ) return false;
	iret=m_nsSocket.Receive(buf,255,MSN_MAX_RESPTIMEOUT);
	if(iret<0){ 
		iret=sprintf(buf,"[msnlib] failed to receive answer of USR %d,err=%d\r\n",trID,iret);
		RW_LOG_PRINT(LOGLEVEL_DEBUG,iret,buf); return false;
	} else buf[iret]=0;
	RW_LOG_PRINT(LOGLEVEL_DEBUG,"[msnlib] <--- %s",buf);
	//��ȷ�ķ��ظ�ʽ USR 6 TWN S lc=1033,id=507,tw=40,fs=1,ru=http%3A%2F%2Fmessenger%2Emsn%2Ecom,ct=1119856294,kpp=1,kv=6,ver=2.1.6000.1,rn=7o5vXh8s,tpf=09b41a915a8e8469b1d3f23814be8e6b
	if(strncmp(buf,"USR ",4)!=0) return false;
	const char *ptr=strstr(buf+4,"TWN S ");
	if(ptr==NULL) return false;
	hashkey.assign(ptr+6);//ȥ������\r\n
	if(hashkey[hashkey.length()-2]=='\r') hashkey.erase(hashkey.length()-2);
	return true;
}
bool msnMessager :: sendcmd_USR(std::string &hashkey)
{
	socketProxy &m_nsSocket=m_curAccount.m_chatSock;
	unsigned long trID=msgID();
	char buf[512]; 
	int iret=sprintf(buf,"USR %d TWN S %s\r\n",trID,hashkey.c_str());
	buf[iret]=0; 
	RW_LOG_PRINT(LOGLEVEL_DEBUG,"[msnlib] ---> %s",buf);
	if( m_nsSocket.Send(iret,buf,-1)<0 ) return false;
	iret=m_nsSocket.Receive(buf,128,MSN_MAX_RESPTIMEOUT);
	if(iret<0){ 
		iret=sprintf(buf,"[msnlib] failed to receive answer of USR %d,err=%d\r\n",trID,iret);
		RW_LOG_PRINT(LOGLEVEL_DEBUG,iret,buf); return false;
	} else buf[iret]=0;
	RW_LOG_PRINT(LOGLEVEL_DEBUG,"[msnlib] <--- %s",buf);
	//��ȷ�ķ��ظ�ʽ USR 7 OK yycnet@hotmail.com 1 0
	if(strncmp(buf,"USR ",4)==0 && strstr(buf+4," OK ") ) return true;
	return false;
}
//����״̬�ı���Ϣ "NLN","FLN","IDL","BSY","AWY","BRB","PHN","LUN","HDN"
//!���ʺŵ�¼��������ϵ���б��Ҫ����"NLN",����״̬
bool msnMessager :: sendcmd_CHG(const char *sta)
{
	unsigned long trID=msgID();
	char buf[512]; int iret;
	if(sta!=NULL) m_curAccount.m_status.assign(sta);
	if(m_curAccount.m_strMsnObj!="")
		iret=sprintf(buf,"CHG %d %s %d %s\r\n",trID,sta,m_curAccount.m_clientID,m_curAccount.m_strMsnObj.c_str());
	else
		iret=sprintf(buf,"CHG %d %s %d\r\n",trID,sta,m_curAccount.m_clientID);
	buf[iret]=0;
	return (m_curAccount.m_chatSock.Send(iret,buf,-1)>0)?true:false;
}

bool msnMessager :: sendcmd_UUX()
{
	unsigned long trID=msgID();
	char buf[128]; 
	int iret=sprintf(buf,"UUX %d 53\r\n<Data><PSM></PSM><CurrentMedia></CurrentMedia></Data>",trID); 
	buf[iret]=0;
	return (m_curAccount.m_chatSock.Send(iret,buf,-1)>0)?true:false;
}

//PNG����ط��� - QNG 43\r\n
bool msnMessager :: sendcmd_PNG()
{
	return (m_curAccount.m_chatSock.Send(5,"PNG\r\n",-1)>0)?true:false;
}
//strFlag - "FL", "AL", "BL", "RL"
bool msnMessager :: sendcmd_REM(const char *email,const char *strFlag)
{
	unsigned long trID=msgID();
	char buf[64]; 
	int iret=sprintf(buf,"REM %d %s %s\r\n",trID,strFlag,email);
	buf[iret]=0;
	return (m_curAccount.m_chatSock.Send(iret,buf,-1)>0)?true:false;
}

//strFlag - "FL", "AL", "BL", "RL"
bool msnMessager :: sendcmd_ADC(const char *email,const char *strFlag)
{
	unsigned long trID=msgID();
	char buf[64]; 
	int iret=sprintf(buf,"ADC %d %s N=%s\r\n",trID,strFlag,email);
	buf[iret]=0;
	return (m_curAccount.m_chatSock.Send(iret,buf,-1)>0)?true:false;
}

//nick --- ����utf8��mime������ǳ��ַ���
bool msnMessager :: sendcmd_ADC(const char *email,const char *nick,long waittimeout)
{
	unsigned long trID=msgID();
	char buf[256]; 
	int iret=sprintf(buf,"ADC %d FL N=%s F=%s\r\n",trID,email,nick);
	buf[iret]=0; bool bret=false;
	if(waittimeout<0) waittimeout=MSN_MAX_RESPTIMEOUT;
	cCond cond; cond.setArgs((long)buf);
	if(m_curAccount.m_chatSock.Send(iret,buf,-1)<0) return bret;
	if(waittimeout>0){	 
		buf[0]=0; m_conds[trID]=&cond;
		bret=cond.wait(waittimeout);
		eraseCond(trID);
		if(bret){ if(strncmp(buf,"ADC ",4)!=0) bret=false; }
	} else bret=true;
	return bret;;
}
//���ͽ���chat session����
bool msnMessager :: sendcmd_XFR(socketProxy &chatSock,const char *email)
{
	unsigned long trID=msgID();
	char buf[256];
	int iret=sprintf(buf,"XFR %d SB\r\n",trID);
	buf[iret]=0; 
	cCond cond; cond.setArgs((long)&buf);
	m_conds[trID]=&cond;
	if(m_curAccount.m_chatSock.Send(iret,buf,-1)>0)
	{	
		buf[0]=0;
		cond.wait(MSN_MAX_RESPTIMEOUT);
	} else buf[0]=0;
	eraseCond(trID);
	if(buf[0]==0) return false; //buf�б����ΪXFR�������Ӧ���,�����ʽ����
	////XFR 53 SB 207.46.4.174:1863 CKI 312825.1120186211.16162
	std::vector<std::string> v;
	iret=splitstring(buf,' ',v);
	if(iret<6 || v[0]!="XFR" || (unsigned long)atol(v[1].c_str())!=trID) return false;
	if(!connectSvr(chatSock,v[3].c_str(),0)) return false;

	trID=msgID();
	iret=sprintf(buf,"USR %d %s %s\r\n",trID,m_curAccount.m_email.c_str(),v[5].c_str());
	buf[iret]=0;
	if( chatSock.Send(iret,buf,-1)<=0 ) return false;
	if((iret=chatSock.Receive(buf,255,MSN_MAX_RESPTIMEOUT))<=0) 
		return false;
	buf[iret]=0; //�������ݸ�ʽ USR 160 OK yycnet@hotmail.com yyc:)
	if(strncmp(buf,"USR ",4) || strstr(buf+4," OK ")==NULL ) return false;
	
	trID=msgID();
	iret=sprintf(buf,"CAL %d %s\r\n",trID,email);
	buf[iret]=0;
	if( chatSock.Send(iret,buf,-1)<=0 ) return false;
	if((iret=chatSock.Peek(buf,255,MSN_MAX_RESPTIMEOUT))<=0) 
		return false;
	buf[iret]=0; //�������ݸ�ʽ CAL 161 RINGING 312825
	return (strncmp(buf,"CAL ",4)==0)?true:false;
//yyc remove �п��ܻ�Ѻ��������ݽ�������������sessionThread�������ݳ��ִ���
//	if((iret=chatSock.Receive(buf,255,MSN_MAX_RESPTIMEOUT))<=0) 
//		return false;
//	buf[iret]=0; //�������ݸ�ʽ CAL 161 RINGING 312825
//	return (strncmp(buf,"CAL ",4)==0)?true:false;
}

//���ͱ�ĳ����ϵ���������������Ϣ
bool msnMessager :: sendcmd_SS_Typing(cContactor *pcon,const char *type_email)
{
	socketProxy &chatSock=pcon->m_chatSock;
	if(chatSock.status()!=SOCKS_CONNECTED) return false;
	if(pcon->m_chat_contacts<=0) return true; //��ʱ�����ͣ���Ϊ��û���˼������죬�����ʱ���ͻ�����MSN����ر�session����
	if(type_email==NULL) type_email=m_curAccount.m_email.c_str();
	char buf[256];
	int len=sprintf(buf+56,"MIME-Version: 1.0\r\nContent-Type: text/x-msmsgscontrol\r\nTypingUser: %s\r\n\r\n",
			type_email);
	int iret=sprintf(buf,"MSG %d U %d\r\n",msgID(),len);
	memmove(buf+(56-iret),buf,iret);
	return (chatSock.Send(len+iret,buf+(56-iret),-1)>0)?true:false;
}

//������������
//�������ݳ��ȱ�������1540(�������ֽڴ�С)��󳤶�
//�����������������ݵ��ֽڳ��ȴ���ָ�����Ҫ�ָ�ͣ�����1500�ָ��
//msgHeader --- ת�����õ�msgHeader���壬��ǰ56�ֽ�Ϊ�����ȴ�д��MSG���ͱ�Ǻͳ���
bool msnMessager::sendcmd_SS_chatMsg(cContactor *pcon,char *msgHeader,
						int headerlen,const char *chatMsg,int msglen)
{
	socketProxy &chatSock=pcon->m_chatSock;
	if(chatSock.status()!=SOCKS_CONNECTED) return false;
//	��Ҫ���͵���Ϣ����utf8����
	if(msglen<=0) msglen=strlen(chatMsg);
	char *pmsgbuf=new char[cCoder::Utf8EncodeSize(msglen)];
	if( pmsgbuf==NULL ) return false;
	msglen=cCoder::utf8_encode(chatMsg,msglen,pmsgbuf);
	pmsgbuf[msglen]=0; chatMsg=pmsgbuf; 
	int iSend,iret; unsigned long trID;
	//���������ݰ�1500�ֽڳ��Ƚ��зָ��
	while(true)
	{
		if( (iSend=msglen) >1500)
		{
			iSend=1500; //�����utf8������ַ������˵�һ���ֽڣ������ֽڶ�����0x10��ͷ,��utf8����˵��
			while( chatMsg[iSend]<0 ) //��ֹ��utf8������ַ��ضϷ��ͣ�
			{
				if( ((chatMsg[iSend]>>6) & 0x3)!=0x2 ) break;
				iSend--;
			}
		}//?if(iSend>1500)
//---------------������Ϣ----------------------------
		trID=msgID();
		iret=sprintf(msgHeader,"MSG %d A %d\r\n",trID,headerlen+iSend);
		memmove(msgHeader+(56-iret),msgHeader,iret);
		if(pcon->m_chat_contacts>0)
		{
			chatSock.Send(headerlen+iret,msgHeader+(56-iret),-1);
			chatSock.Send(iSend,chatMsg,-1);
		}
		else
		{
			long l=headerlen+iret+iSend;
			char *pm=new char[l];
			if(pm){
				memcpy(pm,msgHeader+(56-iret),headerlen+iret);
				memcpy(pm+headerlen+iret,chatMsg,iSend);
				std::pair<char *,long> p(pm,l);
				pcon->m_vecMessage.push_back(p);
			}
			trID=0;
		}
//----------------------------------------------------
		chatMsg+=iSend; msglen-=iSend;
		if(msglen<=0) break;
		if(trID!=0 && trID%2==0) //Ϊ�˱��ⷢ��̫�쵼��MSN����ر����ӣ��ȴ���ӦӦ��
		{ //yyc add 2007-03-13
			cCond cond; cond.setArgs(0);
			this->m_conds[trID]=&cond; //���ʱ�ȴ�3��
			cond.wait(3); this->eraseCond(trID);
		}//?if(trID%4==0)
		//�����������
	}//?while(true);
	delete[] pmsgbuf; return true;
}
bool msnMessager::sendcmd_SS_chatMsgW(cContactor *pcon,char *msgHeader,
						int headerlen,const wchar_t *chatMsgW,int msglen)
{
	socketProxy &chatSock=pcon->m_chatSock;
	if(chatSock.status()!=SOCKS_CONNECTED) return false;
//	��Ҫ���͵���Ϣ����utf8����
	if(msglen<=0) msglen=stringlenW(chatMsgW);
	char *pmsgbuf=new char[cCoder::Utf8EncodeSize(msglen)];
	if( pmsgbuf==NULL ) return false;
	msglen=cCoder::utf8_encodeW(chatMsgW,msglen,pmsgbuf);
	pmsgbuf[msglen]=0; const char *chatMsg=pmsgbuf; 
	int iSend,iret; unsigned long trID;
	//���������ݰ�1500�ֽڳ��Ƚ��зָ��
	while(true)
	{
		if( (iSend=msglen) >1500)
		{
			iSend=1500; //�����utf8������ַ������˵�һ���ֽڣ������ֽڶ�����0x10��ͷ,��utf8����˵��
			while( chatMsg[iSend]<0 ) //��ֹ��utf8������ַ��ضϷ��ͣ�
			{
				if( ((chatMsg[iSend]>>6) & 0x3)!=0x2 ) break;
				iSend--;
			}
		}//?if(iSend>1500)
//---------------������Ϣ----------------------------
		trID=msgID();
		iret=sprintf(msgHeader,"MSG %d A %d\r\n",trID,headerlen+iSend);
		memmove(msgHeader+(56-iret),msgHeader,iret);
		if(pcon->m_chat_contacts>0)
		{
			chatSock.Send(headerlen+iret,msgHeader+(56-iret),-1);
			chatSock.Send(iSend,chatMsg,-1);
		}
		else
		{
			long l=headerlen+iret+iSend;
			char *pm=new char[l];
			if(pm){
				memcpy(pm,msgHeader+(56-iret),headerlen+iret);
				memcpy(pm+headerlen+iret,chatMsg,iSend);
				std::pair<char *,long> p(pm,l);
				pcon->m_vecMessage.push_back(p);
			}
			trID=0;
		}
//----------------------------------------------------
		chatMsg+=iSend; msglen-=iSend;
		if(msglen<=0) break;
		if(trID!=0 && trID%2==0) //Ϊ�˱��ⷢ��̫�쵼��MSN����ر����ӣ��ȴ���ӦӦ��
		{//yyc add 2007-03-13
			cCond cond; cond.setArgs(0);
			this->m_conds[trID]=&cond; //���ʱ�ȴ�3��
			cond.wait(3); this->eraseCond(trID);
		}//?if(trID%4==0)
		//�����������
	}//?while(true);
	delete[] pmsgbuf; return true;
}

bool msnMessager::sendcmd_SS_chatMsg(std::vector<cContactor *> &vec,char *msgHeader,
									 int headerlen,const char *chatMsg,int msglen)
{
	//	��Ҫ���͵���Ϣ����utf8����
	if(msglen<=0) msglen=strlen(chatMsg);
	char *pmsgbuf=new char[cCoder::Utf8EncodeSize(msglen)];
	if( pmsgbuf==NULL ) return false;
	msglen=cCoder::utf8_encode(chatMsg,msglen,pmsgbuf);
	pmsgbuf[msglen]=0; chatMsg=pmsgbuf; 
	int iSend,iret;
	//���������ݰ�1500�ֽڳ��Ƚ��зָ��
	do{
		if( (iSend=msglen) >1500)
		{
			iSend=1500; //�����utf8������ַ������˵�һ���ֽڣ������ֽڶ�����0x10��ͷ,��utf8����˵��
			while( chatMsg[iSend]<0 ) //��ֹ��utf8������ַ��ضϷ��ͣ�
			{
				if( ((chatMsg[iSend]>>6) & 0x3)!=0x2 ) break;
				iSend--;
			}
		}//?if(iSend>1500)
//---------------������Ϣ----------------------------
		std::vector<cContactor *>::iterator it=vec.begin();
		for(;it!=vec.end();it++)
		{
			iret=sprintf(msgHeader,"MSG %d A %d\r\n",msgID(),headerlen+iSend);
			memmove(msgHeader+(56-iret),msgHeader,iret);
			if((*it)->m_chat_contacts>0)
			{
				(*it)->m_chatSock.Send(headerlen+iret,msgHeader+(56-iret),-1);
				(*it)->m_chatSock.Send(iSend,chatMsg,-1);
			}
			else
			{
				long l=headerlen+iret+iSend;
				char *pm=new char[l];
				if(pm){
					memcpy(pm,msgHeader+(56-iret),headerlen+iret);
					memcpy(pm+headerlen+iret,chatMsg,iSend);
					std::pair<char *,long> p(pm,l);
					(*it)->m_vecMessage.push_back(p);
				}
			}
		}//?for;
//----------------------------------------------------
		chatMsg+=iSend; msglen-=iSend;
	}while(msglen>0);
	delete[] pmsgbuf; return true;
}


bool msnMessager::sendcmd_SS_chatMsgW(std::vector<cContactor *> &vec,char *msgHeader,
									 int headerlen,const wchar_t *chatMsgW,int msglen)
{
	//	��Ҫ���͵���Ϣ����utf8����
	if(msglen<=0) msglen=stringlenW(chatMsgW);
	char *pmsgbuf=new char[cCoder::Utf8EncodeSize(msglen)];
	if( pmsgbuf==NULL ) return false;
	msglen=cCoder::utf8_encodeW(chatMsgW,msglen,pmsgbuf);
	pmsgbuf[msglen]=0; const char *chatMsg=pmsgbuf; 
	int iSend,iret;
	//���������ݰ�1500�ֽڳ��Ƚ��зָ��
	do{
		if( (iSend=msglen) >1500)
		{
			iSend=1500; //�����utf8������ַ������˵�һ���ֽڣ������ֽڶ�����0x10��ͷ,��utf8����˵��
			while( chatMsg[iSend]<0 ) //��ֹ��utf8������ַ��ضϷ��ͣ�
			{
				if( ((chatMsg[iSend]>>6) & 0x3)!=0x2 ) break;
				iSend--;
			}
		}//?if(iSend>1500)
//---------------������Ϣ----------------------------
		std::vector<cContactor *>::iterator it=vec.begin();
		for(;it!=vec.end();it++)
		{
			iret=sprintf(msgHeader,"MSG %d A %d\r\n",msgID(),headerlen+iSend);
			memmove(msgHeader+(56-iret),msgHeader,iret);
			if((*it)->m_chat_contacts>0)
			{
				(*it)->m_chatSock.Send(headerlen+iret,msgHeader+(56-iret),-1);
				(*it)->m_chatSock.Send(iSend,chatMsg,-1);
			}
			else
			{
				long l=headerlen+iret+iSend;
				char *pm=new char[l];
				if(pm){
					memcpy(pm,msgHeader+(56-iret),headerlen+iret);
					memcpy(pm+headerlen+iret,chatMsg,iSend);
					std::pair<char *,long> p(pm,l);
					(*it)->m_vecMessage.push_back(p);
				}
			}
		}//?for;
//----------------------------------------------------
		chatMsg+=iSend; msglen-=iSend;
	}while(msglen>0);
	delete[] pmsgbuf; return true;
}


int msnMessager :: encodeChatMsgHead(char *buffer,int buflen,const char *IMFont,const char *dspname)
{
//	ASSERT(buffer!=NULL);
//	ASSERT(buflen>256);
	int len=sprintf(buffer,"MIME-Version: 1.0\r\n"
						   "Content-Type: text/plain; charset=UTF-8\r\n"
						   "X-MMS-IM-Format: ");
	if(IMFont==NULL || IMFont[0]==0)
	{
		if(m_encodeFontname==""){//���������ƽ���utf-8��mime����
			m_encodeFontname=m_fontName;
			int iret=cCoder::utf8_encode(m_encodeFontname.c_str(),m_encodeFontname.length(),buffer+len);
			buffer[len+iret]=0; m_encodeFontname.assign(buffer+len);
			iret=cCoder::mime_encodeEx(m_encodeFontname.c_str(),m_encodeFontname.length(),buffer+len);
			buffer[len+iret]=0; m_encodeFontname.assign(buffer+len);
		}//?if(m_encodeFontname=="")
		len+=sprintf(buffer+len,"FN=%s; EF=%s; CO=%x; CS=%d; PF=%d\r\n",
			m_encodeFontname.c_str(),m_fontEF.c_str(),m_fontColor,0,0);
	}
	else
	{
		std::string encodeFontname;
		int iret=cCoder::utf8_encode(IMFont,strlen(IMFont),buffer+len);
		buffer[len+iret]=0; encodeFontname.assign(buffer+len);
		len+=cCoder::mime_encodeEx(encodeFontname.c_str(),encodeFontname.length(),buffer+len);
		buffer[len++]='\r'; buffer[len++]='\n';
	}
	
	if(dspname && cCoder::Utf8EncodeSize(strlen(dspname))<(buflen-len))
	{//����ʾ���ƽ���utf8����,���Ա��뻺������С�ӱ����޶�
		strcpy(buffer+len,"P4-Context: "); len+=12;
		len+=cCoder::utf8_encode(dspname,strlen(dspname),buffer+len);
		buffer[len++]='\r'; buffer[len++]='\n';
	}
	buffer[len++]='\r'; buffer[len++]='\n';

	return len;
}


int msnMessager :: encodeChatMsgHeadW(char *buffer,int buflen,const wchar_t *IMFont,const wchar_t *dspname)
{
//	ASSERT(buffer!=NULL);
//	ASSERT(buflen>256);
	int len=sprintf(buffer,"MIME-Version: 1.0\r\n"
						   "Content-Type: text/plain; charset=UTF-8\r\n"
						   "X-MMS-IM-Format: ");
	if(IMFont==NULL || IMFont[0]==0)
	{
		if(m_encodeFontname==""){//���������ƽ���utf-8��mime����
			m_encodeFontname=m_fontName;
			int iret=cCoder::utf8_encode(m_encodeFontname.c_str(),m_encodeFontname.length(),buffer+len);
			buffer[len+iret]=0; m_encodeFontname.assign(buffer+len);
			iret=cCoder::mime_encodeEx(m_encodeFontname.c_str(),m_encodeFontname.length(),buffer+len);
			buffer[len+iret]=0; m_encodeFontname.assign(buffer+len);
		}//?if(m_encodeFontname=="")
		len+=sprintf(buffer+len,"FN=%s; EF=%s; CO=%x; CS=%d; PF=%d\r\n",
			m_encodeFontname.c_str(),m_fontEF.c_str(),m_fontColor,0,0);
	}
	else
	{
		std::string encodeFontname;
		int iret=cCoder::utf8_encodeW(IMFont,stringlenW(IMFont),buffer+len);
		buffer[len+iret]=0; encodeFontname.assign(buffer+len);
		len+=cCoder::mime_encodeEx(encodeFontname.c_str(),encodeFontname.length(),buffer+len);
		buffer[len++]='\r'; buffer[len++]='\n';
	}
	
	if(dspname && cCoder::Utf8EncodeSize(stringlenW(dspname))<(buflen-len))
	{//����ʾ���ƽ���utf8����,���Ա��뻺������С�ӱ����޶�
		strcpy(buffer+len,"P4-Context: "); len+=12;
		len+=cCoder::utf8_encodeW(dspname,stringlenW(dspname),buffer+len);
		buffer[len++]='\r'; buffer[len++]='\n';
	}
	buffer[len++]='\r'; buffer[len++]='\n';

	return len;
}
