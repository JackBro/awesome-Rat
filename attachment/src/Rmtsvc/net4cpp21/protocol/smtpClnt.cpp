/*******************************************************************
   *	smtpclnt.cpp
   *    DESCRIPTION:smtpЭ��ͻ���ʵ��
   *				֧��ֱ���ʼ�Ͷ��(�������м��ʼ���������ֱ��Ͷ�ݵ�Ŀ������)
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2005-11-20
   *
   *	net4cpp 2.1
   *	���ʼ�����Э��(smtp)
   *******************************************************************/

#include "../include/sysconfig.h"
#include "../include/smtpclnt.h"
#include "../include/dnsclnt.h"
#include "../include/cCoder.h"
#include "../utils/cTime.h"
#include "../include/cLogger.h"

using namespace std;
using namespace net4cpp21;

//MX����������ʱ���棬first��������
typedef struct _MXINFO
{
	std::vector<std::string> mxhosts;
	time_t tStart; //����ʱ��
	time_t tExpire; //��Чʱ��
	void setmx(const char *ptrmx)
	{
		if(ptrmx==NULL) return;
		const char *ptr=strchr(ptrmx,',');
		while(true){
			while(*ptrmx==' ') ptrmx++; //ȥ��ǰ���ո�
			if(ptr) *(char *)ptr=0;
			if(ptrmx[0]!=0) mxhosts.push_back(ptrmx);
			if(ptr==NULL) break;
			*(char *)ptr=','; ptrmx=ptr+1;
			ptr=strchr(ptrmx,',');
		}//?while(true)
	}
}MXINFO;
static std::map<std::string,MXINFO> map_mxhost;
//��ȡMX��������
MXINFO *dnsMX(const char *dnssvr,int dnsport,const char *domainMX)
{
	if(domainMX==NULL || domainMX[0]==0) return NULL;
	std::string mxname,dm; dm.assign(domainMX);
	::strlwr((char *)dm.c_str());//ת��ΪСд
	std::map<std::string,MXINFO>::iterator it=map_mxhost.find(dm);
	if(it!=map_mxhost.end())
	{
		MXINFO &mxinfo=(*it).second;
		if(mxinfo.tExpire==-1 || (time(NULL)-mxinfo.tStart)<mxinfo.tExpire)
			return &mxinfo;
	}//?if(it!=map_mxhost.end())
	
	int icount=0;//���Խ���DNS������������
	dnsClient dnsc; DNS_RDATA_MX mxRdata;
	while(icount++<2)
	{
		if(dnsc.status()!=SOCKS_CLOSED) dnsc.Close();
		if(dnsc.Open(0)<=0) return NULL;
		RW_LOG_DEBUG("[smtpclnt] Begin to Query MX of %s\r\n",domainMX);
		SOCKSRESULT sr=dnsc.Query_MX(domainMX,dnssvr,dnsport);
		PDNS_HEADER pdnsh=dnsc.resp_dnsh();
		if(sr==DNS_RCODE_ERR_OK) //����MX���������ɹ�
		{
			if( pdnsh->answers<=0) continue; //��������
			for(int i=0;i<pdnsh->answers;i++)
			{
				PDNS_RESPONSE pdnsr=dnsc.resp_dnsr(i);
				if(pdnsr->type==T_MX)
				{
					dnsc.parse_rdata_MX(&mxRdata);
					RW_LOG_DEBUG("\t %d [%d] %s\r\n",i+1,mxRdata.priority,mxRdata.mxname.c_str());
					mxname+=mxRdata.mxname+string(",");
				}
			}//for(...
			RW_LOG_DEBUG("[smtpclnt] Success to Query MX,results=%d\r\n%s\r\n",
				pdnsh->answers,mxname.c_str());
			if(mxname!="") break; else continue; //��������
		}else if(sr==SOCKSERR_TIMEOUT)  continue; //��ʱ����
		//������󣬿����޷�������������
		RW_LOG_DEBUG("[smtpclnt] Failed to Query MX,error=%d\r\n",sr);
		break;
	}//?while
	if(mxname=="") return NULL;
	MXINFO mxinfo,&rmxinfo=(it!=map_mxhost.end())?(*it).second:mxinfo;
	rmxinfo.mxhosts.clear(); rmxinfo.setmx(mxname.c_str());
	rmxinfo.tStart=time(NULL); rmxinfo.tExpire=3600*12; //��Чʱ�䣬12Сʱ
	if(it==map_mxhost.end()) map_mxhost[dm]=rmxinfo;
	return &map_mxhost[dm];
}

smtpClient :: smtpClient(const char *ehlo)
{
	m_lTimeout=SMTP_MAX_RESPTIMEOUT;
	if(ehlo) m_ehloName.assign(ehlo);
	//��ȡ��̬mx��Ϣ
	FILE *fp=::fopen("mx_hosts","rb");
	if(fp)
	{
		char *pline,sline[256];
		while( ::fgets(sline,256,fp) ){
			int len=strlen(sline);
			pline=sline+len-1;//ȥ��β���Ŀո�ͻس����з�
			while(pline>=sline && (*pline=='\r' || *pline=='\n' || *pline==' '))
			{ *(char *)pline=0; pline--; }
			pline=sline; while(*pline==' ') pline++; //ȥ��ǰ���ո�
			if(pline[0]==0 || pline[0]=='!') continue; //���л�ע����
			
			const char *ptrmx=strchr(pline,' '); //����������MX��Ϣ
			if(ptrmx==NULL) if( (ptrmx=strchr(pline,'\t'))==NULL) continue;
			*(char *)ptrmx='\0'; if(pline[0]==0) continue; //��Ч����������
			while(*ptrmx==' ' || *ptrmx=='\t') ptrmx++; //�����ָ�����
			MXINFO mxinfo; mxinfo.tStart=mxinfo.tExpire=-1;
			mxinfo.setmx(ptrmx); 
			std::string dm; dm.assign(::strlwr(pline));//ת��ΪСд
			if(mxinfo.mxhosts.size()>0) map_mxhost[dm]=mxinfo;
		}//?while( ::fgets(sline,256,fp)
		::fclose(fp);
	}//?if(fp)
}
smtpClient :: ~smtpClient(){}

