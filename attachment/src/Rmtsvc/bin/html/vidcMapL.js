//vidcMapL.js

function disp_localip(node)
{
	if(node.length<=0) return;
	var oSelect=document.getElementById("selMapip");
	for(i=oSelect.options.length;i>1;i--) oSelect.options.remove(i-1);
	var localip=node.item(0).text;
	var ss=localip.split(" ");
	for(i=0;i<ss.length;i++)
	{
		var oOption = document.createElement("OPTION");
		oOption.text=ss[i];
		oOption.value=ss[i];
		oSelect.add(oOption);
	}
	oSelect.selectedIndex=0;
}

function disp_maplist(xmlobj)
{
	var oSelect=document.getElementById("selMapped");
	for(i=oSelect.options.length;i>1;i--) oSelect.options.remove(i-1);
	for(i=0;i<xmlobj.childNodes.length;i++)
	{
		var o=xmlobj.childNodes.item(i).attributes.item(0);
		var s=xmlobj.childNodes.item(i).text;
		var oOption = document.createElement("OPTION");
		oOption.text=o.nodeValue+" - " +s;
		oOption.value=o.nodeValue;
		oSelect.add(oOption);
	}
	oSelect.selectedIndex=0;
}

function disp_mapinfo(xmlobj)
{
	document.getElementById("btnRefresh").disabled=false;
	var mapped_port=0;
	var node=xmlobj.getElementsByTagName("svrport");
	if(node.length>0) mapped_port=node.item(0).text;
	if(mapped_port>0)
	{
		document.getElementById("btnStart").disabled=true;
		document.getElementById("btnStop").disabled=false;
		node=xmlobj.getElementsByTagName("ifssl");
		if(node.length>0 && node.item(0).text=="1")
		{
			var s="&nbsp;(����ͻ���֤����֤)";
			node=xmlobj.getElementsByTagName("ifsslv");
			if(node.length>0 && node.item(0).text=="1") s="&nbsp;(��Ҫ�ͻ���֤����֤)";
			document.getElementById("lblStatus").innerHTML="<font color=green>��</font>SSL���ܷ�����������... &nbsp;�˿�:"+mapped_port+s;
		}
		else	document.getElementById("lblStatus").innerHTML="<font color=green>��</font>������������... &nbsp;�˿�:"+mapped_port;
		node=xmlobj.getElementsByTagName("starttime");
		if(node.length>0) document.getElementById("lblRuntime").innerText=node.item(0).text;
		node=xmlobj.getElementsByTagName("connected");
		if(node.length>0) document.getElementById("lblConnected").innerText=node.item(0).text;
	}else{
		document.getElementById("btnStart").disabled=false;
		document.getElementById("btnStop").disabled=true;
		document.getElementById("lblStatus").innerHTML="<font color=red>��</font>����ֹͣ����";
		document.getElementById("lblRuntime").innerText="YYYY��MM��DD�� hh:mm:ss";
		document.getElementById("lblConnected").innerText="0";
	}
	
	if(document.getElementById("btnStop").disabled)
	{
		document.getElementById("btnSave").disabled=false;
		document.getElementById("btnDele").disabled=false;
	}else{
		document.getElementById("btnSave").disabled=true;
		document.getElementById("btnDele").disabled=true;
	}
	node=xmlobj.getElementsByTagName("appip");
	if(node.length>0) document.getElementById("lblAppIP").value=node.item(0).text;
	node=xmlobj.getElementsByTagName("appport");
	if(node.length>0) document.getElementById("lblAppPort").value=node.item(0).text;
	node=xmlobj.getElementsByTagName("appdesc");
	if(node.length>0) document.getElementById("lblAppDesc").value=node.item(0).text;
	node=xmlobj.getElementsByTagName("mapport");
	if(node.length>0) document.getElementById("lblMapport").value=node.item(0).text;
	node=xmlobj.getElementsByTagName("mapname");
	if(node.length>0) document.getElementById("lblMapname").value=node.item(0).text;
	node=xmlobj.getElementsByTagName("maxconn");
	if(node.length>0) document.getElementById("lblMaxconn").value=node.item(0).text;
	node=xmlobj.getElementsByTagName("maxratio");
	if(node.length>0) document.getElementById("lblMaxratio").value=node.item(0).text;
	
	node=xmlobj.getElementsByTagName("bindip");
	var bindip="";
	if(node.length>0) bindip=node.item(0).text;
	var oSelect=document.getElementById("selMapip");
	for(i=0;i<oSelect.options.length;i++)
	{
		if(oSelect.options[i].value==bindip){
			oSelect.options[i].selected=true;
			break;
		}
	}
	
	node=xmlobj.getElementsByTagName("ssltype");
	if(node.length>0){
		 if(node.item(0).text=="+ssl")
		 	document.getElementById("ssltype1").checked=true;
		 else if(node.item(0).text=="-ssl")
		 	document.getElementById("ssltype2").checked=true;
		 else document.getElementById("ssltype0").checked=true;
	}else document.getElementById("ssltype0").checked=true;
	
	if(document.getElementById("ssltype1").checked)
	{
		document.getElementById("sslverify0").disabled=false;
		document.getElementById("sslverify1").disabled=false;
	}
	
	node=xmlobj.getElementsByTagName("sslverify");
	if(node.length>0 && node.item(0).text=="1")
		document.getElementById("sslverify1").checked=true;
	else document.getElementById("sslverify0").checked=true;
	
	node=xmlobj.getElementsByTagName("clicert");
	if(node.length>0)
		document.getElementById("clicert").value=node.item(0).text;
	else document.getElementById("clicert").value="";
	
	node=xmlobj.getElementsByTagName("apptype");
	var apptype="UNK";
	if(node.length>0) apptype=node.item(0).text;
	if(apptype=="FTP")
		document.getElementById("apptype3").checked=true;
	else if(apptype=="WWW")
		document.getElementById("apptype2").checked=true;
	else if(apptype=="TCP")
		document.getElementById("apptype1").checked=true;
	else document.getElementById("apptype0").checked=true;
	
	node=xmlobj.getElementsByTagName("autorun");
	if(node.length>0 && node.item(0).text!=0) 
		document.getElementById("chkAutorun").checked=true;
	else document.getElementById("chkAutorun").checked=false;
		
	var ipfilter_xml=xmlobj.selectSingleNode("//ipfilter")
	if(ipfilter_xml!=null && ipfilter_xml.childNodes.length>0)
	{
		node=ipfilter_xml.getElementsByTagName("ipaddr");
		document.getElementById("lblIPAddr").value=node.item(0).text;
		node=ipfilter_xml.getElementsByTagName("access");
		if(node.length>0 && node.item(0).text=="1")
		{
			document.getElementById("chkIPAccess1").checked=true;
			document.getElementById("chkIPAccess0").checked=false;
		}else{
			document.getElementById("chkIPAccess1").checked=false;
			document.getElementById("chkIPAccess0").checked=true;
		}
	}
}

