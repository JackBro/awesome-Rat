/*******************************************************************
   *	msnlib.cpp
   *    DESCRIPTION:msnЭ�鴦����ʵ��
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

msnMessager :: msnMessager()
{
	m_msgID=0;
	m_Connectivity='N';
	m_curAccount.m_clientID=0x4000d024;//0x4000d034 //���������ͷ�ú����clientID

	m_fontName="MS Sans Serif";
	m_encodeFontname=m_fontName;
	m_fontColor=0;
	m_fontEF="";
	m_lasterrorcode=MSN_ERR_OK;
}

msnMessager :: ~msnMessager()
{
	signout();
}

void msnMessager :: clear()
{
	m_mutex.lock();
	std::map<unsigned long, cCond *>::iterator it_cond=m_conds.begin();
	for(;it_cond!=m_conds.end();it_cond++)
		(*it_cond).second->active(); 
	m_mutex.unlock();
	m_threadpool.join();//�ȴ������߳̽���
	
	std::map<std::string, cContactor *>::iterator it_contact=m_contacts.begin();
	for(;it_contact!=m_contacts.end();it_contact++) 
		delete (*it_contact).second;
	m_contacts.clear();
	return;
}

void msnMessager :: signout()
{
	//�رպ�NS����������
	m_curAccount.m_chatSock.Send(5,"OUT\r\n",-1);
	m_curAccount.m_chatSock.Close();

	clear(); return;
}

bool msnMessager :: signin(const char *strAccount,const char *strPwd)
{
	if(strAccount==NULL || strAccount[0]==0){ 
		m_lasterrorcode= SOCKSERR_MSN_EMAIL;
		return false; 
	}

	socketProxy &m_nsSocket=m_curAccount.m_chatSock;
	m_lasterrorcode=SOCKSERR_MSN_SIGNIN; //�Ѿ���¼
	if(m_nsSocket.status()!=SOCKS_CLOSED) return true;//�Ѿ���¼
	
	clear();
	string strMsnNS; int iMsnNSport=0;//��ȡNS�������ĵ�ַ
	string hashkey;//��ȡhttps��hashkey
	RW_LOG_PRINT(LOGLEVEL_DEBUG,0,"[msnlib] connecting messenger.hotmail.com:1863,please waiting...\r\n");
	//����MSN��ǲ������
	if( !connectSvr(m_nsSocket,MSN_SERVER_HOST,MSN_SERVER_PORT) ) 
	{
		RW_LOG_PRINT(LOGLEVEL_INFO,"[msnlib] failed to connect DS(%s:%d) %s.\r\n",
			MSN_SERVER_HOST,MSN_SERVER_PORT,
			(m_nsSocket.proxyType()!=PROXY_NONE)?"by proxy":"");
		m_lasterrorcode= SOCKSERR_MSN_DSCONN; 
		goto EXIT1; //��������DS������
	}
	m_lasterrorcode= SOCKSERR_MSN_RESP;
	if(!sendcmd_VER()) goto EXIT1;
	if(!sendcmd_CVR(strAccount)) goto EXIT1;
	m_lasterrorcode=sendcmd_USR(strAccount,strMsnNS,iMsnNSport);
	if(m_lasterrorcode!=MSN_ERR_OK) goto EXIT1;

	if( !connectSvr(m_nsSocket,strMsnNS.c_str(),iMsnNSport) ) //����֪ͨ������
	{
		RW_LOG_PRINT(LOGLEVEL_INFO,"[msnlib] failed to connect NS(%s:%d) %s.\r\n",
			strMsnNS.c_str(),iMsnNSport,
			(m_nsSocket.proxyType()!=PROXY_NONE)?"by proxy":"");
		m_lasterrorcode= SOCKSERR_MSN_NSCONN; 
		goto EXIT1;//��������NS������
	}
	m_lasterrorcode= SOCKSERR_MSN_RESP;
	if(!sendcmd_VER()) goto EXIT1;
	if(!sendcmd_CVR(strAccount)) goto EXIT1;
	if(!sendcmd_USR(strAccount,hashkey)) 
	{ 
		m_lasterrorcode=SOCKSERR_MSN_GETHASHKEY; 
		goto EXIT1; 
	}
	m_lasterrorcode=passport_auth(hashkey,strAccount,strPwd);
	if(m_lasterrorcode!=MSN_ERR_OK)
	{//����https��passport ��֤ �ɹ�������֤��
		RW_LOG_PRINT(LOGLEVEL_DEBUG,0,"[msnlib] failed to get passport.\r\n");
		goto EXIT1;
	}
	
	if(!sendcmd_USR(hashkey)) { m_lasterrorcode=SOCKSERR_MSN_SIGNIN; goto EXIT1;}
	if( m_threadpool.addTask((THREAD_CALLBACK *)&receiveThread,(void *)this,0)==0 )
	{ 
		m_lasterrorcode=SOCKSERR_THREAD; 
		goto EXIT1;
	}
	//����SYN�����ȡ�����б�
	m_nsSocket.Send("SYN %d 0 0\r\n",msgID());
	m_curAccount.m_email.assign(strAccount);
	m_lasterrorcode=MSN_ERR_OK;
EXIT1:
	if(m_lasterrorcode!=MSN_ERR_OK) m_nsSocket.Close();
	return (m_lasterrorcode==MSN_ERR_OK);
}

//������������
bool msnMessager :: setChatFont(const char *fontName,const char *fontEF,long fontColor)
{
	m_lasterrorcode=MSN_ERR_OK;
	if(fontName){
		if(fontName[0]==0){
			m_fontName="MS Sans Serif";
			m_encodeFontname=m_fontName;
		}
		else
		{	
			m_fontName.assign(fontName);
			m_encodeFontname=m_fontName;
			//���������ƽ���utf8��mime����
			char buf[256]; 
			int iret=cCoder::utf8_encode(m_encodeFontname.c_str(),
				m_encodeFontname.length(),buf);
			buf[iret]=0; m_encodeFontname.assign(buf);
			iret=cCoder::mime_encodeEx(m_encodeFontname.c_str(),
				m_encodeFontname.length(),buf);
			buf[iret]=0; m_encodeFontname.assign(buf);
		}
	}
	if(fontColor>=0) m_fontColor=(fontColor & 0x00ffffff);
	if(fontEF){
		for(int i=0;i<strlen(fontEF);i++){
			if(fontEF[i]!='B' && fontEF[i]!='I' && fontEF[i]!='S' && fontEF[i]!='U')
				return false;
		}//?for(...
		m_fontEF.assign(fontEF);
	}
	return true;
}

//����PRP����ı��ǳ�
bool msnMessager :: changeNickW(const wchar_t *nickW,int nicklen)
{
	if(nickW==NULL || nickW[0]==0) return false;
	if(nicklen<=0) nicklen=stringlenW(nickW);
	int utf8nicklen=cCoder::Utf8EncodeSize(nicklen)+1;
	char *utf8nick=new char[utf8nicklen];
	if(utf8nick==NULL) return false;
	nicklen=cCoder::utf8_encodeW(nickW,nicklen,utf8nick);
	//����MIME����
	char *pbuf=new char[cCoder::MimeEncodeSize(nicklen)+32];
	if(pbuf==NULL){ delete[] utf8nick; return false; }
	nicklen=cCoder::mime_encodeEx(utf8nick,nicklen,pbuf+30);
	pbuf[30+nicklen++]='\r'; pbuf[30+nicklen++]='\n'; pbuf[30+nicklen]=0;
	unsigned long trID=msgID();
	int len=sprintf(pbuf,"PRP %d MFN ",trID);
	::memmove(pbuf+30-len,pbuf,len);
	bool b=(m_curAccount.m_chatSock.Send(nicklen+len,pbuf+30-len,-1)>0)?true:false;
	if(b){
		m_curAccount.m_nick.assign(nickW);
		wchar2chars(nickW,utf8nick,utf8nicklen);
		m_curAccount.m_nick_char.assign(utf8nick);
	}
	delete[] utf8nick;
	delete[] pbuf; return b;
}
bool msnMessager :: changeNick(const char *nick)
{
	if(nick==NULL || nick[0]==0) return false;
	int nicklen=strlen(nick);
	wchar_t *nickW=new wchar_t[nicklen+1];
	if(nickW==NULL) return false;
	//�����ֽڱ���ת��Ϊunicode˫�ֽڱ���
#ifdef WIN32
	nicklen=MultiByteToWideChar(CP_ACP,0,nick,nicklen,nickW,nicklen);
#else //�����ת����ת�����ַ�������(��Ȼ��˫�ֽ��룬������unicode��)
	nicklen=swprintf(nickW,L"%S",nick);
#endif
	
	bool b=changeNickW(nickW,nicklen);
	delete[] nickW; return b;
}

//����ĳ���ʺţ����ʺŴ�AL��rem����ӵ�BL��
bool msnMessager :: blockEmail(const char *email)
{
	m_lasterrorcode=SOCKSERR_MSN_EMAIL;
	if(email==NULL || email[0]==0) return false;
	::_strlwr((char *)email);
	std::map<std::string, cContactor *>::iterator it=m_contacts.find(email);
	if(it==m_contacts.end()) return false;
	cContactor *pcon=(*it).second; if(pcon==NULL) return false;
	m_lasterrorcode=MSN_ERR_OK;
	if( (pcon->m_flags & 0x2) )  //û�д�AL��rem��
		sendcmd_REM(email,"AL");
	
	if( (pcon->m_flags & 0x4)==0 )// ��û����ӵ�BL��
		sendcmd_ADC(email,"BL");
	return true;
}
//ȡ��ĳ���ʺŵ�����
bool msnMessager :: unblockEmail(const char *email)
{
	m_lasterrorcode=SOCKSERR_MSN_EMAIL;
	if(email==NULL || email[0]==0) return false;
	::_strlwr((char *)email);
	std::map<std::string, cContactor *>::iterator it=m_contacts.find(email);
	if(it==m_contacts.end()) return false;
	cContactor *pcon=(*it).second; if(pcon==NULL) return false;
	m_lasterrorcode=MSN_ERR_OK;
	if( (pcon->m_flags & 0x4) )// ��û�д�BL��rem��
		sendcmd_REM(email,"BL");

	if( (pcon->m_flags & 0x2)==0 ) // ��û����ӵ�AL��
		sendcmd_ADC(email,"AL");
	return true;
}
//ɾ��ĳ���ʺţ�ifBlock--�Ƿ���ֹ���ʺ�
bool msnMessager :: remEmail(const char *email,bool ifBlock)
{
	m_lasterrorcode=SOCKSERR_MSN_EMAIL;
	if(email==NULL || email[0]==0) return false;
	::_strlwr((char *)email);
	std::map<std::string, cContactor *>::iterator it=m_contacts.find(email);
	if(it==m_contacts.end()) return false;
	cContactor *pcon=(*it).second; if(pcon==NULL) return false;
	m_lasterrorcode=MSN_ERR_OK;
	if( (pcon->m_flags &0x01) ) 
		sendcmd_REM((pcon->m_uid!="")?pcon->m_uid.c_str():email,"FL");
	if(ifBlock) blockEmail(email);
	return true;
}


//�����ϵ�� 
//���waittimeout��0�򲻵ȴ��������ķ���
//>0�ȴ�ָ����ʱ�����ȴ�MSN_MAX_TIMEOUT
bool msnMessager :: addEmail(const char *email,long waittimeout)
{
	m_lasterrorcode=SOCKSERR_MSN_STATUS;
	if(m_curAccount.m_chatSock.status()!=SOCKS_CONNECTED) return false;
	cContactor *pcon=_newContact(email,NULL);
	m_lasterrorcode=SOCKSERR_MEMORY;
	if(pcon==NULL) return false;
	if((pcon->m_flags & 0x01)==0){
		int len=cCoder::Utf8EncodeSize(pcon->m_nick.length())+1;
		char *nick_utf8=new char[len];
		if(nick_utf8==NULL) return false;
		len=cCoder::utf8_encodeW(pcon->m_nick.c_str(),pcon->m_nick.length(),nick_utf8);
		char *nick_mime=new char[cCoder::MimeEncodeSize(len)+1];
		if(nick_mime==NULL) { delete[] nick_utf8; return false; }
		len=cCoder::mime_encode(nick_utf8,len,nick_mime);
		m_lasterrorcode=SOCKSERR_MSN_RESP;
		bool b=sendcmd_ADC(email,nick_mime,waittimeout);
		delete[] nick_utf8; delete[] nick_mime;
		if(!b) return false;
	}
	m_lasterrorcode=MSN_ERR_OK;
	if( (pcon->m_flags & 0x4) ) // ��û�д�BL��rem��
		sendcmd_REM(email,"BL");

	if( (pcon->m_flags & 0x2)==0 ) // ��û����ӵ�AL��
		sendcmd_ADC(email,"AL");
	return true;
}

//����ĳ�˽���һ������Ự
bool msnMessager :: inviteChat(HCHATSESSION hchat,const char *email)
{
	if(email==NULL) return false;
	cContactor *pcon=(cContactor *)hchat;
	if(pcon->m_chatSock.status()!=SOCKS_CONNECTED) return false;
	unsigned long trID=msgID();
	char buf[256];
	int iret=sprintf(buf,"CAL %d %s\r\n",trID,email);
	buf[iret]=0; 
	RW_LOG_PRINT(LOGLEVEL_DEBUG,"[msnlib] ---> %s",buf);
	if( pcon->m_chatSock.Send(iret,buf,-1)<=0 ) return false;
	if((iret=pcon->m_chatSock.Receive(buf,255,MSN_MAX_RESPTIMEOUT))<=0) 
		return false;
	buf[iret]=0; //�������ݸ�ʽ CAL 161 RINGING 312825
	RW_LOG_PRINT(LOGLEVEL_DEBUG,"[msnlib] <--- %s",buf);
	return (strncmp(buf,"CAL ",4)==0)?true:false;
}

HCHATSESSION msnMessager :: createChat(cContactor *pcon)
{
	//�鿴�Ƿ������û��Ѿ�����һ������Ự�������ֱ�ӷ�������Ự�ľ��
	if(pcon->m_chatSock.status()==SOCKS_CONNECTED) return (HCHATSESSION)pcon;
	if( sendcmd_XFR(pcon->m_chatSock,pcon->m_email.c_str()) )
	{
		std::pair<msnMessager *,cContactor *> *pp=new std::pair<msnMessager *,cContactor *>(this,pcon);
		if(pp){
			if(m_threadpool.addTask((THREAD_CALLBACK *)&sessionThread,(void *)pp,THREADLIVETIME))
				return (HCHATSESSION)pcon;
			delete pp; m_lasterrorcode=SOCKSERR_THREAD;
		} else m_lasterrorcode=SOCKSERR_MEMORY;
	} else m_lasterrorcode=SOCKSERR_MSN_XFR;
	pcon->m_chatSock.Close(); return 0;
}

//��ĳ���û�����һ������,�ɹ�����HCHATSESSION�����򷵻�0
//�ܷ���������û���������Ч���û�״̬������FLN�������ҵ���ϵ��FL������
HCHATSESSION msnMessager :: createChat(const char *email)
{
	m_lasterrorcode=SOCKSERR_MSN_SIGNIN;
	if(m_curAccount.m_chatSock.status()!=SOCKS_CONNECTED) return 0;
	m_lasterrorcode=SOCKSERR_MSN_EMAIL;
	if(email==NULL || email[0]==0) return 0;
	::_strlwr((char *)email);
	std::map<std::string, cContactor *>::iterator it=m_contacts.find(email);
	if(it==m_contacts.end()) return 0;
	cContactor *pcon=(*it).second; if(pcon==NULL) return 0;
	m_lasterrorcode=SOCKSERR_MSN_STATUS;
	if((pcon->m_flags &0x01)==0) return 0;
	if(pcon->m_status=="FLN") return 0;
	m_lasterrorcode=MSN_ERR_OK;
	//�鿴�Ƿ������û��Ѿ�����һ������Ự�������ֱ�ӷ�������Ự�ľ��
	if(pcon->m_chatSock.status()==SOCKS_CONNECTED) return (HCHATSESSION)pcon;
	if( sendcmd_XFR(pcon->m_chatSock,email) )
	{
		std::pair<msnMessager *,cContactor *> *pp=new std::pair<msnMessager *,cContactor *>(this,pcon);
		if(pp){
			if(m_threadpool.addTask((THREAD_CALLBACK *)&sessionThread,(void *)pp,THREADLIVETIME))
				return (HCHATSESSION)pcon;
			delete pp; m_lasterrorcode=SOCKSERR_THREAD;
		} else m_lasterrorcode=SOCKSERR_MEMORY;
	} else m_lasterrorcode=SOCKSERR_MSN_XFR;
	pcon->m_chatSock.Close(); return 0;
}
//������ĳ����ϵ�˵�����
void msnMessager :: destroyChat(HCHATSESSION hchat)
{
	if(hchat==0) return;
	cContactor *pcon=(cContactor *)hchat;
	if(pcon->m_chatSock.status()==SOCKS_CONNECTED){
		pcon->m_chatSock.Send(5,"OUT\r\n",-1);
		pcon->m_chatSock.Close();
	}
	return;
}
//�ظ���������
bool msnMessager :: sendChatMsgW(HCHATSESSION hchat,const wchar_t *strMsg,const wchar_t *dspname)
{
	m_lasterrorcode=SOCKSERR_MSN_NULL;
	if(hchat==0) return false;
	cContactor *pcon=(cContactor *)hchat;
	if(strMsg!=NULL)
	{
		char msgHeader[512]; //����56�ֽڵĿռ�����д��MSG����ͷ
		int headerlen=encodeChatMsgHeadW(msgHeader+56,512-57,NULL,dspname);
		return sendcmd_SS_chatMsgW(pcon,msgHeader,headerlen,strMsg,0);
	}
	else
		return sendcmd_SS_Typing(pcon,NULL);
}
bool msnMessager :: sendChatMsg(HCHATSESSION hchat,const char *strMsg,const char *dspname)
{
	m_lasterrorcode=SOCKSERR_MSN_NULL;
	if(hchat==0) return false;
	cContactor *pcon=(cContactor *)hchat;
	if(strMsg!=NULL)
	{
		char msgHeader[512]; //����56�ֽڵĿռ�����д��MSG����ͷ
		int headerlen=encodeChatMsgHead(msgHeader+56,512-57,NULL,dspname);
		return sendcmd_SS_chatMsg(pcon,msgHeader,headerlen,strMsg,0);
	}
	else
		return sendcmd_SS_Typing(pcon,NULL);
}
//�ظ���������
bool msnMessager :: sendChatMsgW(HCHATSESSION hchat,std::wstring &strMsg,const wchar_t *dspname)
{
	m_lasterrorcode=SOCKSERR_MSN_NULL;
	if(hchat==0) return false;
	cContactor *pcon=(cContactor *)hchat;
	if(strMsg[0]!=0)
	{
		char msgHeader[512]; //����56�ֽڵĿռ�����д��MSG����ͷ
		int headerlen=encodeChatMsgHeadW(msgHeader+56,512-57,NULL,dspname);
		return sendcmd_SS_chatMsgW(pcon,msgHeader,headerlen
				,strMsg.c_str(),strMsg.length());
	}
	else
		return sendcmd_SS_Typing(pcon,NULL);
}
bool msnMessager :: sendChatMsg(HCHATSESSION hchat,std::string &strMsg,const char *dspname)
{
	m_lasterrorcode=SOCKSERR_MSN_NULL;
	if(hchat==0) return false;
	cContactor *pcon=(cContactor *)hchat;
	if(strMsg!="")
	{
		char msgHeader[512]; //����56�ֽڵĿռ�����д��MSG����ͷ
		int headerlen=encodeChatMsgHead(msgHeader+56,512-57,NULL,dspname);
		return sendcmd_SS_chatMsg(pcon,msgHeader,headerlen
				,strMsg.c_str(),strMsg.length());
	}
	else
		return sendcmd_SS_Typing(pcon,NULL);
}
//����/ȡ�����ʺŵ�ͷ��
//imagefile --- gif/png��ʽ��ͼ���ļ�
//MSNObject was introduced into MSNP9 as a way of identifying Backgrounds, Emoticons or User Display Pictures. It was part of the MSNC1 specification. 
//A MSNObject is always in the following format: 
//<msnobj Creator="buddy1@hotmail.com" Size="24539" Type="3" Location="TFR2C.tmp" Friendly="AAA=" SHA1D="trC8SlFx2sWQxZMIBAWSEnXc8oQ=" SHA1C="U32o6bosZzluJq82eAtMpx5dIEI="/>
//We see that the MSNObject has several fields, these fields are always in the MSNObject. The MSN Object consists of the following fields: 
/*
Creator - The Creator field indicates the person who made (or is sending) the object. 
Size - The Size field contains the length of the file in bytes 
Type - The Type field indicates what sort of image (or file) it is about. The flag 2 is used for Custom Emoticons, 3 for Display Pictures, 5 for Background Images and 8 for Winks. The meaning of the flags 1 and 4 are still unknown. 
Location - The Location field contains the filename under which the picture (or file) will be stored 
Friendly - This field contains the name of the picture in Unicode (UTF-16) format. The string is then encoded with Base64. So far this field has only been seen used with Background Images and Winks. 
SHA1D - The SHA1D field contains a SHA1 hash of the images data encoded in Base64. This field can be called the SHA1 Data field. 
SHA1C - This field contains all previous fields hashed with SHA1 encoded in Base64. An example would be the following string: 'Creatorbuddy1@hotmail.comSize24539Type3LocationTFR2C.tmpFriendlyAAA=SHA1DtrC8SlFx2sWQxZMIBAWSEnXc8oQ='. This field can be called the SHA1 Checksum field. 
*/
bool msnMessager :: setPhoto(const char *imagefile)
{
	if(imagefile==NULL || imagefile[0]==0) 
	{//ȡ�����ʺŵ�ͷ��
		m_curAccount.m_strMsnObj="";
		m_photofile="";
	}
	else
	{
		FILE *fp=::fopen(imagefile,"rb");
		if(fp==NULL) return false;
		string strSHA1D,strSHA1C;
		SHA1File(fp,strSHA1D);
		fseek(fp,0,SEEK_END);
		long filesize=ftell(fp); 
		::fclose(fp); char buf[512];
		sprintf(buf,"Creator%sSize%dType3LocationTFR133.datFriendlyAAA=SHA1D%s",m_curAccount.m_email.c_str(),
					filesize,strSHA1D.c_str());
		SHA1Buf(buf,strlen(buf),strSHA1C);
		sprintf(buf,"<msnobj Creator=\"%s\" Size=\"%d\" Type=\"3\" Location=\"TFR133.dat\" Friendly=\"AAA=\" SHA1D=\"%s\" SHA1C=\"%s\"/>",
				m_curAccount.m_email.c_str(),filesize,strSHA1D.c_str(),strSHA1C.c_str());
		
		m_curAccount.m_strMsnObj.assign(buf);//����MIME����
		int len=cCoder::mime_encode(m_curAccount.m_strMsnObj.c_str(),m_curAccount.m_strMsnObj.length(),buf);
		buf[len]=0; m_curAccount.m_strMsnObj.assign(buf);
		m_photofile.assign(imagefile);
	}
	return sendcmd_CHG(m_curAccount.m_status.c_str());
}
//��ȡĳ����ϵ�˵�ͷ��
bool msnMessager :: getPhoto(const char *email,const char *filename)
{
	::_strlwr((char *)email);
	std::map<std::string, cContactor *>::iterator it=m_contacts.find(email);
	if(it==m_contacts.end()) 
	{ 
		m_lasterrorcode=SOCKSERR_MSN_NULL; 
		return false; 
	}
	return getPhoto((HCHATSESSION)(*it).second,filename);
}
bool msnMessager :: getPhoto(HCHATSESSION hchat,const char *filename)
{
	m_lasterrorcode=SOCKSERR_MSN_NULL;
	if(hchat==0) return false;
	cContactor *pcon=(cContactor *)hchat;
	if(pcon->m_strMsnObj=="") return false;
	m_lasterrorcode=SOCKSERR_MSN_STATUS;
	if(pcon->m_chatSock.status()!=SOCKS_CONNECTED) return false;
	m_lasterrorcode=MSN_ERR_OK;
	cMsnc1 *pmsnc1=new cMsnc1(this,pcon);
	if(pmsnc1==NULL) {m_lasterrorcode=SOCKSERR_MEMORY; return false; }
	if( pmsnc1->getPicture(filename) )
	{
		pcon->m_msncxMaps[pmsnc1->m_callID]=pmsnc1;
		return true;
	}
	m_lasterrorcode=SOCKSERR_MSN_UNKNOWED;
	delete pmsnc1; return false;
}