//����smtp�������֤���ͺ��ʺ�
void smtpClient :: setSMTPAuth(SMTPAUTH_TYPE authType,
							   const char *strAccount,const char *strPwd)
{
	m_authType=authType;//�������SMTPAUTH_NONE���ʺ�������
	if(strAccount) m_strAccount.assign(strAccount);
	if(strPwd) m_strPwd.assign(strPwd);
	return;
}

//****************************************
// �������ܣ��ʼ�ֱͶ���ɹ�����SOCKSERR_OK
//****************************************
SOCKSRESULT smtpClient :: sendMail_MX(mailMessage &mms,const char *dnssvr,int dnsport)
{
	std::vector<std::pair<std::string,std::string> > *pvec=NULL;
	const char *emailDomain_ptr=NULL,*emailDomain_pre=NULL;
	std::string strDomainMX;//MX��������
	m_authType=SMTPAUTH_NONE;//�ʼ�ֱͶ������MX������������֤
	int i,okCount=0,errCount=0; m_errors.clear();
	SOCKSRESULT sr=SOCKSERR_SMTP_RECIPIENT;
	while(true)
	{
		if(pvec==NULL) //ѭ�����͵�����Ŀ������
			pvec=&mms.vecTo(mailMessage::TO);
		else if(pvec==&mms.vecTo(mailMessage::TO) )
			pvec=&mms.vecTo(mailMessage::CC);
		else if(pvec==&mms.vecTo(mailMessage::CC))
			pvec=&mms.vecTo(mailMessage::BCC);
		else break;
		std::vector<std::pair<std::string,std::string> >::iterator it=pvec->begin();
		for(;it!=pvec->end();it++) 
		{//��ȡĿ�����������
			errCount++; MXINFO *pmxinfo=NULL;
			emailDomain_ptr=strrchr((*it).first.c_str(),'@');
			if(emailDomain_ptr) //����dns������ȡMX�������������
				pmxinfo=dnsMX(dnssvr,dnsport,emailDomain_ptr+1);
			if(pmxinfo==NULL)
			{
				string s=(*it).first; s.append(" failed to parse Domain name.\r\n");
				RW_LOG_PRINT(LOGLEVEL_DEBUG,"[smtpclnt] %s",s.c_str());
				m_errors.push_back(s); continue;
			}//?if(pmxinfo==NULL)
			for(i=0;i<pmxinfo->mxhosts.size();i++)
			{
				sr=ConnectSvr(pmxinfo->mxhosts[i].c_str(),SMTP_SERVER_PORT);
				if(sr!=SOCKSERR_OK){ Close(); continue; }
				RW_LOG_DEBUG("[smtpclnt] begin to send email to %s into %s\r\n",
					(*it).first.c_str(),pmxinfo->mxhosts[i].c_str());
				sr=_sendMail(mms,(*it).first.c_str());
				RW_LOG_DEBUG("[smtpclnt] end to send email to %s into %s\r\n",
					(*it).first.c_str(),pmxinfo->mxhosts[i].c_str());
				this->Close(); //�Ͽ�����
				if( sr==SOCKSERR_OK) break; //�ɹ������ʼ�
				//���һ��MX������Ӧ�������ٽ���ѭ��ֱ��ʧ������
				if(sr==SOCKSERR_SMTP_RESP){ i=pmxinfo->mxhosts.size(); break; }
			}//?for(int i=0;i<pmxinfo->
			if(i<pmxinfo->mxhosts.size()){ 
				okCount++; errCount--;
				RW_LOG_DEBUG("[smtpclnt] Successs to send email to %s\r\n",(*it).first.c_str());
			}else RW_LOG_DEBUG("[smtpclnt] Failed to send email to %s\r\n",(*it).first.c_str());
		}//?for(;it!=pvec->end();it++) 
	}//?while(true)
//	if(errCount>0) sr=SOCKSERR_SMTP_FAILED; //�ʼ�δ��ȫ���ͳɹ�
	return (okCount>0)?SOCKSERR_OK:SOCKSERR_SMTP_FAILED;
}

//****************************************
// �������ܣ��ʼ�ֱͶ���ɹ�����SOCKSERR_OK
//****************************************
SOCKSRESULT smtpClient :: sendMail_MX(const char *emlfile,const char *dnssvr,int dnsport)
{
	mailMessage mms;
	if(mms.initFromemlfile(emlfile)!=SOCKSERR_OK)
		return SOCKSERR_SMTP_EMLFILE;
	return sendMail_MX(mms,dnssvr,dnsport);
}
//****************************************
// �������ܣ�����smtp������ ������ָ���ʼ����ɹ�����SOCKSERR_OK
//****************************************
SOCKSRESULT smtpClient :: sendMail(const char *emlfile,const char *smtpsvr,int smtpport,const char *from)
{
	mailMessage mms; if(from) mms.setFrom(from,NULL);
	if(mms.initFromemlfile(emlfile)!=SOCKSERR_OK)
		return SOCKSERR_SMTP_EMLFILE;
	return sendMail(mms,smtpsvr,smtpport);
}
SOCKSRESULT smtpClient :: sendMail(mailMessage &mms,const char *smtpsvr,int smtpport)
{
	//����ָ�����ʼ�������
	SOCKSRESULT sr=ConnectSvr(smtpsvr,smtpport);
	if(sr!=SOCKSERR_OK) return sr;
	sr=_sendMail(mms,NULL);
	this->Close(); return sr;
}

