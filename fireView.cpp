// fireView.cpp : implementation of the CFireView class
//

#include "stdafx.h"
#include "fire.h"
#include "fireDoc.h"
#include "fireView.h"
#include "Sockutil.h"
#include <fstream>
#include <iostream>
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CFireView

IMPLEMENT_DYNCREATE(CFireView, CFormView)

BEGIN_MESSAGE_MAP(CFireView, CFormView)
	ON_BN_CLICKED(IDC_ADDRULE, OnAddrule)
	ON_BN_CLICKED(IDC_START, OnStart)
	ON_BN_CLICKED(IDC_BLOCKPING, OnBlockping)
	ON_BN_CLICKED(IDC_BLOCKALL, OnBlockall)
	ON_BN_CLICKED(IDC_ALLOWALL, OnAllowall)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_VIEWRULES, OnViewrules)
	ON_WM_SHOWWINDOW()
	ON_UPDATE_COMMAND_UI(ID_Start, OnUpdateStart)
	ON_COMMAND(ID_STOP, OnStop)
	ON_UPDATE_COMMAND_UI(ID_STOP, OnUpdateStop)
	ON_UPDATE_COMMAND_UI(ID_ALLOWALL, OnUpdateAllowall)
	ON_UPDATE_COMMAND_UI(ID_BLOCKALL, OnUpdateBlockall)
	ON_COMMAND(ID_Start, OnStart)
	ON_COMMAND(ID_BLOCKALL, OnBlockall)
	ON_COMMAND(ID_ALLOWALL, OnAllowall)
	ON_COMMAND(ID_BLOCKPING, OnBlockping)
	ON_UPDATE_COMMAND_UI(ID_BLOCKPING, OnUpdateBlockping)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFireView construction/destruction
//初始化界面状态
CFireView::CFireView()
	: CFormView(CFireView::IDD)
{
	//********************************************************
	m_pBrush = new CBrush;
	ASSERT(m_pBrush);
	m_clrBk = RGB(0x00,0x66,0x99);
	m_clrText = RGB(0xff,0xff,0x00);
	m_pBrush->CreateSolidBrush(m_clrBk);
	m_pColumns = new CStringList;
	ASSERT(m_pColumns);
	_rows = 1;
	start = TRUE;
	block = TRUE;
	allow = TRUE;
	ping = TRUE ;
}

CFireView::~CFireView()
{
	if (m_pBrush)
		delete m_pBrush;
}

void CFireView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_RESULT, m_cResult);
	DDX_Control(pDX, IDC_VIEWRULES, m_cvrules);
	DDX_Control(pDX, IDC_BLOCKPING, m_cping);
	DDX_Control(pDX, IDC_BLOCKALL, m_cblockall);
	DDX_Control(pDX, IDC_START, m_cstart);
}

BOOL CFireView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	//*****************************************************************
	
	m_filterDriver.LoadDriver("IpFilterDriver", "System32\\Drivers\\IpFltDrv.sys", NULL, TRUE);

	//we don't deregister the driver at destructor
	m_filterDriver.SetRemovable(FALSE);

	//we load the Filter-Hook Driver
	m_ipFltDrv.LoadDriver("DrvFltIp", NULL, NULL, TRUE);
	//****************************************************************
	return CFormView::PreCreateWindow(cs);
}

void CFireView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();
	m_parent = (CMainFrame*)GetParent();
	ShowHeaders();
}

/////////////////////////////////////////////////////////////////////////////
// CFireView diagnostics

#ifdef _DEBUG
void CFireView::AssertValid() const
{
	CFormView::AssertValid();
}

void CFireView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFireView message handlers

void CFireView::OnAddrule() 
{
	// TODO: Add your control notification handler code here
	m_Addrule.DoModal ();	
}


