/*******************************************************************
   *	httprsp.cpp
   *    DESCRIPTION:HTTP ��Ӧ��������
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2006-02-08
   *
   *	net4cpp 2.1
   *	
   *******************************************************************/


#include "../include/sysconfig.h"
#include "../include/httprsp.h"
#include "../include/cCoder.h"
#include "../utils/cTime.h"
#include "../utils/utils.h"
#include "../include/cLogger.h"

using namespace std;
using namespace net4cpp21;

static const char* MIMETYPE_STR[]={
	"text/html",				//MIMETYPE_HTML
	"text/xml",					//MIMETYPE_XML
	"text/plain",				//MIMETYPE_TEXT
	 "text/css",				//MIMETYPE_CSS
	"application/zip",			//MIMETYPE_ZIP
	"application/msword",		//MIMETYPE_WORD
	"application/octet-stream",	//MIMETYPE_OCTET
	"image/x-icon",				//MIMETYPE_ICON
	"image/bmp",				//MIMETYPE_BMP
	"image/gif",				//MIMETYPE_GIF
	"image/png",				//MIMETYPE_PNG
	"image/jpeg",				//MIMETYPE_JPG
	"video/x-msvideo",			//MIMETYPE_AVI
	"video/x-ms-asf",			//MIMETYPE_ASF
	"video/mpeg",				//MIMETYPE_MPEG
	"application/pdf",			//MIMETYPE_PDF
	"message/rfc822",			//MIMETYPE_MHT
};


httpResponse::httpResponse()
{
	m_respcode=0;
	m_httprsp_lContentlen=-1; //��Ӧ���ݴ�Сδ֪
	m_httprsp_dwVer=MAKELONG(1,1);
	m_httprsp_bReceiveALL=true;

}

void httpResponse::init_httprsp()
{
	m_respcode=0;
	m_httprsp_lContentlen=-1;//��Ӧ���ݴ�Сδ֪
	m_httprsp_dwVer=MAKELONG(1,1);
	m_httprsp_bReceiveALL=true;
	m_httprsp_HEADER.clear();
	m_httprsp_SETCOOKIE.clear();
	m_httprsp_data.Resize(0);

	return;
}
void httpResponse::SetCookie(const char *cookiename,const char *cookieval,const char *path)
{
	if(cookiename==NULL || cookiename[0]==0) return;
	TNew_Cookie newCookie;
	newCookie.cookie_name.assign(cookiename);
	if(cookieval) newCookie.cookie_value.assign(cookieval);
	if(path && path[0]!=0) newCookie.cookie_path.assign(path);
	else newCookie.cookie_path.assign("/");
	m_httprsp_SETCOOKIE[newCookie.cookie_name]=newCookie;
	return;

}

//Content-Type: text/xml; charset=utf-8
//�����ύcontent�ı��뷽ʽ
const char *httpResponse::get_contentCharset()
{
	std::map<std::string,std::string>::iterator it=m_httprsp_HEADER.find("Content-Type");
	if(it==m_httprsp_HEADER.end()) return NULL;

	const char *ptr,*pvalue=(*it).second.c_str();
	if( (ptr=strchr(pvalue,';'))==NULL ) return NULL;
	ptr++; while(*ptr==' ') ptr++; //ȥ��ǰ���ո�
	if(strncasecmp(ptr,"charset=",8)!=0) return NULL;
	ptr+=8;while(*ptr==' ') ptr++; //ȥ��ǰ���ո�
	return ptr;
}
MIMETYPE_ENUM httpResponse::get_contentType()
{
	std::map<std::string,std::string>::iterator it=m_httprsp_HEADER.find("Content-Type");
	if(it!=m_httprsp_HEADER.end())
	{	
		const char *pvalue=(*it).second.c_str();
		for(int i=0;i<MIMETYPE_UNKNOWED;i++)
			if(strncasecmp(pvalue,MIMETYPE_STR[i],strlen(MIMETYPE_STR[i]))==0) 
				return (MIMETYPE_ENUM)i;	
	}
	return MIMETYPE_UNKNOWED;
}
MIMETYPE_ENUM httpResponse::get_mimetype()
{
	std::map<std::string,std::string>::iterator it=m_httprsp_HEADER.find("Content-Type");
	if(it==m_httprsp_HEADER.end()) it=m_httprsp_HEADER.find("CONTENT-TYPE");
	if(it!=m_httprsp_HEADER.end())
	{
		for(int i=0;i<MIMETYPE_UNKNOWED;i++)
			if(strcasecmp((*it).second.c_str(),MIMETYPE_STR[i])==0) return (MIMETYPE_ENUM)i;
	}
	return MIMETYPE_UNKNOWED;
}