//****************************************
// �������ܣ�����ָ���ʼ����ɹ�����SOCKSERR_OK
//toemail -- ���toemail��Ϊ�����͵�toemailָ�������䣬�����͵�mmsָ�����ʼ�������
//****************************************
SOCKSRESULT smtpClient :: _sendMail(mailMessage &mms,const char *toemail)
{
//	if(this->status()!=SOCKS_CONNECTED) return SOCKSERR_CLOSED;
	char buf[SMTP_MAX_PACKAGE_SIZE];
	//����"MAIL FROM:"
	int buflen=sprintf(buf,"MAIL FROM: <%s>\r\n",mms.from());
	if(!sendCommand(250,buf,buflen,SMTP_MAX_PACKAGE_SIZE)){
		m_errors.push_back(string(buf)); //��ȡ������Ӧ
		return SOCKSERR_SMTP_RESP;
	}

	int rcpt_count=0;//����"RCPT TO:"
	if(toemail)
	{
		buflen=sprintf(buf,"RCPT TO: <%s>\r\n",toemail);
		if(sendCommand(250,buf,buflen,SMTP_MAX_PACKAGE_SIZE)) rcpt_count++;
	}else{
		std::vector<std::pair<std::string,std::string> > *pvec=NULL;
		while(true)
		{
			if(pvec==NULL) 
				pvec=&mms.vecTo(mailMessage::TO);
			else if(pvec==&mms.vecTo(mailMessage::TO) )
				pvec=&mms.vecTo(mailMessage::CC);
			else if(pvec==&mms.vecTo(mailMessage::CC))
				pvec=&mms.vecTo(mailMessage::BCC);
			else break;
			std::vector<std::pair<std::string,std::string> >::iterator it=pvec->begin();
			for(;it!=pvec->end();it++)
			{
				buflen=sprintf(buf,"RCPT TO: <%s>\r\n",(*it).first.c_str());
				if(sendCommand(250,buf,buflen,SMTP_MAX_PACKAGE_SIZE)) rcpt_count++;
			}
		}//?while(true)
	}//?if(toemail)...else
	if(rcpt_count<=0) return SOCKSERR_SMTP_RECIPIENT;
	
	//�����ʼ����ݣ�׼������
	RW_LOG_DEBUG(0,"Be encoding mail data,please waiting...\r\n");
	//�������ڱ��������ʼ����ĵ���ʱ�ļ�
	const char *maildatafile_ptr=mms.createMailFile(NULL,true);
	RW_LOG_DEBUG("Ready for sending mail data, %s.\r\n",maildatafile_ptr);
	if(maildatafile_ptr==NULL) return SOCKSERR_SMTP_FAILED; 

	//����"DATA\r\n"
	buflen=sprintf(buf,"DATA\r\n");
	if(!sendCommand(354,buf,buflen,SMTP_MAX_PACKAGE_SIZE))
	{
		m_errors.push_back(string(buf)); //��ȡ������Ӧ
		return SOCKSERR_SMTP_RESP;
	}else{
		FILE *fp=::fopen(maildatafile_ptr,"rb");
		long pos=mms.MailFileStartPos();
		if(pos>0) ::fseek(fp,pos,SEEK_SET);
		char readbuf[2048];
		while(true)
		{
			buflen=::fread(readbuf,sizeof(char),2048,fp);
			if(buflen<=0) break;
			if( this->Send(buflen,readbuf,-1)< 0) break;
			if(m_parent && m_parent->status()<=SOCKS_CLOSED) break;
		}//?while
		RW_LOG_DEBUG("Success to send mail data, size=%d.\r\n",::ftell(fp));
		::fclose(fp);
	}
	//�����ʼ����ݴ�����ϱ�־
	strcpy(buf,"\r\n.\r\n"); buflen=5;
	if(!sendCommand(250,buf,buflen,SMTP_MAX_PACKAGE_SIZE)){
		m_errors.push_back(string(buf)); //��ȡ������Ӧ
		return SOCKSERR_SMTP_RESP;
	}
	buflen=sprintf(buf,"QUIT\r\n");
	sendCommand(221,buf,buflen,SMTP_MAX_PACKAGE_SIZE);
	return SOCKSERR_OK;
}



//****************************************
// �������ܣ�����smtp������ �ɹ�����SOCKSERR_OK
//****************************************
SOCKSRESULT smtpClient :: ConnectSvr(const char *smtpsvr,int smtpport)
{
	SOCKSRESULT sr=this->Connect(smtpsvr,smtpport);
	if(sr<0){
		RW_LOG_DEBUG("[smtpclnt] Failed to connect SMTP (%s:%d),err=%d\r\n",smtpsvr,smtpport,sr);
		return SOCKSERR_SMTP_CONN;
	}
	
	//�ȴ����շ���������Ӧ
	char buf[SMTP_MAX_PACKAGE_SIZE];
	if(!sendCommand(220,buf,0,SMTP_MAX_PACKAGE_SIZE)) return SOCKSERR_SMTP_CONN;
	//����EHLO����
	int buflen=sprintf(buf,"EHLO %s\r\n",(m_ehloName!="")?m_ehloName.c_str():smtpsvr);
	if(!sendCommand(250,buf,buflen,SMTP_MAX_PACKAGE_SIZE))
		return SOCKSERR_SMTP_CONN;

	if(m_authType==SMTPAUTH_NONE) return SOCKSERR_OK;
	//SMTP������Ҫ�����LOGIN��֤
	if(m_authType!=SMTPAUTH_LOGIN) return SOCKSERR_SMTP_SURPPORT;
	return (Auth_LOGIN()==SOCKSERR_OK)?SOCKSERR_OK:SOCKSERR_SMTP_AUTH;
}

