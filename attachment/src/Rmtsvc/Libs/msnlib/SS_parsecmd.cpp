/*******************************************************************
   *	SS_parsecmd.cpp
   *    DESCRIPTION:�����SS�������յ�������
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
#include "../../include/cLogger.h"
#include "msnlib.h"

using namespace std;
using namespace net4cpp21;

//��SS�յ���������Ự��Ϣ
//��ʽ��HTTP requestЭ�����ƣ�\r\n\r\n�ָ���Ϣͷ����Ϣ��
//����:
// MSG yycnet@hotmail.com yyc:) 91\r\n
// MIME-Version: 1.0\r\n
// Content-Type: text/x-msmsgscontrol\r\n
// TypingUser: yycnet@hotmail.com\r\n
// \r\n
//��
// MSG yycnet@hotmail.com yyc:) 139\r\n
// MIME-Version: 1.0\r\n
// Content-Type: text/plain; charset=UTF-8\r\n
// X-MMS-IM-Format: FN=%E5%AE%8B%E4%BD%93; EF=; CO=0; CS=86; PF=0\r\n
// \r\n
// gdgdfggdgdgfd

//email --- ���ʹ�msg��Ϣ����ϵ��
unsigned long msnMessager :: sscmd_msg(cContactor *pcon,const char *msg_email,char *pcmd,int cmdlen)
{
	char *pBodyData=NULL;//��Ϣ��
	int bodyDataLen=0; //��Ϣ�峤��
	const char *ptr_ContentType=NULL;//��Ϣ������
	char *ptr_fmtFonts=NULL; //���������ʽ
	char *ptr_p4Context=NULL;//��������ʾ����
	const char *ptr_TypingUser=NULL;//��������û�email
	const char *ptr_P2pDest=NULL;//ָ��P2P-Dest����
	
	const char *ptr_MSNProxy=NULL; //ΪMSN�������Զ���ı�ǩ

//	RW_LOG_PRINT(LOGLEVEL_DEBUG,"MSG len=%d, %s.\r\n",cmdlen,pcmd);
	//��ʼ������Ϣͷ-----------start-------------------------
	char *tmpptr,*ptr,*pStart=pcmd;
	while( (ptr=strchr(pStart,'\r')) )
	{
		*ptr=0;
		if( (tmpptr=strchr(pStart,':')) )
		{
			*tmpptr=0;
			if(strcmp(pStart,"Content-Type")==0)
				ptr_ContentType=tmpptr+2;
			else if(strcmp(pStart,"X-MMS-IM-Format")==0)
				ptr_fmtFonts=tmpptr+2;
			else if(strcmp(pStart,"P4-Context")==0)
				ptr_p4Context=tmpptr+2;
			else if(strcmp(pStart,"TypingUser")==0)
				ptr_TypingUser=tmpptr+2;
//			else if(strcmp(pStart,"P2P-Dest")==0)
//				ptr_P2pDest=tmpptr+2;
#ifdef __SURPPORT_MSNPROXY__
			else if(strcmp(pStart,"MSN-Proxy")==0)
				ptr_MSNProxy=tmpptr+2;
#endif
		}//?if( (tmpptr=strchr(pStart,':')) )
		int i=1; while(*(ptr+i)=='\r' || *(ptr+i)=='\n') i++; //����\r\n
		if(i>2){ //����������\r\n�����������Ϊ��Ϣ�塣��Ϣ���������ContentType����
			pBodyData=ptr+i;
			bodyDataLen=cmdlen-(pBodyData-pcmd);
			break; 
		} 
		pStart=ptr+i;
	}//?while(...
	if(ptr_ContentType==NULL) return 0;
	//������Ϣͷ����----------- end -------------------------

	//����ContentType������Ϣ
	if(strcmp(ptr_ContentType,"text/x-msmsgscontrol")==0)
	{//�յ�һ�����������Ϣ,TypingUser:ָ��ĳ���û���������������Ϣ
		onChatSession((HCHATSESSION)pcon,MSN_CHATSESSION_TYPING,ptr_TypingUser,0);
	}//?if(strcmp(ptr_ContentType,"text/x-msmsgscontrol")==0)

	else if(strncmp(ptr_ContentType,"text/plain",10)==0)
	{//�յ�һ��������Ϣ
		if(pBodyData==NULL) return 0;
#ifdef __SURPPORT_MSNPROXY__
		if(ptr_MSNProxy)
			onProxyChat((HCHATSESSION)pcon,ptr_MSNProxy,pBodyData,bodyDataLen);
		else
#endif
		{//-------------------------------------------------------
			if(strstr(ptr_ContentType+10,"charset=UTF-8")==NULL)
				RW_LOG_PRINT(LOGLEVEL_WARN,0,"[msnChat] text/plain is not UTF-8\r\n");
			
			onChat((HCHATSESSION)pcon,msg_email,pBodyData,bodyDataLen,ptr_fmtFonts,ptr_p4Context);
			if( pBodyData[0]!=0 && pcon->m_shell.isValidW() )
			{
				if(pBodyData[0]==1)
					pcon->m_shell.Write("\r\n",2);
				else pcon->m_shell.WriteCrLf(pBodyData,bodyDataLen);
			}//?if(pBodyDataW[0]!=0)
		}//---------------------------------------------------------
	}//?else if(strncmp(ptr_ContentType,"text/plain",10)==0)

	else if(strncmp(ptr_ContentType,"text/x-msmsgsinvite",10)==0)
	{//msnc0Э����Ϣ.�ļ��������msnftp��msnp10�Ժ���msnc1Э��
	//netmeeting,��Ƶ�����Լ�msnftp����ʹ�õ�msnc0Э�顣yyc comment 2005-07-21
		msnc0_parse(pcon,msg_email,pBodyData); //,bodyDataLen
	}//?else if(strncmp(ptr_ContentType,"text/x-msmsgsinvite",10)==0)

	else if(strcmp(ptr_ContentType,"application/x-msnmsgrp2p")==0)
	{//msnc1Э�� - msnp2p��Ϣ. ��������ͷ�����Լ�msnp9�Ժ���ļ�/ͷ���䶼��ʹ�ô�Э��
		//msnp2p��Ϣ����������� 48�ֽڵ�Binary stuff + option Data + 4�ֽڵ�footer,��msnc1Э��˵��
		unsigned char *pheader=(unsigned char *)pBodyData;  
		unsigned long lfooter;//����ĸ��ֽ�Ϊfooter,in Big Endian order
		*((char *)&lfooter)=*(pBodyData+bodyDataLen-1);
		*((char *)&lfooter+1)=*(pBodyData+bodyDataLen-2);
		*((char *)&lfooter+2)=*(pBodyData+bodyDataLen-3);
		*((char *)&lfooter+3)=*(pBodyData+bodyDataLen-4);
		char *ptrmsg=pBodyData+48; int msglen=bodyDataLen-48-4;
		msnc1_parse(pcon,msg_email,pheader,ptrmsg,lfooter);
	}//?else if(strcmp(ptr_ContentType,"application/x-msnmsgrp2p")==0)

//	else if(strcmp(ptr_ContentType,"text/x-msnmsgr-datacast")==0)
//	{//�Է�����һ�����鶯��
/*	��ʽ MIME-Version: 1.0\r\n
		 Content-Type: text/x-msnmsgr-datacast\r\n                    
		 Message-ID: {E1255EF3-88D3-4270-A9AB-294686282F41}\r\n      //Ψһ��ʶһ��������ϢID
		 Chunks: 3\r\n                                               //��ʾ�˶�������Ҫ��������MSG�鷢���꣬�˴�Ϊ3�顣ʣ��ÿ��MSG���Message-ID�ʹ�MSG��Message-ID��ͬ
		 \r\n
		 ID: 2\r\n													//ID=2�����˴�Ϊһ�����鶯��,�������1����һ��������
		 Data: <msnobj Creator="yycnet@hotmail.com" Size="23427" Type="8"...
*/
/*	���һ���޷������꣬��������������
		 Message-ID: {E1255EF3-88D3-4270-A9AB-294686282F41}\r\n    //��ʾ�˿�Ϊ�Ǹ�������Ϣ�ĺ�����
		 Chunk: 1												   //��ʾ�˿�Ϊ�Ǹ�������Ϣ�ĺ����ĵڼ��飬��Chunks-1��
		 \r\n
		 <��������>...

		 Message-ID: {E1255EF3-88D3-4270-A9AB-294686282F41}\r\n    //��ʾ�˿�Ϊ�Ǹ�������Ϣ�ĺ�����
		 Chunk: 2												   //��ʾ�˿�Ϊ�Ǹ�������Ϣ�ĺ����ĵڼ��飬��Chunks-1��
		 \r\n
		 <��������>...
*/
/*	��������Ϣ
		  MIME-Version: 1.0\r\n
		  Content-Type: text/x-msnmsgr-datacast\r\n
		  \r\n
		  ID: 1\r\n
		  \r\n\r\n
*/
//	}
	return 0;
}