void httpResponse::set_mimetype(MIMETYPE_ENUM mt)
{
	if(mt>=MIMETYPE_HTML && mt<MIMETYPE_UNKNOWED)
		m_httprsp_HEADER["Content-Type"]=MIMETYPE_STR[mt];
	return;
}
void httpResponse::NoCache() //��ֹ����
{
	m_httprsp_HEADER["Cache-control"]="no-cache";
	m_httprsp_HEADER["Pragma"]="no-cache";
	m_httprsp_HEADER["Expires"]="-1";
} 
//����ʣ���δ�������HTTP��Ӧ����
//receiveBytes - ����ָ�����ֽڳ���  <=0:��������δ������
//�ɹ�������,���򷵻ؼ�
bool httpResponse::recv_remainderX(socketTCP *psock,long receiveBytes,time_t timeout)
{
	//	ASSERT(psock);
	if(m_respcode<=0) return false; //�����Ѿ���ȡ��http��Ӧͷ
	long l,remainBytes=receiveBytes;
	while(	!m_httprsp_bReceiveALL )
	{
		if(m_httprsp_lContentlen>m_httprsp_data.len())
		{
			if(m_httprsp_data.Space()<1024) //Ԥ����һ���Ŀռ�
				if( m_httprsp_data.Resize(m_httprsp_data.size()+4096)==NULL ) 
					break;

			char *pbuf=m_httprsp_data.str()+m_httprsp_data.len();
			l=m_httprsp_data.Space()-1;
			if(receiveBytes>0 && l>remainBytes) l=remainBytes;
			//�������HTTP_MAX_RESPTIMEOUT��û�յ����ݿ���Ϊ�ͻ����쳣
			l=psock->Receive(pbuf,l,timeout);
			if(l<0) break; //RW_LOG_DEBUG("Failed to Receive - %d\r\n",l); 
			if(l==0){ cUtils::usleep(SCHECKTIMEOUT); continue; }//==0������������������������
			m_httprsp_data.len()+=l; 
			if( receiveBytes>0) if( (remainBytes-=l)<=0 ) break; //������ָ��������
		} else m_httprsp_bReceiveALL=true;
	}//?while
	m_httprsp_data[m_httprsp_data.len()]=0;
	if( receiveBytes>0) if(remainBytes<=0) return true;
	return m_httprsp_bReceiveALL;
}

//����http��ӦΪָ�����ļ�(������http��Ӧͷ)
//���ر����ļ��Ĵ�С��==0��������
unsigned long httpResponse::save_resp(socketTCP *psock,const char *filename)
{
	//	ASSERT(psock);
	if(m_respcode<=0) return 0; //�����Ѿ���ȡ��http��Ӧͷ
	if(filename==NULL || filename[0]==0) return 0;
	long startPos=0; FILE *fp=NULL;
	std::map<std::string,std::string>::iterator it=m_httprsp_HEADER.find("Range");
	if(it!=m_httprsp_HEADER.end()) startPos=atol((*it).second.c_str()+6);
	if(m_respcode==206) //��Ӧ����Ϊ��������
	{//��ȡ��ʼλ�� //��ʽ Content-Range: bytes %d-%d/%d
		it=m_httprsp_HEADER.find("Content-Range");
		if(it!=m_httprsp_HEADER.end()) startPos=atol((*it).second.c_str()+6);
	}
	fp=(startPos>0)?fopen(filename,"ab"):fopen(filename,"wb");
	if(fp==NULL) return 0; else if(startPos>0) fseek(fp,startPos,SEEK_SET); //ָ���Ƶ�ָ����λ��
/*  //yyc remove 2007-12-20 begin
	FILE *fp=::fopen(filename,"wb");
	if(fp==NULL) return 0;
	if(m_respcode==206) //��Ӧ����Ϊ��������
	{//��ȡ��ʼλ�� //��ʽ Content-Range: bytes %d-%d/%d
		long startPos=0;
		std::map<std::string,std::string>::iterator it=m_httprsp_HEADER.find("Content-Range");
		if(it!=m_httprsp_HEADER.end()) startPos=atol((*it).second.c_str()+6);
		::fseek(fp,startPos,SEEK_SET); //ָ���Ƶ�ָ����λ��
	}
*/ //yyc remove 2007-12-20 end 
	unsigned long recvBytes=0;
	if(m_httprsp_data.len()>0)
	{
		::fwrite(m_httprsp_data.str(),sizeof(char),m_httprsp_data.len(),fp);
		recvBytes+=m_httprsp_data.len();
		m_httprsp_data.len()=0;
	}
	char buf[4096]; long buflen=4096;
	while(recvBytes<(unsigned long)m_httprsp_lContentlen)
	{
		int iret=psock->checkSocket(SCHECKTIMEOUT,SOCKS_OP_READ);
		if(iret<0) break; 
		if(iret==0) continue;
		iret=psock->Receive(buf,buflen,-1);
		if(iret<0) break; recvBytes+=iret;
		::fwrite(buf,sizeof(char),iret,fp);
	}//?while(...
	::fclose(fp); 
	RW_LOG_PRINT(LOGLEVEL_DEBUG,"[httpResponse] success to save %s, size=%d - %d\r\n",
		filename,recvBytes,m_httprsp_lContentlen);
	return recvBytes;
}