function processRequest() 
{
	if (xmlHttp.readyState == 4) { // �ж϶���״̬
		if (xmlHttp.status == 200) { // ��Ϣ�Ѿ��ɹ����أ���ʼ������Ϣ
			
			var xmlobj = xmlHttp.responseXML;
			var localip=xmlobj.getElementsByTagName("localip");
			if(localip!=null) disp_localip(localip);
			
			var maplist=xmlobj.selectSingleNode("//maplist")
			if(maplist!=null) disp_maplist(maplist);
				
			var mapinfo=xmlobj.selectSingleNode("//mapinfo")
			if(mapinfo!=null && mapinfo.childNodes.length>0)
				disp_mapinfo(mapinfo);
				
			var retmsg=xmlobj.getElementsByTagName("retmsg");
    			if(retmsg.length>0)
				alert(retmsg.item(0).text);
            	} //else alert("�����ҳ�����쳣,status="+xmlHttp.status);
            	hidePopup();
        }
}

function initMapinfo()
{
	document.getElementById("btnRefresh").disabled=true;
	document.getElementById("btnStart").disabled=true;
	document.getElementById("btnStop").disabled=true;
	
	document.getElementById("lblStatus").innerHTML="<font color=red>��</font>����ֹͣ����";
	document.getElementById("lblRuntime").innerText="YYYY��MM��DD�� hh:mm:ss";
	document.getElementById("lblConnected").innerText="0";
	
	document.getElementById("btnSave").disabled=false;
	document.getElementById("btnDele").disabled=true;
	
	document.getElementById("lblAppIP").value="";
	document.getElementById("lblAppPort").value="";
	document.getElementById("lblAppDesc").value="";
	document.getElementById("lblMapport").value="";
	document.getElementById("lblMapname").value="";
	document.getElementById("lblMaxconn").value="0";
	document.getElementById("lblMaxratio").value="0";
	document.getElementById("selMapip").selectedIndex=0;
	
	document.getElementById("ssltype0").checked=true;
	document.getElementById("apptype0").checked=true;
	if(document.getElementById("tcpudp0").checked)
	{
		document.getElementById("ssltype0").disabled=false;
		document.getElementById("ssltype1").disabled=false;
		document.getElementById("ssltype2").disabled=false;
		document.getElementById("apptype0").disabled=false;
		document.getElementById("apptype1").disabled=false;
		document.getElementById("apptype2").disabled=false;
		document.getElementById("apptype3").disabled=false;
	}
	else
	{
		document.getElementById("ssltype0").disabled=true;
		document.getElementById("ssltype1").disabled=true;
		document.getElementById("ssltype2").disabled=true;
		document.getElementById("apptype0").disabled=true;
		document.getElementById("apptype1").disabled=true;
		document.getElementById("apptype2").disabled=true;
		document.getElementById("apptype3").disabled=true;
	}
	document.getElementById("sslverify0").checked=true;
	document.getElementById("sslverify0").disabled=true;
	document.getElementById("sslverify1").disabled=true;
	document.getElementById("clicert").value="";
		
	document.getElementById("chkAutorun").checked=false;
	document.getElementById("chkIPAccess1").checked=true;
	document.getElementById("lblIPAddr").value="";
}