/*
//ͷ���ȡ����
Sender                  SS                Recver
 <-------  ���ͻ�ȡͷ������ --------
  ---------Acknowledged Message----->
  -------   ����ͬ����Ӧ 200 OK ---->
  <--------Acknowledged Message------
  ------- ����׼������������Ϣ ----->
  <--------Acknowledged Message------             //���Ͷ˽��յ����ն˵ķ���׼��Ӧ�����ܿ�ʼ��������
  ------- ��������  ---------------->              field6 20 00 00 00
  <--------Acknowledged Message------              //���������ݽ�����Ϻ���һ��ACK��Ϣ
  <-------   Bye message ------------
  ---------Acknowledged Message----->			  //And finally if the Bye message is received by the SC and everything is fine, 
												  //it can send an Acknowledged Message back to the RC

  //BYE��Ϣ������RC(������/��������)����
  //the RC must send a Bye Message to the SC to say that the session can be closed.

�ļ���ȡ����
sender                                   Recver
  ------- �����ļ��������� ---------->
  <--------Acknowledged Message-------
  <------ ͬ����� 200 OK-------------
  ----------Acknowledged Message----->
  - invite(Direct-Connect handshake)->  ---				//Content-Type: application/x-msnmsgr-transreqbody
										  |  �˲�����п���
  <--- ... ...Handshake end.. ...--->   ---
  ------ �����ļ����� --------------->              field 30 00 00 01
  <--------Acknowledged Message-------              //���������ݽ�����Ϻ���һ��ACK��Ϣ
  -------   Bye message ------------->				//���Ͷ˻����һ��Bye��Ϣ
  --------- has sended bye ---------->              //filed6 40 00 000 00 ��Got byeͬ���ṹ
  <-------- has got bye --------------				//filed6 40 00 000 00
  
*/