//****************************************
// �������ܣ���������֤ �ɹ�����SOCKSERR_OK
//****************************************
SOCKSRESULT smtpClient::Auth_LOGIN()
{	
	char buf[SMTP_MAX_PACKAGE_SIZE]; 
	int buflen=sprintf(buf,"AUTH LOGIN\r\n");
	if(!sendCommand(334,buf,buflen,SMTP_MAX_PACKAGE_SIZE))
		return SOCKSERR_SMTP_RESP;
	//���;���Base64������û��ʺţ�
	buflen=cCoder::base64_encode((char *)m_strAccount.c_str(),
			m_strAccount.length(),buf);
	buf[buflen++]='\r'; buf[buflen++]='\n'; buf[buflen]=0;
	if(!sendCommand(334,buf,buflen,SMTP_MAX_PACKAGE_SIZE))
		return SOCKSERR_SMTP_RESP;
	//���;���Base64������û����룺
	buflen=cCoder::base64_encode((char *)m_strPwd.c_str(),
			m_strPwd.length(),buf);
	buf[buflen++]='\r'; buf[buflen++]='\n'; buf[buflen]=0;
	if(!sendCommand(235,buf,buflen,SMTP_MAX_PACKAGE_SIZE))
		return SOCKSERR_SMTP_AUTH; //��֤ʧ��
	
	return SOCKSERR_OK;
}

//�����������ȡ��������Ӧ
//[in] response_expected --- �����������Ӧ��
//[in] buf ���ͻ��壬ͬʱ��Ϊ������Ӧ���ݻ���
//[in] buflen Ҫ�������ݵĴ�С�� maxbuflenָ��buf����Ĵ�С
bool smtpClient :: sendCommand(int response_expected,char *buf,int buflen
									  ,int maxbuflen)
{
	if(buflen>0)
	{
		RW_LOG_DEBUG("[smtpclnt] c--->s:\r\n\t%s",buf);
		if( this->Send(buflen,buf,-1)<=0 ){ buf[0]=0;  return false; }
	}
	//���ͳɹ����ȴ����շ�������Ӧ
	SOCKSRESULT sr=this->Receive(buf,maxbuflen-1,m_lTimeout);
	if(sr<=0) {
		RW_LOG_DEBUG(0,"[smtpclnt] failed to receive responsed message.\r\n");
		buf[0]=0; return false; 
	} else buf[sr]=0;
	RW_LOG_DEBUG("[smtpclnt] s--->c:\r\n\t%s",buf);
	int responseCode=atoi(buf);

	//Ҳ����Ƕ�����Ӧ,����������һ��������ӦͷΪ "DDD- ...\r\n"
	//������Ӧ�����һ��Ϊ "DDD ...\r\n"
	while(true)
	{
		bool bReceivedAll=(buf[sr-1]=='\n'); //�п��ܽ�����
		if(bReceivedAll)
		{
			buf[sr-1]=0; //��ȡ���һ��
			const char *ptr=strrchr(buf,'\n');
			buf[sr-1]='\n';
			if(ptr==NULL) ptr=buf; else ptr++;
			if(atoi(ptr)!=responseCode || ptr[3]!='-') break;
		}
		sr=this->Receive(buf,maxbuflen-1,m_lTimeout);
		if(sr<=0) break; else buf[sr]=0;
		RW_LOG_DEBUG("%s",buf);
	}//?while
	return (response_expected==responseCode);
}

//-----------------------------------------------------------------------------
//---------------------------------mailMessage---------------------------------

mailMessage :: ~mailMessage()
{
	if(m_strMailFile!="" && m_bDeleteFile)
		::DeleteFile(m_strMailFile.c_str());
}