function tcpudp_sel()
{
	var oSelect=document.getElementById("selMapped");
	for(i=oSelect.options.length;i>1;i--) oSelect.options.remove(i-1);
	initMapinfo();
	
	showPopup(250, 200, 150, 20);
	var stype="tcp";
	if(document.getElementById("tcpudp1").checked) stype="udp";
	xmlHttp.open("GET", "/mportL?cmd=list&type="+stype, true);
	xmlHttp.onreadystatechange = processRequest;
	xmlHttp.send( null );
}

function ssltype_sel()
{
	if(document.getElementById("ssltype1").checked)
	{
	document.getElementById("sslverify0").disabled=false;
	document.getElementById("sslverify1").disabled=false;
	}
	else if(document.getElementById("ssltype2").checked)
	{
	document.getElementById("sslverify0").disabled=true;
	document.getElementById("sslverify1").disabled=true;
	var scert=document.getElementById("clicert").value;
	var new_clicert=prompt("���Ӧ�÷�����Ҫ���пͻ�֤����֤,���ڴ�����ͻ���֤��(PEM��ʽ)\r\n��ʽ:<֤��>,<˽Կ>,<����> ����: client.pem,client.key,1234",scert);
	if(new_clicert!=null) document.getElementById("clicert").value=new_clicert;
	}else{
	document.getElementById("sslverify0").disabled=true;
	document.getElementById("sslverify1").disabled=true;
	}
}

function window_onload()
{
	if(!oPopup) createpopup();   	
	if(!xmlHttp) createXMLHttpRequest();
	tcpudp_sel();
}