//����http��Ӧͷ
//�ɹ�������Ӧ����<0�������� 0:δ֪����Ӧ�� 
SOCKSRESULT httpResponse::recv_rspH(socketTCP *psock,time_t timeout)
{
//	ASSERT(psock);
	const char * pFoundTerminator=NULL; //http��Ӧͷ�Ƿ�������
	cBuffer buffer(1024); char *pbuf;
	while(	psock->status()==SOCKS_CONNECTED ) 
	{
		pbuf=buffer.str()+buffer.len();
		//�������timeout��û�յ����ݿ���Ϊ�ͻ����쳣
		int iret=psock->Receive(pbuf,buffer.Space()-1,timeout);
		if(iret<0) break;
		if(iret==0){ cUtils::usleep(MAXRATIOTIMEOUT); continue; }//==0������������������������
		buffer.len()+=iret; buffer[buffer.len()]=0;
		if( (pFoundTerminator=strstr(buffer.str(),"\r\n\r\n")) )
		{	
			//���web���񷵻���Ӧ��100 ����˵�����յ�һ�������ݣ���δ������ϣ����ȴ����պ���������
			//��˶�100����Ӧ�룬�ӵ��������,����:
			//HTTP/1.1 100 Continue\r\n
			//Server: Microsoft-IIS/5.0\r\n
			//Date: Thu, 15 Sep 2005 06:14:13 GMT\r\n\r\n
			if(atoi(buffer.str()+9)==100){
				iret=buffer.len()-(pFoundTerminator+4-buffer.str());
				if(iret>0) ::memmove(buffer.str(),pFoundTerminator+4,iret);
				buffer.len()=iret; continue;
			} else break;
		}//?if( (pFoundTerminator=strstr(buffer.str(),"\r\n\r\n")) )
		if(buffer.Space()<256) //Ԥ����һ���Ŀռ䣬�Ա�������������ͷ
			if( buffer.Resize(buffer.size()+1024)==NULL ) break;						
	}//?while
	if(pFoundTerminator==NULL) return SOCKSERR_HTTP_RESP; //δ���յ�������http����ͷ

	//��ӡ���յ�http��Ӧͷ
	*(char *)pFoundTerminator=0;
	RW_LOG_DEBUG("[httprsp] Received HTTP Response Header\r\n%s\r\n",buffer.str());
	*(char *)pFoundTerminator='\r';

	init_httprsp(); //��ʼ��http �������ı���
	if(ParseResponse(buffer.str())==0) return 0;

	if(m_httprsp_lContentlen!=0)
	{//�ж���Ӧ�����Ƿ�������
		long receivedBytes=buffer.len()-(pFoundTerminator+4-buffer.str());
		if(m_httprsp_lContentlen>0 && receivedBytes>=m_httprsp_lContentlen)
			m_httprsp_bReceiveALL=true; //��Ӧ�����Ѿ��������
		else m_httprsp_bReceiveALL=false;
		//���ѽ��յ����ݴ�ŵ�m_httprsp_data��
		if(receivedBytes>0)
			::memmove(buffer.str(),pFoundTerminator+4,receivedBytes);
		buffer.len()=receivedBytes;
		m_httprsp_data=buffer;
	}//?if(m_httpreq_lContentlen>0)
	else m_httprsp_bReceiveALL=true;
	return m_respcode;
}