void CFireView::OnStart() 
{
	CString		_text;
	m_cstart.GetWindowText(_text);
	
	//Start响应事件
	if(_text != "Stop" )
	{
		if(m_ipFltDrv.WriteIo(START_IP_HOOK, NULL, 0) != DRV_ERROR_IO)
		{
			MessageBox("Firewall Started Sucessfully");
			start = FALSE;
			m_cstart.SetWindowText("Stop");
			m_parent ->SetOnlineLed(TRUE);
			m_parent ->SetOfflineLed(FALSE);
		}
	}

	//Stop响应事件
	else
	{
		if(m_ipFltDrv.WriteIo(STOP_IP_HOOK, NULL, 0) != DRV_ERROR_IO)
		{
			MessageBox("Firewall Stopped Succesfully");
			m_cstart.SetWindowText("Start");
			start = TRUE;
			m_parent ->SetOnlineLed(FALSE);
			m_parent ->SetOfflineLed(TRUE);
		}
	}	
}

//禁用所有的ICMP包
void CFireView::OnBlockping() 
{
	MessageBox("此功能需要你来实现！");
	// Your code
}

//禁用所有包
void CFireView::OnBlockall() 
{
	MessageBox("此功能需要你来实现！");
	// Your code
}

//启用所有包
void CFireView::OnAllowall() 
{
	MessageBox("此功能需要你来实现！");
	// Your code
}



//使用指定过滤规则
BOOL CFireView::ImplementRule(void)
{

	// MessageBox("此功能需要你来实现！");

	// Clear data: ListCtrl and filters
	ClearListCtrl();
	if (m_ipFltDrv.WriteIo(CLEAR_FILTER, NULL, 0) != DRV_ERROR_IO) {
		
	} else {
		MessageBox("Failed to clear filter");
		return FALSE;
	}
	
	// Read filters from file
	ReadFilters();
		
	// Show filters in Listctrl
	ShowFilters();

	// Add filters to drivers
	AddFilters();

	return TRUE;
}


//将字符串解析为filter特定格式

void CFireView:: ParseToIp(CString str)
{
	// Your code, please pay attention to the form of IP address and port!
}


//增加过滤规则表列
BOOL CFireView::AddColumn(LPCTSTR strItem,int nItem,int nSubItem,int nMask,int nFmt)
{
	LV_COLUMN lvc;
	lvc.mask = nMask;
	lvc.fmt = nFmt;
	lvc.pszText = (LPTSTR) strItem;
	lvc.cx = m_cResult.GetStringWidth(lvc.pszText) + 25;
	if(nMask & LVCF_SUBITEM)
	{
		if(nSubItem != -1)
			lvc.iSubItem = nSubItem;
		else
			lvc.iSubItem = nItem;
	}
	return m_cResult.InsertColumn(nItem,&lvc);
}

//增加过滤规则表一个元素
BOOL CFireView::AddItem(int nItem,int nSubItem,LPCTSTR strItem ,int nImageIndex)
{
	LV_ITEM lvItem;
	lvItem.mask = LVIF_TEXT;
	lvItem.iItem = nItem;
	lvItem.iSubItem = nSubItem;
	lvItem.pszText = (LPTSTR) strItem;

	if(nImageIndex != -1)
	{
		lvItem.mask |= LVIF_IMAGE;
		lvItem.iImage |= LVIF_IMAGE;
	}
	if(nSubItem == 0)
		return m_cResult.InsertItem(&lvItem);

	return m_cResult.SetItem(&lvItem);
}

void CFireView::AddHeader(LPTSTR hdr)
{
	if (m_pColumns)
		m_pColumns->AddTail(hdr);
}

void CFireView::ShowHeaders()
{
	int nIndex = 0;
	POSITION pos = m_pColumns->GetHeadPosition();
	while (pos)
	{
		CString hdr = (CString)m_pColumns->GetNext(pos);
		AddColumn(hdr,nIndex++);
	}
}

void CFireView::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CFormView::OnShowWindow(bShow, nStatus);
	AddHeader(_T("PROTOCOL"));
	AddHeader(_T("ACTION"));
	AddHeader(_T("Source IP"));
	AddHeader(_T("Dest IP"));
	AddHeader(_T("Source PORT"));
	AddHeader(_T("Dest PORT"));
	AddHeader(_T("Source MASK"));
	AddHeader(_T("Dest MASK"));
}

void CFireView::OnStop() 
{
	OnStart();	
}

void CFireView::OnUpdateStart(CCmdUI* pCmdUI) 
{	
	// TODO: Add your command update UI handler code here
	pCmdUI ->Enable(start);	
}

void CFireView::OnUpdateStop(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI ->Enable(!start);
}

