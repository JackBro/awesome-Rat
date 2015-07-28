/*******************************************************************
   *	telnetserver.h 
   *    DESCRIPTION: Telnet����
   *
   *    AUTHOR:yyc
   *
   *    HISTORY:
   *	
   *******************************************************************/

#include "rmtsvc.h" 
#include "shellCommandEx.h"
#include "other/Wutils.h"

class clsOutput_sock : public clsOutput
{
public:
	clsOutput_sock(socketTCP *psock):m_psock(psock){};
	virtual ~clsOutput_sock(){}
	int print(const char *buf,int len)
	{
		if(m_psock==NULL) return 0;
		return m_psock->Send(len,buf,-1);
	}
	socketBase *psocket(){ return m_psock; }
private:
	socketTCP *m_psock;
};

void cTelnetEx :: onCommand(const char *strCommand,socketTCP *psock)
{
	//�ж��Ƿ���������ļ�
	const char *ptr_outfile=strchr(strCommand,'>');
	if(ptr_outfile){ *(char *)ptr_outfile++=0;
		while(*ptr_outfile==' ') ptr_outfile++; //ȥ��ǰ���ո�
	}

	int cmdlen=strlen(strCommand); //ȥ��β���ո�
	while(cmdlen>0 && *(strCommand+cmdlen-1)==' ') cmdlen--;
	if(cmdlen<=0) return; else *((char *)strCommand+cmdlen)=0;
//----------  ��չ�������� start------------------------------
	BOOL bRet=FALSE; std::string strOutput;
	const char *strCmd=strCommand;
	const char *strParam=strchr(strCommand,' ');
	if(strParam) { *(char *)strParam=0; strParam++; }

	if(strcasecmp(strCmd,"update")==0 || strcasecmp(strCmd,"down")==0) //����������
	{
		if(strParam==NULL) return;
		bool bUpdate=(strcasecmp(strCmd,"update")==0);
		while(*strParam==' ') strParam++;
		int iType=0; const char *strurl=strParam;
		if(strncasecmp(strurl,"http://",7)==0) iType=1;
		else if(strncasecmp(strurl,"https://",8)==0) iType=1;
		else if(strncasecmp(strurl,"ftp://",6)==0) iType=2;
		std::string strSaveas;
		if(iType>0){//����ָ�����ļ�
			clsOutput_sock sout(psock);
			const char *ptr=strchr(strurl,' ');
			if( ptr ){ *(char *)ptr=0; ptr+=1;
				while(*ptr==' ') ptr++; strSaveas.assign(ptr); 
			}
			if(strSaveas==""){ if( (ptr=strrchr(strurl,'/')) ) strSaveas.assign(ptr+1); }
			if(strSaveas[0]!='\\' && strSaveas[1]!=':') strSaveas.insert(0,g_savepath);
			if(bUpdate) strSaveas.append(".upd"); //��ֹ���ص��ļ���Ҫ�����ĳ�������
			bRet=(iType==2)?downfile_ftp(strurl,strSaveas.c_str(),sout):downfile_http(strurl,strSaveas.c_str(),sout);
			strurl=(bRet)?strSaveas.c_str():NULL;
		}else if(!bUpdate) strOutput.append("Failed , wrong URLs.\r\n");
		//����rmtsvc
		if(bUpdate)  bRet=updateRV(strurl,strOutput);
	}else if(strcasecmp(strCmd,"telnet")==0) //����telnet
		bRet=FALSE;
	else //ִ����չ����
		bRet=doCommandEx(strCmd,strParam,strOutput);
//----------  ��չ��������  end ------------------------------
	
	if(bRet)
	{
		FILE *fp=(ptr_outfile)?::fopen(ptr_outfile,"w"):NULL;
		if(fp){
			::fwrite(strOutput.c_str(),sizeof(char),strOutput.length(),fp);
			::fclose(fp);
			strOutput.assign("output >> "); strOutput.append(ptr_outfile);
		}
		strOutput.append("\r\n****Success to action****\r\n");
	}else strOutput.append("****Failed to action*****\r\n");
	if(strOutput!="") psock->Send(strOutput.length(),strOutput.c_str(),-1);
	return;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
telServerEx :: telServerEx()
{
	m_strSvrname.assign("Telnet Server");
	m_bindip="";
	m_svrport=0;
}
telServerEx :: ~telServerEx()
{
	 Stop(); m_threadpool.join();
}

//��������
bool telServerEx :: Start() 
{
	if(m_svrport==0) return true; //����������
	
	const char *ip=(m_bindip=="")?NULL:m_bindip.c_str();
	BOOL bReuseAddr=(ip)?SO_REUSEADDR:FALSE;//����IP������˿�����
	SOCKSRESULT sr=Listen( ((m_svrport<0)?0:m_svrport) ,bReuseAddr,ip);
	return (sr>0)?true:false;
}


void telServerEx ::revConnectionThread(socketTCP *psock)
{
	telServerEx *ptelsvr=(telServerEx *)psock->parent();
	MyService *pmysvr=MyService::GetService();
	socketBase *pevent=(pmysvr)?pmysvr->GetSockEvent():NULL;
	psock->setParent(pevent);
	ptelsvr->onConnect(psock);
	delete psock; return;
}
SOCKSRESULT telServerEx :: revConnect(const char *host,int port,time_t lWaitout)
{
	socketTCP *psock=new socketTCP;
	if(psock==NULL) return SOCKSERR_INVALID;
	else psock->setParent(this);
	SOCKSRESULT sr=psock->Connect(host,port,lWaitout);
	if(sr>0)
	if(m_threadpool.addTask((THREAD_CALLBACK *)&revConnectionThread,(void *)psock,THREADLIVETIME)==0)
		sr=SOCKSERR_THREAD;
	
	if(sr<=0) delete psock;
	return sr;
}

//����telnet����������Ϣ
//�����ʽ: 
//	telnet [port=<����˿�>] [bindip=<������󶨵ı���IP>]  [account=<�����ʺ�:����>] 
//port=<����˿�>    : ���÷���˿ڣ������������Ĭ��Ϊ0.����Ϊ0������web���� <0���漴����˿�
//bindip=<������󶨵ı���IP> : ���ñ�����󶨵ı���IP�������������Ĭ�ϰ󶨱�������IP
//account=<�����ʺ�:����>
void telServerEx :: docmd_sets(const char *strParam)
{
	std::map<std::string,std::string> maps;
	if(splitString(strParam,' ',maps)<=0) return;
	std::map<std::string,std::string>::iterator it;

	if( (it=maps.find("port"))!=maps.end())
	{//���÷���Ķ˿�
		m_svrport=atoi((*it).second.c_str());
	}
	if( (it=maps.find("bindip"))!=maps.end())
	{//���÷����IP
		m_bindip=(*it).second;
	}
	
	if( (it=maps.find("account"))!=maps.end())
	{
		const char *ptr=strchr((*it).second.c_str(),':');
		if(ptr){
			*(char *)ptr=0;
			setTelAccount((*it).second.c_str(),ptr+1);
			*(char *)ptr=':';
		}else setTelAccount(NULL,NULL); //�����ʺ�����
	}
	
	return;
}
//���÷����ip���˹�������ĳ���ʺŵ�IP���˹���
//�����ʽ:
//	iprules [access=0|1] ipaddr="<IP>,<IP>,..."
//access=0|1     : �Է�������IP�������Ǿܾ����Ƿ���
//����:
// iprules access=0 ipaddr="192.168.0.*,192.168.1.10"
void telServerEx :: docmd_iprules(const char *strParam)
{
	std::map<std::string,std::string> maps;
	if(splitString(strParam,' ',maps)<=0) return;
	std::map<std::string,std::string>::iterator it;

	int ipaccess=1;
	if( (it=maps.find("access"))!=maps.end())
		ipaccess=atoi((*it).second.c_str());
	
	if( (it=maps.find("ipaddr"))!=maps.end())
	{
		std::string ipRules=(*it).second;
		this->rules().addRules_new(RULETYPE_TCP,ipaccess,ipRules.c_str());
	}else this->rules().addRules_new(RULETYPE_TCP,ipaccess,NULL);

	return;
}