//����http��Ӧͷ
SOCKSRESULT httpResponse::send_rspH(socketTCP *psock,int respcode,const char *respDesc)
{
	cBuffer outbuf(1024);
	if(respcode>0) m_respcode=respcode;
	outbuf.len()+=sprintf(outbuf.str()+outbuf.len(),"HTTP/1.1 %d %s\r\n",m_respcode,respDesc);
	
	if(m_httprsp_lContentlen>=0){
		sprintf(outbuf.str()+outbuf.len(),"%d",m_httprsp_lContentlen);
		m_httprsp_HEADER["Content-Length"]=std::string(outbuf.str()+outbuf.len());
	}
	if(m_httprsp_lContentlen!=0) //����Ӧ����
	{
		//206ָ��������ǲ������ݣ�����Ӧͷ�������� Content-Range: bytes %d-%d/%d,�����ذ���Accept-Ranges
		if(respcode!=206) m_httprsp_HEADER["Accept-Ranges"]="bytes";
		if(m_httprsp_HEADER.count("Content-Type")<1) 
			m_httprsp_HEADER["Content-Type"]=MIMETYPE_STR[MIMETYPE_HTML];
	}
	
	if(!m_httprsp_HEADER.empty())
	{
		std::map<std::string,std::string>::iterator it=m_httprsp_HEADER.begin();
		for(;it!=m_httprsp_HEADER.end();it++)
		{
			outbuf.len()+=sprintf(outbuf.str()+outbuf.len(),
			"%s: %s\r\n",(*it).first.c_str(),(*it).second.c_str());
			if(outbuf.Space()<256) //Ԥ����һ���Ŀռ�
				if( outbuf.Resize(outbuf.size()+256)==NULL ) break;	
		}
	}//?

	if(!m_httprsp_SETCOOKIE.empty()){
		std::map<std::string,TNew_Cookie>::iterator it=m_httprsp_SETCOOKIE.begin();
		for(;it!=m_httprsp_SETCOOKIE.end();it++){
			TNew_Cookie &newCookie=(*it).second;
			if(newCookie.cookie_expires!="")
			outbuf.len()+=sprintf(outbuf.str()+outbuf.len(),"Set-Cookie: %s=%s; expires=%s; path=%s\r\n",
				newCookie.cookie_name.c_str(),newCookie.cookie_value.c_str(),newCookie.cookie_expires.c_str(),newCookie.cookie_path.c_str());
			else
			outbuf.len()+=sprintf(outbuf.str()+outbuf.len(),"Set-Cookie: %s=%s; path=%s\r\n",
				newCookie.cookie_name.c_str(),newCookie.cookie_value.c_str(),newCookie.cookie_path.c_str());
		}//?for(;
	}

	outbuf.len()+=sprintf(outbuf.str()+outbuf.len(),"\r\n");
//	RW_LOG_DEBUG(outbuf.len(),outbuf.str());
	return psock->Send(outbuf.len(),outbuf.str(),-1);
}