void CFireView::OnUpdateAllowall(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI ->Enable(allow);
}

void CFireView::OnUpdateBlockall(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI ->Enable(block);
}

void CFireView::OnUpdateBlockping(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI ->Enable(ping);	
}

BOOL CFireView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}
//***********************************************************************

HBRUSH CFireView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);
	switch(nCtlColor)
	{
	case CTLCOLOR_BTN:
	case CTLCOLOR_STATIC:
		pDC->SetBkColor(m_clrBk);
		pDC->SetTextColor(m_clrText);
	case CTLCOLOR_DLG:
		return static_cast<HBRUSH>(m_pBrush->GetSafeHandle());
	}
	return CFormView::OnCtlColor(pDC,pWnd,nCtlColor);
}

void CFireView::OnViewrules() 
{
	ImplementRule();
}

// 清理ListCtrl的项目
void CFireView::ClearListCtrl()
{
	int nCount = m_cResult.GetItemCount();

	// Delete all of the items from the list view control.
	for (int i=0; i < nCount; i++)
	{
		m_cResult.DeleteItem(0);
	}
}

void CFireView::AddFilters()
{
	
}

void CFireView::ReadFilters()
{
	std::ifstream in("saved.rul");

	// Clear the queue of filters list.
	while(!m_filterList.empty()) m_filterList.pop();
	//filterList* ptr = m_filterList;
	//ptr->next = NULL;
	//ptr->ipf = NULL;
	while(!in.eof()) {
		char s;		// To take the beginning of the syntax;
		in >> s;
		//IPFilter* ipf;
		if(s!='{')	// File structure error
			return;
		else{
			//ipf = new IPFilter();
		}
		string element;
		while(in >> element) {
			/*if(element.find("protocol") != std::string::npos) {
				string value;
				in >> value;
				//ipf->protocol = (USHORT) atoi(value.c_str());
				m_filterList.push(value);
			}
			if(element.find("action") != std::string::npos) {
				string value;
				in >> value;
				ipf->drop = (value == "1");
			}
			if(element.find("saddr") != std::string::npos) {
				string value;
				in >> value;
				ipf->sourceIp = inet_addr(value.c_str());
			}
			if(element.find("daddr") != std::string::npos) {
				string value;
				in >> value;
				ipf->destinationIp = inet_addr(value.c_str());
			}
			if(element.find("sport") != std::string::npos) {
				string value;
				in >> value;
				ipf->sourcePort = htons(atoi(value.c_str()));
			}
			if(element.find("dport") != std::string::npos) {
				string value;
				in >> value;
				ipf->destinationPort = htons(atoi(value.c_str()));
			}
			if(element.find("smask") != std::string::npos) {
				string value;
				in >> value;
				ipf->sourceMask = inet_addr(value.c_str());
			}
			if(element.find("dmask") != std::string::npos) {
				string value;
				in >> value;
				ipf->destinationMask = inet_addr(value.c_str());
			}*/
			if(element == "}")
				break;
			else {
				string value;
				if(element.find("protocol") != std::string::npos) {
					USHORT v;	in >> v;
					value = ProtocolName(v);
				} else if(element.find("action") != std::string::npos) {
					int v; in >> v;
					value = v == 1?"DENY":"ALLOW";					
				} else {
					in >> value;
				}
				//MessageBox(value.c_str());
				m_filterList.push(value);
			}
		}
		
/*		ptr->ipf = *ipf;
		ptr->next = new filterList();
		ptr->next->ipf = NULL;
		ptr->next->next = NULL;
		ptr = ptr->next;*/
	}

}


void CFireView::ShowFilters()
{
// TODO:
	while(!m_filterList.empty())
	{
		int code = AddItem(0,0,m_filterList.front().c_str(),-1);
		m_filterList.pop();
		for (int i=1; i<8; i++){
			//MessageBox(m_filterList.front().c_str());
			AddItem(0, i, m_filterList.front().c_str(), -1);
			m_filterList.pop();
		}
	}
	//MessageBox("Finished");
}

string CFireView::ProtocolName(USHORT v)
{
	switch(v){
		case 1:		return string("ICMP");
		case 6:		return string("TCP");
		case 17:	return string("UDP");
		case 65535: return string("N/A");
	}
}