function mapped_info()
{
	var oSelect=document.getElementById("selMapped");
	if(oSelect.selectedIndex==0)
		alert("��ѡ��ӳ�����ˢ��״̬!");
	else{
		var stype="tcp";
		if(document.getElementById("tcpudp1").checked) stype="udp";
		showPopup(250, 200, 150, 20);
		//��ȡָ��ӳ��˿ڵ���Ϣ
		xmlHttp.open("GET", "/mportL?cmd=info&type="+stype+"&mapname="+oSelect[oSelect.selectedIndex].value, true);
		xmlHttp.onreadystatechange = processRequest;
		xmlHttp.send( null );
	}
}

function mapped_change(el)
{
	initMapinfo();
	if(el.selectedIndex==0) return;
	mapped_info();
}

function mapped_start()
{
	var oSelect=document.getElementById("selMapped");
	if(oSelect.selectedIndex==0)
		alert("��ѡ��Ҫ������ӳ�����!");
	else{
		var stype="tcp";
		if(document.getElementById("tcpudp1").checked) stype="udp";
		showPopup(250, 200, 150, 20);
		xmlHttp.open("GET", "/mportL?cmd=start&type="+stype+"&mapname="+oSelect[oSelect.selectedIndex].value, true);
		xmlHttp.onreadystatechange = processRequest;
		xmlHttp.send( null );
	}
}

function mapped_stop()
{
	var oSelect=document.getElementById("selMapped");
	if(oSelect.selectedIndex==0)
		alert("��ѡ��Ҫֹͣ��ӳ�����!");
	else{
		var stype="tcp";
		if(document.getElementById("tcpudp1").checked) stype="udp";
		showPopup(250, 200, 150, 20);
		xmlHttp.open("GET", "/mportL?cmd=stop&type="+stype+"&mapname="+oSelect[oSelect.selectedIndex].value, true);
		xmlHttp.onreadystatechange = processRequest;
		xmlHttp.send( null );
	}
}

function mapped_dele()
{
	var oSelect=document.getElementById("selMapped");
	if(oSelect.selectedIndex==0)
		alert("��ѡ��Ҫɾ����ӳ�����!");
	else if(confirm("ȷ��ɾ��ӳ�� "+oSelect[oSelect.selectedIndex].value+"?"))
	{
		var stype="tcp";
		if(document.getElementById("tcpudp1").checked) stype="udp";
		var mapname=oSelect[oSelect.selectedIndex].value;
		initMapinfo();
		showPopup(250, 200, 150, 20);
		xmlHttp.open("GET", "/mportL?cmd=dele&type="+stype+"&mapname="+mapname, true);
		xmlHttp.onreadystatechange = processRequest;
		xmlHttp.send( null );
	}
}

function mapped_save()
{
	var strEncode=chkvalid();
	if(strEncode=="") return;
	
	var oSelect=document.getElementById("selMapped");
	var mapname=document.getElementById("lblMapname").value;
	mapname=mapname.toLowerCase();
	var idx=0;
	for(i=1;i<oSelect.options.length;i++)
	{
		var s=oSelect.options.item(i).value;
		if(mapname==s){ idx=i; break; }
	}
	if(idx==0)
	{
		if( !confirm("ȷ�����ӳ����� "+mapname+"?") ) return;
	}else{
		if( !confirm("ȷ���޸�ӳ����� "+mapname+"?") ) return;
	}
	var stype="tcp";
	if(document.getElementById("tcpudp1").checked) stype="udp";
	initMapinfo();
	if(stype=="udp"){ alert("���汾��ʱ��֧��udpӳ��"); return; }
	showPopup(250, 200, 150, 20);
	xmlHttp.open("POST", "/mportL?cmd=save&type="+stype, true);
	xmlHttp.setRequestHeader("Content-Type","application/x-www-form-urlencoded; charset=utf-8");
	xmlHttp.onreadystatechange = processRequest;
	xmlHttp.send( strEncode );
	return;
}