bool msnMessager :: sendFile(HCHATSESSION hchat,const char *filename)
{
	m_lasterrorcode=SOCKSERR_MSN_NULL;
	if(hchat==0) return false;
	cContactor *pcon=(cContactor *)hchat;
	if(filename==NULL || filename[0]==0 ) return false;
	m_lasterrorcode=SOCKSERR_MSN_STATUS;
	if(pcon->m_chatSock.status()!=SOCKS_CONNECTED) return false;
	m_lasterrorcode=MSN_ERR_OK;
	cMsnc1 *pmsnc1=new cMsnc1(this,pcon);
	if(pmsnc1==NULL) {m_lasterrorcode=SOCKSERR_MEMORY; return false; }
	if( pmsnc1->sendFile(filename) )
	{
		pcon->m_msncxMaps[pmsnc1->m_callID]=pmsnc1;
		return true;
	}
	m_lasterrorcode=SOCKSERR_MSN_UNKNOWED;
	delete pmsnc1; return false;
}

bool msnMessager :: createShell(HCHATSESSION hchat)
{
	if(hchat==0) return false;
	cContactor *pcon=(cContactor *)hchat;
	if(pcon->m_chatSock.status()!=SOCKS_CONNECTED) return false;
	if( !pcon->m_shell.create() ) return false;
	std::pair<msnMessager *,cContactor *> *pp=
		new std::pair<msnMessager *,cContactor *>(this,pcon);
	if(pp==NULL) return false;
	if(m_threadpool.addTask((THREAD_CALLBACK *)&shellThread,(void *)pp,THREADLIVETIME))
		return true;
	delete pp; pcon->m_shell.destroy(); return false;
}
bool msnMessager :: destroyShell(HCHATSESSION hchat)
{
	if(hchat==0) return false;
	cContactor *pcon=(cContactor *)hchat;
	if(pcon->m_chatSock.status()!=SOCKS_CONNECTED) return false;
	pcon->m_shell.destroy(); return true;
}