MIMETYPE_ENUM httpResponse::MimeType(const char *filename)
{
	const char *ptr=strrchr(filename,'.');
	if(ptr)
	{
	if(strcasecmp(ptr,".ico")==0)
		return MIMETYPE_ICON;
	else if(strcasecmp(ptr,".bmp")==0)
		return MIMETYPE_BMP;
	else if(strcasecmp(ptr,".jpg")==0)
		return MIMETYPE_JPG;
	else if(strcasecmp(ptr,".jpeg")==0)
		return MIMETYPE_JPG;
	else if(strcasecmp(ptr,".gif")==0)
		return MIMETYPE_GIF;
	else if(strcasecmp(ptr,".png")==0)
		return MIMETYPE_PNG;
	else if(strcasecmp(ptr,".doc")==0)
		return MIMETYPE_WORD;
	else if(strcasecmp(ptr,".xls")==0)
		return MIMETYPE_WORD;
	else if(strcasecmp(ptr,".zip")==0)
		return MIMETYPE_ZIP;
	else if(strcasecmp(ptr,".css")==0)
		return MIMETYPE_CSS;
	else if(strcasecmp(ptr,".xml")==0)
		return MIMETYPE_XML;
	else if(strncasecmp(ptr,".htm",4)==0)
		return MIMETYPE_HTML;
	else if(strcasecmp(ptr,".avi")==0)
		return MIMETYPE_AVI;
	else if(strncasecmp(ptr,".asf",4)==0)
		return MIMETYPE_ASF;
	else if(strncasecmp(ptr,".mpg",4)==0)
		return MIMETYPE_MPEG;
	else if(strncasecmp(ptr,".pdf",4)==0)
		return MIMETYPE_PDF;
	else if(strcasecmp(ptr,".txt")==0)
		return MIMETYPE_TEXT;
	else if(strcasecmp(ptr,".mht")==0)
		return MIMETYPE_MHT;
	}
	return MIMETYPE_OCTET;
}
//������չ���ж�Ҫ�����ļ�������
inline MIMETYPE_ENUM getMimeType(const char *filename,FILE *fp)
{
	const char *ptr=strrchr(filename,'.');
	if(ptr)
	{
	if(strcasecmp(ptr,".ico")==0)
		return MIMETYPE_ICON;
	else if(strcasecmp(ptr,".bmp")==0)
		return MIMETYPE_BMP;
	else if(strcasecmp(ptr,".jpg")==0)
		return MIMETYPE_JPG;
	else if(strcasecmp(ptr,".jpeg")==0)
		return MIMETYPE_JPG;
	else if(strcasecmp(ptr,".gif")==0)
		return MIMETYPE_GIF;
	else if(strcasecmp(ptr,".png")==0)
		return MIMETYPE_PNG;
	else if(strcasecmp(ptr,".doc")==0)
		return MIMETYPE_WORD;
	else if(strcasecmp(ptr,".xls")==0)
		return MIMETYPE_WORD;
	else if(strcasecmp(ptr,".zip")==0)
		return MIMETYPE_ZIP;
	else if(strcasecmp(ptr,".css")==0)
		return MIMETYPE_CSS;
	else if(strcasecmp(ptr,".xml")==0)
		return MIMETYPE_XML;
	else if(strncasecmp(ptr,".htm",4)==0)
		return MIMETYPE_HTML;
	else if(strcasecmp(ptr,".avi")==0)
		return MIMETYPE_AVI;
	else if(strncasecmp(ptr,".asf",4)==0)
		return MIMETYPE_ASF;
	else if(strncasecmp(ptr,".mpg",4)==0)
		return MIMETYPE_MPEG;
	else if(strncasecmp(ptr,".pdf",4)==0)
		return MIMETYPE_PDF;
	else if(strcasecmp(ptr,".txt")==0)
		return MIMETYPE_TEXT;
	else if(strcasecmp(ptr,".mht")==0)
		return MIMETYPE_MHT;
	}
	if(fp==NULL) return MIMETYPE_OCTET;
	//��ȡǰ128���ֽ��ж��Ƿ�Ϊ���ı��ļ�
	unsigned char buf[128];
	long oldpos=::ftell(fp);
	int iret=::fread(buf,sizeof(unsigned char),128,fp);
	::fseek(fp,oldpos,SEEK_SET);
	MIMETYPE_ENUM mt=MIMETYPE_TEXT;
	for(int i=0;i<iret;i++)
		if((buf[i]>0 && buf[i]<10) || buf[i]==0xff) { mt=MIMETYPE_OCTET; break; }
	return mt;
}