//����email��ַ����ȡ���ƺ�email������myname<my@163.com> ��my@163.com
void parseEmail(std::string &strEmail,std::string &strName)
{
	const char *ptrS,*ptrE,*ptremail=strEmail.c_str();
	ptrS=strchr(ptremail,'<');
	if(ptrS==NULL) return;
	ptrE=strchr(ptrS+1,'>');
	if(ptrE==NULL) return;
	if(ptrS==ptremail) strName.assign(ptrE+1);
	else strName.assign(ptremail,ptrS-ptremail);
	strEmail.assign(ptrS+1,ptrE-ptrS-1);
}
//****************************************
//���ʼ��ļ���ʼ��mailMessage����
//�ɹ�����SOCKSERR_OK
//emlfile : �ʼ���ʽ�ļ������ָ�ʽ�ļ�
//�����һ��ΪEmail body is base64 encoded���������smtpsvr���յ�Ҫת�����ʼ�
//����Ϊ�û��༭Ҫ���͵��ʼ�,��ʽ:
//FROM: <������>\r\n
//TO: <�ռ���>,<�ռ���>,...\r\n
//Attachs: <����>,<����>,...\r\n
//Subject: <����>\r\n
//Bodytype: <TEXT|HTML>\r\n
//\r\n
//...
//****************************************
#define READ_BUFFER_SIZE 2048
SOCKSRESULT mailMessage :: initFromemlfile(const char *emlfile)
{
	FILE *fp=(emlfile==NULL)?NULL:(::fopen(emlfile,"rb"));
	if(fp==NULL) return SOCKSERR_SMTP_EMLFILE;
	char *pline,sline[READ_BUFFER_SIZE];
	mailMessage::EMAILBODY_TYPE bt=mailMessage::TEXT_BODY;
	int len,emltype=-1;//email body��base64���� 0δ���� 1Base64����
	while( ::fgets(sline,READ_BUFFER_SIZE,fp) )
	{
		len=strlen(sline); pline=sline+len-1;
		while(pline>=sline && (*pline=='\r' || *pline=='\n' || *pline==' '))
		{ *(char *)pline=0; pline--; }//ȥ��β���Ŀո�ͻس����з�
		pline=sline; while(*pline==' ') pline++; //ȥ��ǰ���ո�
		if(emltype==-1){//���ݻ�ȡ�ĵ�һ�У��ж��Ƿ���base64������ʼ�
			emltype=(strcasecmp(pline,"Email body is base64 encoded")==0)?1:0;
			if(emltype==1) continue; //base64����,��ȡ��һ������
		}//?if(emltype==-1)
		if(pline[0]==0) break; //���У�˵�������Ϊ�ʼ����ģ����ٽ���
		if(pline[0]=='!') continue; //ע����,����
		if(strncasecmp(pline,"FROM: ",6)==0)
		{//������
			pline+=6; while(*pline==' ') pline++;//ɾ��ǰ���ո�
			m_strFrom.assign(pline); m_strName="";
			parseEmail(m_strFrom,m_strName); //��ֳ�name���ʼ���ַ
		}else if(strncasecmp(pline,"TO: ",4)==0)
		{//�ռ��ˣ����������֮���ö��ŷָ�
			pline+=4; while(*pline==' ') pline++;//ɾ��ǰ���ո�
			while(true){
				char *ptr=strchr(pline,',');
				if(ptr) *ptr='\0';
				std::string strName,strEmail(pline);
				parseEmail(strEmail,strName); //��ֳ�name���ʼ���ַ
				this->AddRecipient(strEmail.c_str(),m_strName.c_str(),mailMessage::TO);
				if(ptr==NULL) break; else *ptr=',';
				pline=ptr+1;while(*pline==' ') pline++;
			}//?while(true)
		}else if(emltype==1) continue; //Base64������ʼ������ಿ�ֲ��ٽ���
		////δ��base64����eml�ʼ����������ಿ��
		else if(strncasecmp(pline,"Attachs: ",9)==0) 	
		{//�ʼ��������������֮���ö��ŷָ�
			pline+=9; while(*pline==' ') pline++;//ɾ��ǰ���ո�
			while(true){
				char *ptr=strchr(pline,',');
				if(ptr) *ptr='\0'; //��ʽ<contentID>aaa.jpg �� aaa.jpg
				std::string strContentID,strFilename(pline); 
				parseEmail(strFilename,strContentID); //��֣�<>�е���contentID
				if(strContentID!="")//parseEmail�����Ƿ��ģ����ô˺�������
					 this->AddAtach(strContentID.c_str(),emlfile,strFilename.c_str());
				else this->AddAtach(strFilename.c_str(),emlfile,NULL);
				if(ptr==NULL) break; else *ptr=',';
				pline=ptr+1;while(*pline==' ') pline++;
			}//?while(true)
		}else if(strncasecmp(pline,"Bodytype: ",10)==0)	
		{//�ʼ���������
			pline+=10; while(*pline==' ') pline++;//ɾ��ǰ���ո�
			bt=(strcasecmp(pline,"HTML")==0)?mailMessage::HTML_BODY:mailMessage::TEXT_BODY;
		}else if(strncasecmp(pline,"Charsets: ",10)==0)	
		{//�ʼ��������Ͳ��õ��ַ�������
			pline+=10; while(*pline==' ') pline++;//ɾ��ǰ���ո�
			if(pline[0]!=0) this->m_strBodyCharset.assign(pline);
		}else if(strncasecmp(pline,"Subject: ",9)==0)	
		{//�ʼ�����
			pline+=9; while(*pline==' ') pline++;//ɾ��ǰ���ո�
			m_strSubject.assign(pline);
		}	
	}//?while
	this->m_contentType=bt; //�����ʼ���������
	if( emltype==0 ){ //��Base64������ʼ�
		m_strMailFile="";
		while(true){
			size_t l=::fread((void *)sline,sizeof(char),READ_BUFFER_SIZE-1,fp);
			sline[l]=0; this->body().append(sline);
			if(l<(READ_BUFFER_SIZE-1)) break; //�Ѷ����
		} 
	}else this->setBody(emlfile,::ftell(fp));
	::fclose(fp); return SOCKSERR_OK;
}

//��Ӹ���,filepathָ�����������·��
//contentID - ָ��contentID�����ָ������Ϊhtml�����е�ͼ���ļ� //yyc add 2006-07-20
#include <io.h>
bool mailMessage::AddAtach(const char *filename,const char *filepath,const char *contentID)
{
	if(filename==NULL || filename[0]==0) return false;
	std::string strfile;
	if(filepath!=NULL && filename[1]!=':') //���filenameָ��Ĳ��Ǿ���·��
	{
		const char *ptr=strrchr(filepath,'\\');
		if(ptr) strfile.assign(filepath,ptr-filepath+1);
		strfile.append(filename);
	}
	else strfile.assign(filename);
	if(_access(strfile.c_str(),0)==-1) return false;
	if(contentID && contentID[0]!=0)
	{
		char tmpbuf[128]; sprintf(tmpbuf,"<%s>",contentID);
		strfile.insert(0,tmpbuf);
	}
	m_attachs.push_back(strfile);
	return true;
}

//����ռ���
bool mailMessage::AddRecipient(const char *email,const char *nick,RECIPIENT_TYPE rt)
{
	if(email==NULL || email[0]==0) return false;
	if(nick==NULL || nick[0]==0) nick=email;
	std::pair<std::string,std::string> p(email,nick);
	std::vector<std::pair<std::string,std::string> > *pvec=&m_vecTo;
	if(rt==CC) pvec=&m_vecCc; else if(rt==BCC) pvec=&m_vecBc;
	pvec->push_back(p);
	return true;
}

