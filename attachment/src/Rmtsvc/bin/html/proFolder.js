
function processRequest() 
{
	if (xmlHttp.readyState == 4) { // �ж϶���״̬
		if (xmlHttp.status == 200) 
		{ // ��Ϣ�Ѿ��ɹ����أ���ʼ������Ϣ
			var xmlobj = xmlHttp.responseXML;
			var node = xmlobj.getElementsByTagName("fname");
			if(node.length>0)
				document.all("fname").innerText=node.item(0).text;
			node = xmlobj.getElementsByTagName("fpath");
			if(node.length>0)
				document.all("fpath").innerText=node.item(0).text;
			node = xmlobj.getElementsByTagName("fsize");
			if(node.length>0)
				document.all("fsize").innerText=node.item(0).text;
			node = xmlobj.getElementsByTagName("fsubs");
			if(node.length>0)
				document.all("fsubs").innerText=node.item(0).text;
			node = xmlobj.getElementsByTagName("fctime");
			if(node.length>0)
				document.all("fctime").innerText=node.item(0).text;
			node = xmlobj.getElementsByTagName("protype");
			if(node.length>0)
			{
				var pro=node.item(0).text;
				var pos=pro.indexOf('R');
				if(pos!=-1) document.all("chkRead").checked=true;
				pos=pro.indexOf('H');
				if(pos!=-1) document.all("chkHide").checked=true;
				pos=pro.indexOf('A');
				if(pos!=-1) document.all("chkAchi").checked=true;
			}
            	}
            	hidePopup();
        }
}
var spath="";
function window_onload()
{
	spath=window.dialogArguments;
	var qx=0;
	var p=spath.indexOf(',');
	if(p!=-1)
	{
		qx=spath.substr(0,p);
		spath=spath.substr(p+1);
	}
	if(!oPopup) createpopup();
	if(!xmlHttp) createXMLHttpRequest();
	xmlHttp.open("GET", "/profolder?path="+spath, true);
	xmlHttp.onreadystatechange = processRequest;
	xmlHttp.send( null );
	showPopup(100, 150, 150, 20);
	
	if((qx & ACCESS_FILE_ALL)==ACCESS_FILE_ALL)
	{
		document.all("chkRead").disabled=false;
		document.all("chkHide").disabled=false;
		document.all("chkAchi").disabled=false;
		document.all("btnApply").disabled=false;
	}
}


function mdProperty()
{
	showPopup(100, 150, 150, 20);
	
	var pf="";
	if(document.all("chkRead").checked) pf=pf+"R";
	if(document.all("chkHide").checked) pf=pf+"H";
	if(document.all("chkAchi").checked) pf=pf+"A";
	xmlHttp.open("GET", "/profolder?path="+spath+"&prof="+pf, true);
	xmlHttp.onreadystatechange = processRequest;
	xmlHttp.send( null );
}