void setLastModify(const char *filename,std::map<std::string,std::string> &HEADER)
{
	WIN32_FIND_DATA finddata;
	HANDLE hd=::FindFirstFile(filename, &finddata);
	if(hd!=INVALID_HANDLE_VALUE)
	{
		FILETIME ft=finddata.ftLastWriteTime;
		::FindClose(hd); 
		char buf[64]; cTime ct(ft); 
		ct.FormatGmt(buf,64,"%a, %d %b %Y %H:%M:%S GMT");
		HEADER["Last-Modified"]=string(buf);
		ct=cTime::GetCurrentTime();
		ct.FormatGmt(buf,64,"%a, %d %b %Y %H:%M:%S GMT");
		HEADER["Date"]=string(buf);
	}//?if(hd!=INVALID_HANDLE_VALUE)
	return;
}
//�����ļ����ɹ�����SOCKSERR_OK
SOCKSRESULT httpResponse::sendfile(socketTCP *psock,const char *filename,MIMETYPE_ENUM mt,long startPos,long endPos)
{
	if(filename==NULL || filename[0]==0) return SOCKSERR_PARAM;
	FILE *fp=::fopen(filename,"rb");
	if(fp==NULL) return SOCKSERR_PARAM;
	//������չ���ж�Ҫ�����ļ�������
	if(mt==MIMETYPE_UNKNOWED) mt=getMimeType(filename,NULL);
	if(mt!=MIMETYPE_NONE) set_mimetype(mt);
	setLastModify(filename,m_httprsp_HEADER);

	char buf[SSENDBUFFERSIZE]; SOCKSRESULT sr;
	fseek(fp,0,SEEK_END); long filesize=::ftell(fp);
	if(endPos<=0) endPos=filesize-1;
	m_httprsp_lContentlen=endPos-startPos+1;
	if(startPos>0)
	{
		sprintf(buf,"bytes %d-%d/%d",startPos,endPos,filesize);
		m_httprsp_HEADER["Content-Range"]=string(buf);
		sr=send_rspH(psock,206,"Partial content");
	}else sr=send_rspH(psock,200,"OK");
	if(sr<0) return sr;

	::fseek(fp,startPos,SEEK_SET);
	while(psock->status()==SOCKS_CONNECTED)
	{
		int iret=psock->checkSocket(SCHECKTIMEOUT,SOCKS_OP_WRITE);
		if(iret<0) break; else if(iret==0) continue;
		iret=::fread(buf,sizeof(char),SSENDBUFFERSIZE,fp);
		if(psock->Send(iret,buf,-1)<0) break;
		if(iret<SSENDBUFFERSIZE) break;
		if(::ftell(fp)>endPos) break;
	}//?while(...
	::fclose(fp);
	return SOCKSERR_OK;
}

//�����ļ��������ϵ����ݣ��ɹ�����SOCKSERR_OK
const char HTTPBOUNDARY_STRING[]="--#yycnet.yeah.net#";
SOCKSRESULT httpResponse::sendfile(socketTCP *psock,const char *filename,MIMETYPE_ENUM mt,long* lpstartPos,long* lpendPos,int iRangeNums)
{
	if(filename==NULL || filename[0]==0 || iRangeNums<=0) return SOCKSERR_PARAM;
	FILE *fp=::fopen(filename,"rb");
	if(fp==NULL) return SOCKSERR_PARAM;
	char buf[1024]; SOCKSRESULT sr;
	sprintf(buf,"multipart/byteranges; boundary=%s",HTTPBOUNDARY_STRING);
	m_httprsp_HEADER["Content-Type"]=string(buf);
	setLastModify(filename,m_httprsp_HEADER);

	long i,l,filesize,lContentlen=0;
	fseek(fp,0,SEEK_END); filesize=::ftell(fp);
	//������չ���ж�Ҫ�����ļ�������
	if(mt==MIMETYPE_UNKNOWED) mt=getMimeType(filename,NULL);
	std::vector<std::pair<const char *,long> > v;
	char *pbuf=buf;
	for(i=0;i<iRangeNums;i++)
	{
		if(lpendPos[i]<=0) lpendPos[i]=filesize-1;
		l=sprintf(pbuf,"--%s\r\nContent-Type: %s\r\nContent-Range: bytes %d-%d/%d\r\n\r\n",
			HTTPBOUNDARY_STRING,MIMETYPE_STR[mt],lpstartPos[i],lpendPos[i],filesize);
		lContentlen+=(l+lpendPos[i]-lpstartPos[i]+1);
		std::pair<const char *,long> p(pbuf,l);
		v.push_back(p); pbuf+=(l+1);
	}//?for(

	m_httprsp_lContentlen=lContentlen + 2 * iRangeNums; //��Ҫ����ÿ�����ݿ�����\r\n
	sr=send_rspH(psock,206,"Partial content");
	if(sr<0) return sr;
	char *readbuf=new char[SSENDBUFFERSIZE];
	if(readbuf==NULL) return SOCKSERR_MEMORY;
	
	for(i=0;i<iRangeNums;i++)
	{
		std::pair<const char *,long> &p=v[i];
		sr=psock->Send(p.second,p.first,-1);
		if(sr<0) return sr;
		::fseek(fp,lpstartPos[i],SEEK_SET);
		//��ȡ������ָ����С������
		long readlen=lpendPos[i]-lpstartPos[i]+1;
		while(readlen>0)
		{
			int iret=psock->checkSocket(SCHECKTIMEOUT,SOCKS_OP_WRITE);
			if(iret<0) break; else if(iret==0) continue;
			l=(readlen>SSENDBUFFERSIZE)?SSENDBUFFERSIZE:readlen;
			iret=::fread(readbuf,sizeof(char),l,fp);
			if(psock->Send(iret,readbuf,-1)<0) break;
			if(iret<l) break; else readlen-=iret;
		}
		psock->Send(2,"\r\n",-1); //���ݿ������\r\n����
	}
	delete[] readbuf;
	::fclose(fp);
	return SOCKSERR_OK; 
}