//����msnc1Э�� - ����msnp2p����
void msnMessager :: msnc1_parse(cContactor *pcon,const char *msg_email,unsigned char *pBinarystuff,
								char *ptrmsg,unsigned long lfooter)
{ //�Ƚ���48�ֽ�Binary stuff
	//The first field is a DWORD and is the SessionID, which is zero when the Clients are negotiating about the session
	long sessionID=*((long *)pBinarystuff);
	//The second field is a DWORD and is the Identifier which identifies the message, the first message you receive from the other Client is the BaseIndentifier, 
	//the other messages contains this Identifier +1 or -2 or something like that, the BaseIdentifier is random generated. The Identifier can be in range from 4 to a max of 4294967295, I think.
	unsigned long messageID=*((unsigned long *)(pBinarystuff+4));
	unsigned long dataOffset=*((unsigned long *)(pBinarystuff+8));//�����Ӧ��8�ֽڳ�
	unsigned long totalSize=*((unsigned long *)(pBinarystuff+16));//�����Ӧ��8�ֽڳ�
	unsigned long dataMessageSize=*((unsigned long *)(pBinarystuff+24));
	//The sixth field is a DWORD and is the Flag field, it's 0x0 when no flags are specified, 
	//0x2 if it's an reply to a received message, 0x8 if there is an error on the binary level, 
	//0x20 when the data is for User Display Images or Emoticons, 0x40 --- sended bye����got bye
	//0x01000030 if it's the data of a file.
	unsigned long dwFlags=*((unsigned long *)(pBinarystuff+28));
	//The seventh field is a DWORD and is an important field, if the SessionID field is zero and the data doesn't contain the SessionID then this field contains the Identifier of the previous received message, 
	//most of the time the Flag field is 0x2 then. If the data contains the SessionID or if the SessionID field is non-zero then this field is just some random generated number.
	unsigned long field7=*((unsigned long *)(pBinarystuff+32));
	
//	RW_LOG_PRINT(LOGLEVEL_DEBUG,"[msnc1] sessionID=0x%x,messageID=0x%x,MessageSize=0x%x,dataOffset=0x%x,totalSize=0x%x,flags=0x%x,field7=0x%x,footer=0x%x\r\n",
//		sessionID,messageID,dataMessageSize,dataOffset,totalSize,dwFlags,field7,lfooter);
//	if(lfooter==0) RW_LOG_PRINT(LOGLEVEL_DEBUG,"[msnc1] msg=%s.\r\n",ptrmsg);

	//��Э����˵�������lfooter=0x00 ---Э�̹��� 0x01 --- for User Display Images and Emoticons, 0x02 --- for File Transfers
	
	//ֻ�� "got bye"��Ϣ��ACKӦ����Ϣ ��dataMessageSize=0
	if(dwFlags==0x02) return; //�յ�һ��Ӧ����Ϣ.���ԣ������κδ���
	if(dataMessageSize==0) return; //�յ�һ��0������Ϣ�������κδ���Ʃ��GOT/SENDED_BYE��Ϣ��
	
	if(lfooter!=0) //���ڴ���Ự״̬����ʱsessionID��Ϊ0
	{
		if(sessionID==0) return; //{ printf("aaaaErr: lfooter=%d, sessionID==0\r\n",lfooter);	return; }
		std::map<long,cMsnc1 *>::iterator it=pcon->m_msnc1Maps.find(sessionID);
		cMsnc1 *pmsnc1=(it==pcon->m_msnc1Maps.end())?NULL:(*it).second;
		if(pmsnc1==NULL) return; //{ printf("aaaaErr: sessionID=0x%x,pmsnc1=NULL\r\n",sessionID);	return; }
		
		if(lfooter==MSNINVITE_TYPE_PICTURE) //0x01) //User Display Images and Emoticons
		{
			if(dwFlags==0) //׼����ʼ����Display Images and Emoticons���ݣ�ע�����
			{//���ļ�׼��д
				pmsnc1->beginWrite();
				pmsnc1->sendmsg_ACK(pBinarystuff); //��ӦACK��Ϣ
				return;
			}
			else if(dwFlags==0x20) //Display Images and Emoticons����
			{
				pmsnc1->writeFile(ptrmsg,dataMessageSize);
				if((dataMessageSize+dataOffset)<totalSize) return;//����δ�������
			}//?else if(dwFlags==0x20)
		}//?if(lfooter==0x01)
		else if(lfooter==MSNINVITE_TYPE_FILE) //0x02//for File Transfers
		{
			if(dwFlags==0x01000030) //�ļ����ݰ�
			{//����Ϣ���������ļ���������.totalSize���ļ���С
				pmsnc1->writeFile(ptrmsg,dataMessageSize);
				if((dataMessageSize+dataOffset)<totalSize) return;//����δ�������
			}//?if(dwFlags==0x01000030) //�ļ����ݰ�
		}//?else if(lfooter==0x02)
/*		else if(lfooter==MSNINVITE_TYPE_ROBOT)
		{//ptrmsg��ʽ:
			return;
		} */
		pmsnc1->endWrite();//д�ļ�����
		pmsnc1->sendmsg_ACK(pBinarystuff); //��ӦACK��Ϣ

		//yyc add 2006-05-19
		onInvite((HCHATSESSION)pcon,pmsnc1->inviteType(),MSNINVITE_CMD_COMPLETED,pmsnc1);

		pcon->m_msnc1Maps.erase(it);
		if(lfooter==0x01){ 
			pmsnc1->sendmsg_BYE(); 
			std::map<std::string,cMsncx *>::iterator it1=pcon->m_msncxMaps.find(pmsnc1->m_callID);
			if(it1!=pcon->m_msncxMaps.end()){ pcon->m_msncxMaps.erase(it1); delete pmsnc1; }
		}//?if(lfooter==0x01)
		return;
	}//?if(lfooter!=0)

	if(sessionID!=0) return;//{ printf("aaaaErr: lfooter=0, sessionID==0x%x\r\n",sessionID);	return; }

	//------------------------------------------------------------------------------------------
	//----------------------------������Ϣ------------------------------------------------------
	//��֤������������Ϣ������������һ����������Ϣ
	if(dataOffset!=0)
	{
		if(pcon->m_buffer.size()<totalSize) return;
		::memcpy(pcon->m_buffer.str()+pcon->m_buffer.len(),ptrmsg,dataMessageSize);
		pcon->m_buffer.len()+=dataMessageSize;
		if((dataMessageSize+dataOffset)<totalSize) return;
		ptrmsg=pcon->m_buffer.str(); 
		dataOffset=0; dataMessageSize=totalSize;
	}
	else if(dataMessageSize<totalSize)
	{//��Ϣδ������
		if(pcon->m_buffer.size()<totalSize){
			pcon->m_buffer.Resize(0);
			pcon->m_buffer.Resize(totalSize);
		}
		if(pcon->m_buffer.size()!=0){
			::memcpy(pcon->m_buffer.str(),ptrmsg,dataMessageSize);
			pcon->m_buffer.len()=dataMessageSize;
		}
		return;
	}//?else if(dataMessageSize<totalSize)
	
	const char *ptr_CallID=NULL;//ָ��Call-ID
	const char *ptr_ContentType=NULL;//ָ��Content-Type
	const char *ptr_Context=NULL; int ptr_Context_len=0;
	const char *ptr_SessionID=NULL;//ָ�� SessionID
	const char *ptr_AppID=NULL;
	const char *ptr_branch=NULL;
	//��ʼ������Ϣ-----------start-------------------------
	char *tmpptr,*ptr,*pStart=ptrmsg;
	while( (ptr=strchr(pStart,'\r')) )
	{
		*ptr=0;
		if( (tmpptr=strchr(pStart,':')) )
		{
			*tmpptr=0;
			if(strcmp(pStart,"Call-ID")==0)
				ptr_CallID=tmpptr+2;
			else if(strcmp(pStart,"Content-Type")==0)
				ptr_ContentType=tmpptr+2;
			else if(strcmp(pStart,"SessionID")==0)
				ptr_SessionID=tmpptr+2;
			else if(strcmp(pStart,"AppID")==0)
				ptr_AppID=tmpptr+2;
			else if(strcmp(pStart,"Via")==0)
			{
				if( (ptr_branch=strstr(tmpptr+2,";branch=")) ) ptr_branch+=8;
			}
			else if(strcmp(pStart,"Context")==0)
			{
				ptr_Context=tmpptr+2; //Context�Ǿ���base64�����Ҫ����
				ptr_Context_len=strlen(ptr_Context);
				ptr_Context_len=cCoder::base64_decode((char *)ptr_Context,ptr_Context_len,(char *)ptr_Context);
				*((char *)ptr_Context+ptr_Context_len)=0;
//				RW_LOG_PRINT(LOGLEVEL_DEBUG,"Context Base64-decode(%d): %s.\r\n",ptr_Context_len,ptr_Context);
//				for(int ii=0;ii<ptr_Context_len;ii++){	printf("0x%x ",*((unsigned char *)ptr_Context+ii)); if(((ii+1)%16)==0) printf("\r\n");} 
			}
		}//?if( (tmpptr=strchr(pStart,':')) )
		pStart=ptr+1; while(*pStart=='\r' || *pStart=='\n') pStart++; //����\r\n
	}//?while(...
	//������Ϣ����----------- end -------------------------
	if(ptr_ContentType==NULL) return;//{ printf("aaaaErr: ptr_ContentType=NULL.\r\n");	return; }
	if(ptr_CallID==NULL) return;//{ printf("aaaaErr: sessionID==0 && ptr_CallID==NULL.\r\n");	return; }

	cMsnc1 *pmsnc1=NULL;
	std::map<std::string,cMsncx *>::iterator it=pcon->m_msncxMaps.find(ptr_CallID);
	if(it!=pcon->m_msncxMaps.end()) pmsnc1=(cMsnc1 *)(*it).second;
	if(pmsnc1) pmsnc1->sendmsg_ACK(pBinarystuff); //��ӦACK��Ϣ
	if(strncmp(ptrmsg,"BYE ",4)==0)
	{
		if(pmsnc1==NULL) return;
		RW_LOG_PRINT(LOGLEVEL_DEBUG,0,"[msnc1] Received BYE of x-msnmsgrp2p\r\n");
		pmsnc1->sendmsg_Got_BYE();
		sessionID=atol(pmsnc1->m_sessionID.c_str());

		pcon->m_msncxMaps.erase(it); delete pmsnc1;
		std::map<long,cMsnc1 *>::iterator it1=pcon->m_msnc1Maps.find(sessionID);
		if(it1!=pcon->m_msnc1Maps.end()) pcon->m_msnc1Maps.erase(it1);
	}
	else if(strncmp(ptrmsg,"MSNSLP/1.",9)==0)
	{
		if(pmsnc1==NULL) return;
		int respcode=atoi(ptrmsg+11);
		sessionID=atol(pmsnc1->m_sessionID.c_str());
		RW_LOG_PRINT(LOGLEVEL_DEBUG,"[msnc1] Received response of x-msnmsgrp2p,respcode=%d\r\n",respcode);
		if(respcode==200){ //�ɹ�������Ӧ
			pcon->m_msnc1Maps[sessionID]=pmsnc1;
			onInvite((HCHATSESSION)pcon,pmsnc1->inviteType(),MSNINVITE_CMD_ACCEPT,pmsnc1);
			if(pmsnc1->inviteType()==MSNINVITE_TYPE_FILE)
				m_threadpool.addTask((THREAD_CALLBACK *)&cMsnc1::sendThread,(void *)pmsnc1,THREADLIVETIME);
		}///if(respcode==200)
		else {
			onInvite((HCHATSESSION)pcon,pmsnc1->inviteType(),MSNINVITE_CMD_REJECT,pmsnc1);
			pcon->m_msncxMaps.erase(it);
			std::map<long,cMsnc1 *>::iterator it1=pcon->m_msnc1Maps.find(sessionID);
			if(it1!=pcon->m_msnc1Maps.end()) pcon->m_msnc1Maps.erase(it1);
			delete pmsnc1;
		}
	}//?else if(strncmp(ptrmsg,"MSNSLP/1.",9)==0)
	else if(strncmp(ptrmsg,"INVITE ",7)==0)
	{
		if(strcmp(ptr_ContentType,"application/x-msnmsgr-sessionreqbody")==0)
		{
			if(ptr_AppID==NULL || ptr_Context==NULL) return;
			int inviteTypeID=atoi(ptr_AppID);
			if( (pmsnc1=new cMsnc1(this,pcon,inviteTypeID))==NULL ) return;	
			if(ptr_SessionID) pmsnc1->m_sessionID.assign(ptr_SessionID);
			if(ptr_branch) pmsnc1->m_branch.assign(ptr_branch);
			pmsnc1->m_callID.assign(ptr_CallID);
			pmsnc1->sendmsg_ACK(pBinarystuff); //��ӦACK��Ϣ
			if(inviteTypeID==MSNINVITE_TYPE_PICTURE) //ĳ���û����������ȡ���ʺŵ�ͷ��
				pmsnc1->m_offsetIdentifier-=3;
			
			//����ptr_Context������
			long filesize=0;//�����ļ���С
			if(inviteTypeID==MSNINVITE_TYPE_FILE)
			{//�ļ��������󣬽���Ҫ������ļ������ļ���С����cmsnc1::sendFile����
				filesize=*((long *)(ptr_Context+8));
				//�ļ�����unicode����
				int len=WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK|WC_DISCARDNS|WC_SEPCHARS|WC_DEFAULTCHAR,
					(unsigned short *)(ptr_Context+20),-1,(char *)ptr_Context,ptr_Context_len,NULL,NULL);
				*((char *)ptr_Context+len)=0;
				//�����ļ����ʹ�С���Ա�onInvite�¼�����ͨ��msncx�����ȡ�ļ����ʹ�С�������Ƿ����
				pmsnc1->filename().assign(ptr_Context);
				pmsnc1->filesize(filesize);
			}
/*			else if(inviteTypeID==MSNINVITE_TYPE_CAM)
			{//context��������unicode�����UID�ַ�����ʽ������{4BD96FC0-AB17-4425-A14A-439185962DC8}
				int len=WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK|WC_DISCARDNS|WC_SEPCHARS|WC_DEFAULTCHAR,
					(unsigned short *)(ptr_Context),-1,(char *)ptr_Context,ptr_Context_len,NULL,NULL);
				*((char *)ptr_Context+len)=0;
			}//?else if(inviteTypeID==INVITE_TYPE_CAM)
			else if(inviteTypeID==MSNINVITE_TYPE_ROBOT) //����������
			{//context��������unicode�����<��������>;1;<����������>
				int len=WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK|WC_DISCARDNS|WC_SEPCHARS|WC_DEFAULTCHAR,
					(unsigned short *)(ptr_Context),-1,(char *)ptr_Context,ptr_Context_len,NULL,NULL);
				*((char *)ptr_Context+len)=0;
			}
*/			
			RW_LOG_PRINT(LOGLEVEL_DEBUG,"[msnc1] Received INVITE of x-msnmsgrp2p, type=%d\r\n",inviteTypeID);
			bool bAccept=onInvite((HCHATSESSION)pcon,inviteTypeID,
									MSNINVITE_CMD_INVITE,pmsnc1);
			if(bAccept)
			{
				pmsnc1->sendmsg_ACCEPT();//��������
				bool bValid=false;
				if(inviteTypeID==MSNINVITE_TYPE_PICTURE)
				{//׼�����ͱ��ʺŵ�ͷ������,context��������û����mime�����msnobj�����ַ���
					if( (bValid=pmsnc1->sendPicture(m_photofile.c_str())) )
						m_threadpool.addTask((THREAD_CALLBACK *)&cMsnc1::sendThread,(void *)pmsnc1,THREADLIVETIME);
				}
				else if(inviteTypeID==MSNINVITE_TYPE_FILE)
				{	//�Է����ܽ���Direct-Connect handshake
					//�������ȷ���һ��content-type==application/x-msnmsgr-transreqbody��invite
					if( (bValid=pmsnc1->beginWrite(ptr_Context,filesize)) )
					{
						sessionID=atol(pmsnc1->m_sessionID.c_str());
						pcon->m_msnc1Maps[sessionID]=pmsnc1;
					}
				}//?else if(inviteTypeID==INVITE_TYPE_FILE)
/*				else if(inviteTypeID==MSNINVITE_TYPE_ROBOT) //����������
				{
					RW_LOG_PRINT(LOGLEVEL_INFO,"[msnc1] Robot invite,context=%s\r\n",ptr_Context);
				} */
				else
					RW_LOG_PRINT(LOGLEVEL_INFO,"[msnc1] unknowed invite,AppID=%d\r\n",inviteTypeID);	
				if(bValid){ pcon->m_msncxMaps[ptr_CallID]=pmsnc1; pmsnc1=NULL; }
			}//?if(bAccept)
			else
				pmsnc1->sendmsg_REJECT();//�ܾ�����
			delete pmsnc1; return;
		}//?f(strcmp(ptr_ContentType,...
		else
		{
			if(pmsnc1==NULL) return;//{ printf("aaaaErr: pmsnc1==NULL.\r\n");	return; }
/*			if(strcmp(ptr_ContentType,"application/x-msnmsgr-transreqbody")==0)
			{//Direct-Connect handshake INVITE
				//��֧��ֱ������
			}
			else if(strcmp(ptr_ContentType,"application/x-msnmsgr-transrespbody")==0)
			{
			}
*/
		}//?f(strcmp(ptr_ContentType,...else...
	}//?else if(strncmp(ptrmsg,"INVITE ",7)==0)
	
	return;
}

