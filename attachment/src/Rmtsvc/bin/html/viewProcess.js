var autoRefresh=1000; //�Զ�ˢ��ʱ��ms
var imgLoaded=true; 

var curPid=0;
var curPname="";
var curHmdl=0;
var loadcount=0;

function processRequest() 
{
	if (xmlHttp.readyState == 4) { // �ж϶���״̬
		if (xmlHttp.status == 200) { // ��Ϣ�Ѿ��ɹ����أ���ʼ������Ϣ
			var xmlobj = xmlHttp.responseXML;
			var nodes=xmlobj.getElementsByTagName("pcname");
			document.all("lblName").innerText=nodes[0].firstChild.data;
			nodes=xmlobj.getElementsByTagName("OS");
			document.all("lblOS").innerText=nodes[0].firstChild.data;
			nodes=xmlobj.getElementsByTagName("CPU");
			document.all("lblCPU").innerText=nodes[0].firstChild.data;
			nodes=xmlobj.getElementsByTagName("status");
			document.all("lblSta").innerText=nodes[0].firstChild.data;
			nodes=xmlobj.getElementsByTagName("account");
			document.all("lblAccount").innerText=nodes[0].firstChild.data;
			nodes=xmlobj.getElementsByTagName("pid");
			document.all("lblProcess").innerText="pid:"+nodes[0].firstChild.data;
			curPid=nodes[0].firstChild.data;
			mlistXML.src="/mlist?pid="+curPid;
			
            } else alert("�����ҳ�����쳣");
        }
}

function loadImg()
{
	if(imgLoaded)
	{
		document.usageimage.src="/usageimage";
		imgLoaded=false;
	}
}

function Imgloaded()
{
	imgLoaded=true;
	if(autoRefresh>0) window.setTimeout("loadImg()",autoRefresh);
}

function ifRefresh(t)
{
	if(autoRefresh>0)
		autoRefresh=0;
	else
	{
		autoRefresh=t;
		loadImg();
	}
}

function window_onload()
{
	if(!oPopup) createpopup();
	if(!xmlHttp) createXMLHttpRequest();
	xmlHttp.open("GET", "/sysinfo", true);
	xmlHttp.onreadystatechange = processRequest;
    	xmlHttp.send(null);
    	loadImg();
}

function processClick(tblElement)
{
	var row=tblElement.rowIndex;
	plistXML.recordset.absoluteposition=row;
	curPid=plistXML.recordset("pid");
	curPname=plistXML.recordset("pname");
	document.all("lblProcess").innerText="pid:"+curPid+" - "+curPname;
	mlistXML.src="/mlist?pid="+plistXML.recordset("pid");
}

function moduleClick(tblElement)
{
	var row=tblElement.rowIndex;
	mlistXML.recordset.absoluteposition=row;
	curHmdl=mlistXML.recordset("hmdl");
	loadcount=mlistXML.recordset("usage");
	document.all("lblModule").innerText=mlistXML.recordset("mname");
}
//������ǰѡ�н���
function pkill()
{
	if(curPname!="" && confirm("ȷ�Ž�������"+curPname) )
	{
		xmlHttp.open("GET", "/pkill?pid="+curPid, false);
    		xmlHttp.send(null);
    		if(xmlHttp.responseText=="true")
    			plistXML.src='/plist';
    		else alert(xmlHttp.responseText);
    	}
}

function deattach()
{
	if(curPid==0 || curHmdl=="") return;
	if(loadcount<=0 || loadcount==65535)
		alert("����ж�ش�DLL");
	else if( confirm("ȷ��ж�ش�DLL") )
	{
		xmlHttp.open("GET", "/mdattach?pid="+curPid+"&hmdl="+curHmdl+"&count="+loadcount, false);
    		xmlHttp.send(null);
    		mlistXML.src="/mlist?pid="+curPid;
	}
}

function proFile()
{
	var qx=parent.frmLeft.userQX;
	var fpath=mlistXML.recordset("mname");
	if(fpath=="")
		alert("��ѡ��Ҫ�鿴���Ե��ļ�!");
	else
	{
		window.showModalDialog("proFile.htm",qx+","+fpath,"dialogHeight: 350px;dialogWidth: 400px;center: yes;resizable: no;scroll: no;status: no");
	}
}

//----------------���� func--------------------------
function sort(xmlObj, xslObj, sortByColName) 
{ 
var xmlData=eval("document.all."+xmlObj).XMLDocument;
var xslData=eval("document.all."+xslObj).XMLDocument;
var nodes=xslData.documentElement.selectSingleNode("xsl:for-each"); 
var s=nodes.selectSingleNode("@order-by").value;
if(s.substr(1)==sortByColName)
{
	if(s.charAt(0)=="+")
		s="-"+sortByColName;
	else s="+"+sortByColName;
}
else
	s="+"+sortByColName;
nodes.selectSingleNode("@order-by").value=s;

xmlData.documentElement.transformNodeToObject(xslData.documentElement,xmlData); 
} 