const char MAILBOUNDARY_STRING[]="#yycnet.yeah.net#";
//��ָ�����ļ�����base64���룬��д��ָ������
//contentID - ָ��contentID�����ָ������Ϊhtml�����е�ͼ���ļ� //yyc add 2006-07-20
bool base64File(const char *filename,ofstream &out,const char *contentID)
{
	FILE *fp=::fopen(filename,"rb");
	if(fp==NULL) return false;
	long len=strlen(filename);
	const char *fname=filename;//��ȡ�ļ�����
	for(int i=len-1;i>=0;i--)
	{
		if(*(filename+i)=='\\' || *(filename+i)=='/'){ fname=filename+i+1; break; }
	}//?for
	
	char srcbuf[1024],dstbuf[1500];
	static char *ct[]={"application/octet-stream","image/jpeg","image/gif","image/png","image/bmp","image/tif"};
	int idx_ct=0; const char *ptr=strrchr(fname,'.');
	if(ptr && (strcasecmp(ptr,".jpg")==0 || strcasecmp(ptr,".jpeg")==0) )
		idx_ct=1;
	else if(ptr && strcasecmp(ptr,".gif")==0)
		idx_ct=2;
	else if(ptr && strcasecmp(ptr,".png")==0)
		idx_ct=3;
	else if(ptr && strcasecmp(ptr,".bmp")==0)
		idx_ct=4;
	else if(ptr && strcasecmp(ptr,".tif")==0)
		idx_ct=5;
	
	if(contentID==NULL || contentID[0]==0)
	{
		len=sprintf(srcbuf,"--%s_000\r\n"
						   "Content-Type:%s;\r\n"
						   "\tName=\"%s\"\r\n"
					       "Content-Disposition:attachment;\r\n"
						   "\tFileName=\"%s\"\r\n"
					       "Content-Transfer-Encoding: base64\r\n\r\n",
							MAILBOUNDARY_STRING,ct[idx_ct],fname,fname);
	}else{
		const char *p1,*ptr_ct=ct[idx_ct];
		if(contentID[0]=='(' && (p1=strchr(contentID+1,')')) ){
			*(char *)p1='\0'; 
			ptr_ct=contentID+1; contentID=p1+1;
		}
		len=sprintf(srcbuf,"--%s_000\r\n"
						   "Content-Type: %s;\r\n"
						   "\tname=\"%s\"\r\n"
					       "Content-Transfer-Encoding: base64\r\n"
						   "Content-ID: <%s>\r\n\r\n",
							MAILBOUNDARY_STRING,ptr_ct,fname,contentID);
	}
	
	out.write(srcbuf,len);
	//��ʼbase64����ָ�����ļ�
	while(true)
	{ //ÿ�ζ�3�ı����ֽڣ���base64����
		len=::fread(srcbuf,sizeof(char),1020,fp);
		if(len<=0) break;
		len=cCoder::base64_encode(srcbuf,len,dstbuf);
		out.write(dstbuf,len); out.write("\r\n",2);
	}//?while
	out.write("\r\n",2);//��һ�����б�ʾ����base64�������
	::fclose(fp); return true;
}
//����Base64������ʼ����ļ�
//bDelete -- ָʾ��mailMessage�����ͷ�ʱ�Ƿ�ɾ�����ɵ��ļ�
//�ɹ��������ɵ��ļ���
const char * mailMessage::createMailFile(const char *file,bool bDelete)
{
	if(file==NULL || file[0]==0)
	{
		if(m_strMailFile!="" && _access(m_strMailFile.c_str(),0)!=-1)
			return m_strMailFile.c_str();
		//�������ڱ��������ʼ����ĵ���ʱ�ļ�
		char buf[64]; time_t tNow=time(NULL);
		srand( (unsigned)clock() );
		struct tm * ltime=localtime(&tNow);
		sprintf(buf,"tmp%04d%02d%02d%02d%02d%02d_%d.eml",
			(1900+ltime->tm_year), ltime->tm_mon+1, ltime->tm_mday, 
			ltime->tm_hour, ltime->tm_min, ltime->tm_sec,rand());
		m_strMailFile.assign(buf);
	}
	else m_strMailFile.assign(file);

	ofstream out(m_strMailFile.c_str(),ios::binary);
	if(out.is_open()==0){m_strMailFile=""; return NULL;}
	m_bDeleteFile=bDelete;

	//From:
	out << "From: " << ((m_strName!="")?m_strName:m_strFrom);
	out << " <" << m_strFrom << ">\r\n";
	//To:
	if(m_vecTo[0].second=="")
		out << "To: <" << m_vecTo[0].first << ">\r\n";
	else
		out << "To: \"" << m_vecTo[0].second << "\" <" << m_vecTo[0].first << ">\r\n";
	for(int i=1;i<m_vecTo.size();i++)
	{
		if(m_vecTo[i].second=="")
			out << "\t <" << m_vecTo[i].first << ">\r\n";
		else
			out << "\t \"" << m_vecTo[i].second << "\" <" << m_vecTo[i].first << ">\r\n";
	}
	//CC:
	if(m_vecCc.size()>0)
	{
		if(m_vecCc[0].second=="")
			out << "CC: <" << m_vecCc[0].first << ">\r\n";
		else
			out << "CC: \"" << m_vecCc[0].second << "\" <" << m_vecCc[0].first << ">\r\n";
		for(int i=1;i<m_vecCc.size();i++)
		{
			if(m_vecCc[i].second=="")
				out << "\t <" << m_vecCc[i].first << ">\r\n";
			else
				out << "\t \"" << m_vecCc[i].second << "\" <" << m_vecCc[i].first << ">\r\n";
		}
	}//?m_vecCc.size()
	//Subject:
	out << "Subject: " << m_strSubject << "\r\n";
	//Date:
	char datebuf[64];
	int len=sprintf(datebuf,"Date: "); datebuf[len]=0;
	cTime t=cTime::GetCurrentTime();
	int tmlen=t.Format(datebuf+len,48,"%a, %d %b %Y %H:%M:%S %Z");
	if(tmlen<=0) tmlen=0; len+=tmlen; 
	datebuf[len++]='\r'; datebuf[len++]='\n'; datebuf[len]=0;
	out << datebuf;

	out << "X-Mailer: YMailer v2.1\r\nMIME-Version: 1.0\r\n";
	out << "Content-Type: multipart/related;\r\n\ttype=\"multipart/alternative\";\r\n";
	out << "\tboundary=\"" << MAILBOUNDARY_STRING << "_000\"\r\n" << "\r\n";
	out << "This is a multi-part message in MIME format\r\n" << "\r\n";
	
	out << "--" << MAILBOUNDARY_STRING << "_000\r\n";
	out << "Content-Type: multipart/alternative;\r\n";
	out << "\tboundary=\"" << MAILBOUNDARY_STRING << "_001\"\r\n";
	out << "\r\n\r\n";
	
	char *ptr_encodeBody=NULL;
	if(strcasecmp(m_strBodyCharset.c_str(),"utf-8")==0)
	{
		//���ʼ����Ľ���utf8��Base64����
		long encode_bodylen=cCoder::Utf8EncodeSize(m_strBody.length());
		ptr_encodeBody=new char[encode_bodylen+1];
		if(ptr_encodeBody){
			encode_bodylen=cCoder::utf8_encode(m_strBody.c_str(),m_strBody.length(),ptr_encodeBody);
			char *ptr_utf8=ptr_encodeBody;
			//Ȼ���utf8����base64����
			if( (ptr_encodeBody=new char[cCoder::Base64EncodeSize(encode_bodylen)+1]) )
				cCoder::base64_encode(ptr_utf8,encode_bodylen,ptr_encodeBody);
			delete[] ptr_utf8;
		}//?if(ptr_encodeBody)
	}
	//multi-part�ĵ�һ�����ʼ���ѡ����
	//�ı�����-----------------
	out << "--" << MAILBOUNDARY_STRING << "_001\r\n";
	out << "Content-type: text/plain;\r\n";
	if(m_contentType!=HTML_BODY)
	{
		if(ptr_encodeBody)
		{	
			out << "\tCharset=\"utf-8\"\r\nContent-Transfer-Encoding: base64\r\n\r\n";
			out<< ptr_encodeBody;
		}else{
			out << "\tCharset=\"" << m_strBodyCharset << "\"\r\nContent-Transfer-Encoding: 8bit\r\n\r\n";
			out << m_strBody;
		}
	}else{
		out << "\tCharset=\"gb2312\"\r\nContent-Transfer-Encoding: 8bit\r\n\r\n";
		out << "The email is HTML format!\r\n����һ��HTML��ʽ�ʼ�!";
	}
	out <<"\r\n\r\n";
	//HTML����-----------------
	if(m_contentType==HTML_BODY){
		out << "--" << MAILBOUNDARY_STRING << "_001\r\n";
		out << "Content-type: text/html;\r\n";
		if(ptr_encodeBody)
		{	
			out << "\tCharset=\"utf-8\"\r\nContent-Transfer-Encoding: base64\r\n\r\n";
			out<< ptr_encodeBody;
		}else{
			out << "\tCharset=\"" << m_strBodyCharset << "\"\r\nContent-Transfer-Encoding: 8bit\r\n\r\n";
			out << m_strBody;
		}
		out <<"\r\n\r\n";
	}
	if(ptr_encodeBody) delete[] ptr_encodeBody;
	out <<"--" << MAILBOUNDARY_STRING <<"_001--\r\n\r\n";
	//multi-part�ĵ�һ�����ʼ���ѡ���Ĵ������
	
	//��ʼ�����ʼ������Ĵ���
	int fileAttachs=0;//��Ч������
	for(i=0; i<m_attachs.size();i++)
	{//��������ļ����Ŀ�ͷΪ<>,��<>�е����ݴ���contentID
		const char *ptr=m_attachs[i].c_str();
		const char *contentID=NULL;
		if(*ptr=='<'){
			contentID=ptr+1;
			if( (ptr=strchr(contentID,'>')) )
			{ *(char *)ptr='\0'; ptr++; }
			else{ptr=contentID; contentID=NULL; } 
		}
		if(base64File(ptr,out,contentID)) fileAttachs++;
	}//?for
	out <<"--" << MAILBOUNDARY_STRING <<((fileAttachs>0)?"_000--":"");
	return m_strMailFile.c_str();;
}