//����msnc0��ϢЭ��
/*
MSG yycnet@hotmail.com yyc:) 29\r\n
MSG len=293, MIME-Version: 1.0\r\n
Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n
\r\n
Application-Name: NetMeeting\r\n
Application-GUID: {44BBA842-CC51-11CF-AAFA-00AA00B6015C}\r\n
Session-Protocol: SM1\r\n
Invitation-Command: INVITE\r\n
Invitation-Cookie: 25402056\r\n
Session-ID: {C4E9035F-CCEB-40F0-8F17-135FB734073B}\r\n
\r\n\r\n
*/
/*   ��Ƶ����
MSG yycnet@hotmail.com yyc:) 491\r\n
MSG len=491, MIME-Version: 1.0\r\n
Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n
\r\n
Application-Name: 音频对话\r\n
Application-GUID: {02D3C01F-BF30-4825-A83A-DE7AF41648AA}\r\n
Session-Protocol: SM1\r\n
Context-Data: Requested:SIP_A,;Capabilities:SIP_A,;\r\n
Invitation-Command: INVITE\r\n
Avm-Support: 7\r\n
Avm-Request: 2\r\n
Invitation-Cookie: 26216456\r\n
Session-ID: {2175E8D4-7CAA-49DD-A520-C2786E891F6F}\r\n
Conn-Type: IP-Restrict-NAT\r\n
Sip-Capability: 1\r\n
Public-IP: 61.237.235.88\r\n
Private-IP: 192.168.0.12\r\n
UPnP: TRUE\r\n
\r\n\r\n
*/
void msnMessager :: msnc0_parse(cContactor *pcon,const char *msg_email,char *ptrmsg)
{
	std::map<std::string,cMsncx *> &msncxMaps=pcon->m_msncxMaps;

	const char *ptr_InviteCommand=NULL;//ָ��Invitation-Command
	const char *ptr_InviteCookie=NULL;//ָ��Invitation-Cookie
	const char *ptr_ApplicationName=NULL;//ָ��Application-Name ������������
	const char *ptr_ApplicationGUID=NULL;//ָ��Application-GUID �������͵�UID
	
	const char *ptr_Connectivity=NULL;//ָ��Connectivity ���뷢�����Ƿ���ֱ�����ӣ������ڷ���ǽ����
	const char *ptr_ApplicationFile=NULL;//ָ��Application-File �ļ�������ļ���
	const char *ptr_ApplicationFileSize=NULL;//ָ��Application-FileSize �ļ�����Ĵ�С

	//�������룬�������ӵ�IP�Ͷ˿�
	const char *ptr_IPAddress=NULL;//ָ��IP-Address
	const char *ptr_Port=NULL;//ָ��Port
	const char *ptr_IPAddress_Internal=NULL;//ָ��IP-Address
	const char *ptr_PortX=NULL;//ָ��Port

	const char *ptr_AuthCookie=NULL;//ָ��AuthCookie
	//�ܾ�����
	const char *ptr_CancelCode=NULL; //ָ��Cancel-Code���ܾ���ԭ��
	
//	RW_LOG_PRINT(LOGLEVEL_DEBUG,"[msnc0] %s.\r\n",ptrmsg);
	//��ʼ������Ϣ-----------start-------------------------
	char *tmpptr,*ptr,*pStart=ptrmsg;
	while( (ptr=strchr(pStart,'\r')) )
	{
		*ptr=0;
		if( (tmpptr=strchr(pStart,':')) )
		{
			*tmpptr=0;
			if(strcmp(pStart,"Invitation-Command")==0)
				ptr_InviteCommand=tmpptr+2;
			else if(strcmp(pStart,"Invitation-Cookie")==0)
				ptr_InviteCookie=tmpptr+2;
			else if(strcmp(pStart,"Application-Name")==0)
				ptr_ApplicationName=tmpptr+2;
			else if(strcmp(pStart,"Application-GUID")==0)
				ptr_ApplicationGUID=tmpptr+2;
			else if(strcmp(pStart,"Cancel-Code")==0)
				ptr_CancelCode=tmpptr+2;
			else if(strcmp(pStart,"Application-File")==0)
				ptr_ApplicationFile=tmpptr+2;
			else if(strcmp(pStart,"Application-FileSize")==0)
				ptr_ApplicationFileSize=tmpptr+2;
			else if(strcmp(pStart,"AuthCookie")==0)
				ptr_AuthCookie=tmpptr+2;
			else if(strcmp(pStart,"IP-Address")==0)
				ptr_IPAddress=tmpptr+2;
			else if(strcmp(pStart,"Port")==0)
				ptr_Port=tmpptr+2;
			else if(strcmp(pStart,"IP-Address-Internal")==0)
				ptr_IPAddress_Internal=tmpptr+2;
			else if(strcmp(pStart,"PortX")==0)
				ptr_PortX=tmpptr+2;
			else if(strcmp(pStart,"Connectivity")==0)
				ptr_Connectivity=tmpptr+2;
		}//?if( (tmpptr=strchr(pStart,':')) )
		pStart=ptr+1; while(*pStart=='\r' || *pStart=='\n') pStart++; //����\r\n
	}//?while(...
	//������Ϣ����----------- end -------------------------
	if(ptr_InviteCommand==NULL) return;
	if(ptr_InviteCookie==NULL) return;

	if(strcmp(ptr_InviteCommand,"INVITE")==0)
	{//��������
		if(ptr_ApplicationGUID==NULL) return;//GUID�������������
		int inviteType=MSNINVITE_TYPE_UNKNOW;
		if(strcmp(ptr_ApplicationGUID,"{5D3E02AB-6190-11d3-BBBB-00C04F795683}")==0)
			inviteType=MSNINVITE_TYPE_FILE;
		else if(strcmp(ptr_ApplicationGUID,"{44BBA842-CC51-11CF-AAFA-00AA00B6015C}")==0)
			inviteType=MSNINVITE_TYPE_NETMEET;
		else if(strcmp(ptr_ApplicationGUID,"{2175E8D4-7CAA-49DD-A520-C2786E891F6F}")==0)
			inviteType=MSNINVITE_TYPE_AUDIO;//��Ƶ����
		if(inviteType==MSNINVITE_TYPE_UNKNOW){
			RW_LOG_PRINT(LOGLEVEL_INFO,"[msnc0] unknowed invite,GUID=%s.\r\n",ptr_ApplicationGUID); 
			return;
		}
		cMsnc0 *pmsnc0=new cMsnc0(this,pcon,ptr_InviteCookie);
		if(pmsnc0==NULL) return;//{ printf("aaaaErr: new pmsnc0==NULL.\r\n");	return; }
		
		long filesize=0; std::string filename;
		if(inviteType==MSNINVITE_TYPE_FILE) //�ļ���������
		{
			filesize=(ptr_ApplicationFileSize)?atol(ptr_ApplicationFileSize):0;
			if(ptr_ApplicationFile){//����utf8����
				int len=cCoder::utf8_decode(ptr_ApplicationFile,strlen(ptr_ApplicationFile),(char *)ptr_ApplicationFile);
				*((char *)ptr_ApplicationFile+len)=0; filename.assign(ptr_ApplicationFile);
			}
		}//?if(inviteType==INVITE_TYPE_FILE)
		bool bAccept=onInvite((HCHATSESSION)pcon,inviteType,MSNINVITE_CMD_INVITE,pmsnc0);
		if(bAccept)
		{
			bool bListen=(m_Connectivity=='Y' && (ptr_Connectivity && strcmp(ptr_Connectivity,"N")==0) )?true:false;
			pmsnc0->sendmsg_ACCEPT(bListen);
			pmsnc0->beginWrite(filename.c_str(),filesize);
			//yyc add 2006-05-19 begin
			if( bListen && //�����������ȴ��Է�����
			    m_threadpool.addTask((THREAD_CALLBACK *)&cMsnc0::msnc0Thread,(void *)pmsnc0,THREADLIVETIME)!=0 )
				pmsnc0=NULL;
			else { msncxMaps[ptr_InviteCookie]=pmsnc0; pmsnc0=NULL; }
			//yyc add 2006-05-19 end
			//yyc remove 2006-05-19 begin 
//			msncxMaps[ptr_InviteCookie]=pmsnc0; pmsnc0=NULL;
//			if(bListen) //�����������ȴ��Է�����
//				m_threadpool.addTask((THREAD_CALLBACK *)&cMsnc0::msnc0Thread,(void *)pmsnc0,THREADLIVETIME)
			//yyc remove 2006-05-19 end
		}//?if(bAccept)
		else pmsnc0->sendmsg_REJECT("REJECT");
		delete pmsnc0; return;
	}//?if(strcmp(ptr_InviteCommand,"INVITE")==0)
	else if(strcmp(ptr_InviteCommand,"ACCEPT")==0)
	{//ȷ�Ͻ��յ�Ӧ��
		std::map<std::string,cMsncx *>::iterator it=msncxMaps.find(ptr_InviteCookie);
		if(it==msncxMaps.end()) return;
		cMsnc0 *pmsnc0=(cMsnc0 *)(*it).second;
		onInvite((HCHATSESSION)pcon,pmsnc0->inviteType(),MSNINVITE_CMD_ACCEPT,pmsnc0);
		bool bValid=false;
		if( (ptr_IPAddress_Internal || ptr_IPAddress) && ptr_Port )
		{
			const char *iphost=(ptr_IPAddress_Internal)?ptr_IPAddress_Internal:ptr_IPAddress;
			//����Ҫ���ӵĶԷ����ݴ��������������߳��첽����
			pmsnc0->setHostinfo(iphost,atoi(ptr_Port),ptr_AuthCookie);
			bValid=(m_threadpool.addTask((THREAD_CALLBACK *)&cMsnc0::msnc0Thread,(void *)pmsnc0,THREADLIVETIME)!=0);
		}//?if(ptr_IPAddress && ptr_Port )
		else if(pmsnc0->bSender() && m_Connectivity=='Y') //������������ߣ��ҶԷ�����Ӧû�е�ַ��Ϣ
		{//���ʺſ���������˿ڣ��������ݴ���
			pmsnc0->sendmsg_ACCEPT(true);//����һ���������ȴ��Է�����
			bValid=(m_threadpool.addTask((THREAD_CALLBACK *)&cMsnc0::msnc0Thread,(void *)pmsnc0,THREADLIVETIME)!=0);
		}
		if(!bValid){//��������
			pmsnc0->sendmsg_REJECT("FAIL");
			msncxMaps.erase(it); delete pmsnc0;
		}//?if(!bValid)
		//yyc add 2006-05-19 begin
		else
			msncxMaps.erase(it);
		//yyc add 2006-05-19 end
	}//?else if(strcmp(ptr_InviteCommand,"ACCEPT")==0)
	else if(strcmp(ptr_InviteCommand,"CANCEL")==0)
	{//�û��ܾ�������
		std::map<std::string,cMsncx *>::iterator it=msncxMaps.find(ptr_InviteCookie);
		if(it==msncxMaps.end()) return;
		cMsnc0 *pmsnc0=(cMsnc0 *)(*it).second;
		onInvite((HCHATSESSION)pcon,pmsnc0->inviteType(),MSNINVITE_CMD_REJECT,pmsnc0);
		msncxMaps.erase(it); delete pmsnc0;
	}//?else if(strcmp(ptr_InviteCommand,"CANCEL")==0)
	return;
}