//����http��Ӧͷ��������Ӧ��
int httpResponse::ParseResponse(const char *httprspH)
{
	//����ÿһ������
	bool bFirstLine = true;
	const char *ptrLineStart=httprspH;
	const char *ptrLineEnd=strchr(ptrLineStart,'\n');
	while(true)
	{
		if(ptrLineEnd){
			*(char *)ptrLineEnd='\0';
			if(*(ptrLineEnd-1)=='\r') *(char*)(ptrLineEnd-1)='\0';
		}

		if(bFirstLine)
		{//�����һ������
			if(strncasecmp(ptrLineStart,"HTTP/",5)!=0) return 0;
			const char *ptrStart=ptrLineStart+5;
			const char *pFoundTerminator=strchr(ptrStart,' ');
			if( (m_respcode=atoi(pFoundTerminator+1))==0 ) return 0;
			//��ȡhttpЭ��汾
			pFoundTerminator=strchr(ptrStart,'.');
			if(pFoundTerminator)
			{
				WORD wMinorVersion=(WORD)atoi(pFoundTerminator+1);
				WORD wMajorVersion=(WORD)atoi(ptrStart);
				m_httprsp_dwVer=MAKELONG(wMinorVersion, wMajorVersion);
			}
			bFirstLine=false;
		}
		else
		{//��������������
			const char *pvalue,*pFoundTerminator=strchr(ptrLineStart,':');
			if(pFoundTerminator)
			{
				*(char *)pFoundTerminator='\0';
				pvalue=pFoundTerminator+1;
				while(*pvalue==' ') pvalue++;//ɾ��ǰ���ո�
				
				if(strcasecmp(ptrLineStart,"Content-Length")==0)
				{
					m_httprsp_lContentlen=atol(pvalue);
				}
//				else if(strcmp(ptrLineStart,"Cookie")==0)
//				{//cookie���ݸ�ʽ��Cookie: name=value; name=value...
//					std::string strtmp(pvalue);
//					parseParam((char *)strtmp.c_str(),';',m_httprsp_COOKIE);
//				}
				//�����õ�cookie����ʽSet-Cookie: ssic=aaaa; expires=Thu, 13-Dec-2007 04:02:04 GMT; path=/
				else if(strcmp(ptrLineStart,"Set-Cookie")==0)
					parse_SetCookie(pvalue);
				else
					m_httprsp_HEADER[ptrLineStart]=string(pvalue);

				*(char *)pFoundTerminator=':';
			}//?if(pFoundTerminator)
		}//?if(bFirstLine)...else

		if(ptrLineEnd==NULL) 
			break;
		else
		{
			*(char *)ptrLineEnd='\n';
			if(*(ptrLineEnd-1)=='\0') *(char*)(ptrLineEnd-1)='\r';
			//����������http����ͷ����
			if(ptrLineStart[0]=='\r' || ptrLineStart[0]=='\n') break; 
		}
		ptrLineStart=ptrLineEnd+1;
		ptrLineEnd=strchr(ptrLineStart,'\n');
	}//?while
	return m_respcode;
}