//��Ч�Լ��
function chkvalid()
{
	var rets="param=mtcpl ";
	var mapname=document.getElementById("lblMapname").value;
	var re_mapname=/^\w{1,20}$/;
	if( !re_mapname.test(mapname) ){
		alert("ӳ��������Ʋ���Ϊ�������20���ַ�\r\nֻ�ܰ���������ĸ A-Za-z0-9_");
		document.getElementById("lblMapname").focus();
		return "";
	}else rets=rets+"name="+mapname+" ";
	
	var sip=document.getElementById("lblAppIP").value;
	var sport=document.getElementById("lblAppPort").value;
	if(sport!="") sip=sip+":"+sport;
	//��������Ӧ�÷����ַ����ʽaaa.bb.cc:ddd,aaa1.bb1.cc1:dddd
	var re_sip=/^((\w+.?)+:\d+,?)+$/;
	if( !re_sip.test(sip) )
	{
		alert("Ҫӳ���Ӧ�÷����ַ�Ͷ˿����벻��ȷ"); 
		document.getElementById("lblAppIP").focus();
		return "";
	}else rets=rets+"appsvr="+sip+" ";
	
	var mport=document.getElementById("lblMapport").value;
	var re_mport=/^\d*-?\d*$/;
	if(!re_mport.test(mport))
	{
		alert("������Ϸ���ӳ��˿�");
		document.getElementById("lblMapport").focus();
		return "";
	}else rets=rets+"mport="+mport;
	if(document.getElementById("ssltype2").checked)
		rets=rets+"-ssl ";
	else if(document.getElementById("ssltype1").checked)
		rets=rets+"+ssl ";
	else rets=rets+" ";
	if(document.getElementById("ssltype1").checked && document.getElementById("sslverify1").checked)
		rets=rets+"sslverify=1 ";
	rets=rets+"bindip="+document.getElementById("selMapip").value+" ";
	
	if(document.getElementById("apptype3").checked)
		rets=rets+"apptype=FTP ";
	else if(document.getElementById("apptype2").checked)
		rets=rets+"apptype=WWW ";
	else if(document.getElementById("apptype1").checked)
		rets=rets+"apptype=TCP ";
	else rets=rets+"apptype=UNK ";
	
	var re_maxd=/^\d*$/;
	var maxconn=document.getElementById("lblMaxconn").value;
	if(!re_maxd.test(maxconn))
		rets=rets+"maxconn=0 ";
	else rets=rets+"maxconn="+maxconn+" ";
	var maxratio=document.getElementById("lblMaxratio").value;
	if(!re_maxd.test(maxratio))
		rets=rets+"maxratio=0 ";
	else rets=rets+"maxratio="+maxratio+" ";
	
	var appdesc=document.getElementById("lblAppDesc").value;
	var re_desc=/^[^&]*$/;
	if(!re_desc.test(appdesc))
	{
		alert("�����в��������&�ַ�");
		document.getElementById("lblAppDesc").focus();
		return "";
	}else rets=rets+"appdesc=\""+appdesc+"\"\r\n";
	
	if(document.getElementById("ssltype2").checked)
	{
		rets=rets+"sslc name="+mapname+" cert="+document.getElementById("clicert").value+" \r\n";
	}else rets=rets+"sslc name="+mapname+" cert= \r\n";
		
	rets=rets+"iprules type=mtcpl name="+mapname+" ";
	if(document.getElementById("chkAutorun").checked)
		rets=rets+"autorun=1 ";
	else	rets=rets+"autorun=0 ";
	
	if(document.getElementById("lblIPAddr").value!="")
	{
		var s=document.getElementById("lblIPAddr").value;
		var re=/^(((\d{1,3}|\*{1}).){3}(\d{1,3}|\*{1}),)*((\d{1,3}|\*{1}).){3}(\d{1,3}|\*{1}),?$/;
		if(!re.test(s))
		{
			alert("��������ȷ��IP���˵�ַ");
			return "";
		}
		if(document.getElementById("chkIPAccess1").checked==false && 
		   document.getElementById("chkIPAccess0").checked==false )
		{
		   	alert("������IP���˵ķ������/��ֹ");
		   	return "";
		}
		rets=rets+"ipaddr="+document.getElementById("lblIPAddr").value;
		if(document.getElementById("chkIPAccess1").checked)
			rets=rets+" access=1";
		else rets=rets+" access=0";
	}else rets=rets+"ipaddr= access=1";
	return rets;
}