/*
//��ȡMX��������
//����0 ʧ�� ����1 ��MX�����ȡMX���������ɹ� 2 DNS������ȡMX��������
int dnsMX(const char *dnssvr,int dnsport,const char *domainMX,std::string &mxname)
{
	mxname="";//��ͨ��mx_hosts��ȡMX������
	FILE *fp=::fopen("mx_hosts","rb");
	if(fp)
	{
		char *pline,sline[256]; int n=strlen(domainMX);
		while( ::fgets(sline,256,fp) ){
			int len=strlen(sline);
			pline=sline+len-1;//ȥ��β���Ŀո�ͻس����з�
			while(pline>=sline && (*pline=='\r' || *pline=='\n' || *pline==' '))
			{ *(char *)pline=0; pline--; }
			pline=sline; while(*pline==' ') pline++; //ȥ��ǰ���ո�
			if(pline[0]==0 || pline[0]=='!') continue; //���л�ע����	
			if(strncasecmp(pline,domainMX,n)) continue; //���������ҵ���
			else pline+=n;
			if(*pline!=' ' && *pline!='\t') continue; //�������
			while(*pline==' ' || *pline=='\t') pline++; //�����ָ�����
			mxname.assign(pline); break;
		}//?while( ::fgets(sline,256,fp)
		::fclose(fp);
	}//?if(fp)
	if(mxname!="") return 1;
	dnsClient dnsc; DNS_RDATA_MX mxRdata; 
	if(dnsc.Open(0)<=0) return 0;
	SOCKSRESULT sr=dnsc.Query_MX(domainMX,dnssvr,dnsport);
	if(sr==DNS_RCODE_ERR_OK) //����MX���������ɹ�
	{
		PDNS_HEADER pdnsh=dnsc.resp_dnsh();
		RW_LOG_DEBUG("[smtpclnt] Success to Query MX,results=%d.\r\n",pdnsh->answers);
		for(int i=0;i<pdnsh->answers;i++)
		{
			PDNS_RESPONSE pdnsr=dnsc.resp_dnsr(i);
			if(pdnsr->type==T_MX)
			{
				dnsc.parse_rdata_MX(&mxRdata);
				RW_LOG_DEBUG("\t %d [%d] %s\r\n",i+1,mxRdata.priority,mxRdata.mxname.c_str());
				mxname+=mxRdata.mxname+string(",");
			}
		}//for(...
		return 2;
	}else RW_LOG_DEBUG("[smtpclnt] Failed to Query MX,error=%d\r\n",sr);
	return 0;
}
SOCKSRESULT smtpClient :: sendMail_MX(mailMessage &mms,const char *dnssvr,int dnsport)
{
	std::vector<std::pair<std::string,std::string> > *pvec=NULL;
	const char *emailDomain_ptr=NULL,*emailDomain_pre=NULL;
	std::string strDomainMX;//MX��������
	m_authType=SMTPAUTH_NONE;//�ʼ�ֱͶ������MX������������֤
	int errCount=0; m_errors.clear();
	SOCKSRESULT sr=SOCKSERR_SMTP_RECIPIENT;
	while(true)
	{
		if(pvec==NULL) //ѭ�����͵�����Ŀ������
			pvec=&mms.vecTo(mailMessage::TO);
		else if(pvec==&mms.vecTo(mailMessage::TO) )
			pvec=&mms.vecTo(mailMessage::CC);
		else if(pvec==&mms.vecTo(mailMessage::CC))
			pvec=&mms.vecTo(mailMessage::BCC);
		else break;
		std::vector<std::pair<std::string,std::string> >::iterator it=pvec->begin();
		for(;it!=pvec->end();it++) 
		{//��ȡĿ�����������
			errCount++;
			emailDomain_ptr=strrchr((*it).first.c_str(),'@');
			if(emailDomain_ptr)
			{	
				emailDomain_ptr++; //����dns������ȡMX�������������
				if(emailDomain_pre==NULL || strDomainMX=="" || 
					strcasecmp(emailDomain_ptr,emailDomain_pre)!=0)
					dnsMX(dnssvr,dnsport,emailDomain_ptr,strDomainMX);
				
				emailDomain_pre=emailDomain_ptr;
				RW_LOG_DEBUG("[smtpclnt] begin to send email to %s into %s.\r\n",
					(*it).first.c_str(),strDomainMX.c_str());

				if(strDomainMX!="")//MX�����������
				{//׼�������ʼ�
					//strDomainMX��","�ŷָ�Ķ�������ַ���,ѭ����ȡMX����
					const char *ptrDomainMX=strDomainMX.c_str();
					while(ptrDomainMX[0]!=0)
					{
						const char *delmPos=strchr(ptrDomainMX,',');
						if(delmPos) *(char *)delmPos='\0';
						
						sr=ConnectSvr(ptrDomainMX,SMTP_SERVER_PORT);
						if( sr==SOCKSERR_OK)
						{
							if( (sr=_sendMail(mms,(*it).first.c_str()))!=SOCKSERR_OK)
							{
								string s=(*it).first; s.append(" failed to send.\r\n");
								m_errors.push_back(s);
								RW_LOG_DEBUG("[smtpclnt] Failed to send, error=%d\r\n",sr);
								if(delmPos) { *(char *)delmPos=','; delmPos=NULL; } //�˳�ѭ��
							}//?if( (sr=_sendMail(mms,(*it).first.c_str()))!=SOCKSERR_OK)
							else
							{ 
								errCount--;
								if(delmPos) { *(char *)delmPos=','; delmPos=NULL; } //�˳�ѭ��
							}
						}//?if( sr==SOCKSERR_OK)
						else
						{
							string s=(*it).first; s.append(" failed to connect ");
							s+=string(ptrDomainMX); s.append(".\r\n"); m_errors.push_back(s);
							RW_LOG_DEBUG("[smtpclnt] Failed to connect %s,return %d.\r\n",ptrDomainMX,sr);
						}//?if( sr==SOCKSERR_OK)...else
						this->Close(); //�Ͽ�����

						if(delmPos){
							*(char *)delmPos=',';
							ptrDomainMX=delmPos+1;
						}else break;
					}//?while(...)
				}//?if(strDomainMX!="")
				else
				{
					sr=SOCKSERR_SMTP_DNSMX;
					string s=(*it).first; s.append(" failed to parse Domain name.\r\n");
					m_errors.push_back(s);
				}
				RW_LOG_DEBUG("[smtpclnt] end to send email to %s into %s.\r\n",
					(*it).first.c_str(),strDomainMX.c_str());
			}//?if(emailDomain_ptr)
			else
			{
				sr=SOCKSERR_SMTP_EMAIL;
				string s=(*it).first; s.append(" is not a valid email.\r\n");
				m_errors.push_back(s);
				RW_LOG_DEBUG(0,"[smtpclnt] "); RW_LOG_DEBUG(s.length(),s.c_str());
			}
		}//?for(;it!=pvec->end();it++)
	}//?while
	if(errCount>0) sr=SOCKSERR_SMTP_FAILED; //�ʼ�δ��ȫ���ͳɹ�
	return sr;
}
*/