//�����������
void httpResponse::parse_SetCookie(const char *strParam)
{
	std::string strCookie(strParam);
	char *ptr,*ptrEnd,*ptrStart=(char *)strCookie.c_str();
	
	std::string sName,sVal;
	TNew_Cookie newCookie; newCookie.cookie_path="/";
	while(true)
	{
		while(*ptrStart==' ') ptrStart++;//ɾ��ǰ���ո�
		ptrEnd=strchr(ptrStart,';');
		if(ptrEnd) *ptrEnd=0;
		if( (ptr=strchr(ptrStart,'=')) )
		{
			*ptr++=0;
			cCoder::mime_decode(ptrStart,ptr-1-ptrStart,ptrStart);
			if(ptrEnd)
				 cCoder::mime_decode(ptr,ptrEnd-ptr,ptr);
			else cCoder::mime_decode(ptr,strlen(ptr),ptr);
			sName.assign(::_strlwr(ptrStart)); //nameӦ�����ִ�Сд�����ȫת��ΪСд
			sVal.assign(ptr);
			if(sName=="expires") newCookie.cookie_expires=sVal;
			else if(sName=="path") newCookie.cookie_path=sVal;
			else{ newCookie.cookie_name=sName; newCookie.cookie_value=sVal; }
		}
		if(ptrEnd==NULL) break; else ptrStart=ptrEnd+1;
	}//?while
	if(newCookie.cookie_name!="")
		m_httprsp_SETCOOKIE[newCookie.cookie_name]=newCookie;
	return;
}
void httpResponse::parseParam(char *strParam,char delm,
							 std::map<std::string,std::string> &maps)
{
	char *ptr,*ptrEnd,*ptrStart=strParam;
	while(*ptrStart==' ') ptrStart++;//ɾ��ǰ���ո�
	while( (ptrEnd=strchr(ptrStart,delm)) )
	{
		*ptrEnd=0;
		if( (ptr=strchr(ptrStart,'=')) )
		{
			*ptr=0;
			cCoder::mime_decode(ptrStart,ptr-ptrStart,ptrStart);
			ptr++;
			cCoder::mime_decode(ptr,ptrEnd-ptr,ptr);
			//nameӦ�����ִ�Сд�����ȫת��ΪСд
			::_strlwr(ptrStart);
			maps[ptrStart]=string(ptr);
		}
		ptrStart=ptrEnd+1;
		while(*ptrStart==' ') ptrStart++;//ɾ��ǰ���ո�
	}//?while
	//���ܻ���һ������û�д���
	if( (ptr=strchr(ptrStart,'=')) )
	{
		*ptr=0;
		cCoder::mime_decode(ptrStart,ptr-ptrStart,ptrStart);
		ptr++; ptrEnd=ptr; //���һ���������ܺ��лس�����(Mozilla�����)
		while(*ptrEnd && *ptrEnd!='\r' && *ptrEnd!='\n') ptrEnd++;
		cCoder::mime_decode(ptr,ptrEnd-ptr,ptr);
		//nameӦ�����ִ�Сд�����ȫת��ΪСд
		::_strlwr(ptrStart);
		maps[ptrStart]=string(ptr);
	}
	return;
}
//�������
void httpResponse::encodeParam(cBuffer &buf,char delm,
							 std::map<std::string,std::string> &maps)
{
	std::map<std::string,std::string>::iterator it=maps.begin();
	for(;it!=maps.end();it++)
	{
		int l=cCoder::MimeEncodeSize((*it).first.length())+
			cCoder::MimeEncodeSize((*it).second.length())+1;
		if(l<128) l=128; //���Ǳ���һ���Ŀռ����Ƶ�����䲢�ƶ�
		if(buf.Space()<l)
			if(buf.Resize(buf.size()+l)==NULL) break;

		if(it!=maps.begin()) { buf[buf.len()]=delm; buf.len()++; }
		l=cCoder::mime_encodeEx((*it).first.c_str(),(*it).first.length(),
			buf.str()+buf.len());
		buf.len()+=l; buf[buf.len()]='='; buf.len()++;
		l=cCoder::mime_encodeEx((*it).second.c_str(),(*it).second.length(),
			buf.str()+buf.len());
		buf.len()+=l; 
	}//?for
	return;
}

