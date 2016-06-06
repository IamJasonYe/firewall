// AddRuleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "fire.h"
#include "AddRuleDlg.h"
#include <stdlib.h>
//#include "DrvFltIp.h"
//********************************************************
#include <winsock2.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddRuleDlg dialog


CAddRuleDlg::CAddRuleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAddRuleDlg::IDD, pParent)
{
	m_sdadd = _T("");
	m_sdport = _T("");
	m_ssadd = _T("");
	m_ssport = _T("");
	ipFltDrv.LoadDriver("DrvFltIp", NULL, NULL, TRUE);	
}


void CAddRuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddRuleDlg)
	DDX_Control(pDX, IDC_COMBO2, m_protocol);
	DDX_Control(pDX, IDC_COMBO1, m_action);
	DDX_Text(pDX, IDC_DADD, m_sdadd);
	DDV_MaxChars(pDX, m_sdadd, 15);
	DDX_Text(pDX, IDC_DPORT, m_sdport);
	DDX_Text(pDX, IDC_SADD, m_ssadd);
	DDV_MaxChars(pDX, m_ssadd, 15);
	DDX_Text(pDX, IDC_SPORT, m_ssport);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddRuleDlg, CDialog)
	//{{AFX_MSG_MAP(CAddRuleDlg)
	ON_EN_KILLFOCUS(IDC_SADD, OnKillfocusSadd)
	ON_EN_KILLFOCUS(IDC_DADD, OnKillfocusDadd)
	ON_BN_CLICKED(IDC_ADDSAVE, OnAddsave)
	ON_EN_KILLFOCUS(IDC_SPORT, OnKillfocusSport)
	ON_EN_KILLFOCUS(IDC_DPORT, OnKillfocusDport)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

//**************************************************************************
//�����IPFilter������ָ��������
DWORD CAddRuleDlg::AddFilter(IPFilter pf)
{

	DWORD result = ipFltDrv.WriteIo(ADD_FILTER, &pf, sizeof(pf));
	if (result != DRV_SUCCESS) 
	{
		AfxMessageBox("Unable to add rule to the driver");
		return FALSE;
	}

	else
		return TRUE;
}

//*************************************************************************
//��������IP��ַ�Ƿ�Ƿ�

BOOL CAddRuleDlg::Verify(CString str)
{
	// Your code
	return inet_addr(str) != INADDR_NONE;
}


//*****************************************************************

void CAddRuleDlg::OnKillfocusSadd() 
{
	// TODO: Add your control notification handler code here
	UpdateData();
	BOOL bresult = Verify(m_ssadd);
	if(bresult == FALSE)
		MessageBox("Invalid IP Address");	
}

void CAddRuleDlg::OnKillfocusDadd() 
{
	// TODO: Add your control notification handler code here
	// This will check wether the IP address you had given
	// corresponds to a valid IP address or not. If not it
	// will prompt you for a valid IP address.

	UpdateData();
    BOOL bresult = Verify(m_sdadd);
	if(bresult == FALSE)
		MessageBox("Invalid IP Address");
	
}

//�������Ĺ���д���ļ� (�������)
void CAddRuleDlg::OnAddsave() 
{
	// Get and convert string from dialog components.

	CString protocal; 
	if(m_protocol.GetCurSel() != -1)
		protocal.Format(_T("%d"),ProtocolNum(m_protocol.GetCurSel()));
	else
		protocal = "-1";
	CString action; action.Format(_T("%d"), m_action.GetCurSel());
	CString smask, dmask;
	// Check the inputs
	CheckValidity(protocal, action, m_ssadd, m_sdadd, m_ssport, m_sdport, smask, dmask);
	
	if(NewFile()) { // Create File Successfully
		GotoEnd();
		IPFilter pf;
		pf.protocol			= ProtocolNum(m_protocol.GetCurSel());
		pf.sourceIp			= inet_addr(m_ssadd);
		pf.destinationIp	= inet_addr(m_sdadd);
		pf.sourceMask		= inet_addr(smask);
		pf.destinationMask 	= inet_addr(dmask);
		pf.sourcePort		= htons(atoi(m_ssport));
		pf.destinationPort	= htons(atoi(m_sdport));
		pf.drop				= m_action.GetCurSel() == 1;
		AddFilter(pf);
	
		CString str = "{\n    [protocol] " + protocal + " "
					"[action] " + action + " "
					"[saddr] " + m_ssadd + " " 
					"[daddr] " + m_sdadd + " "
					"[sport] " + m_ssport + " "
					"[dport] " + m_sdport + " "
					"[smask] " + smask + " "
					"[dmask] " + dmask + " \n}\n";
		SaveFile( (LPTSTR)(LPCTSTR)str);
		CloseFile();
		MessageBox("Saved!");
	}
	else { // Failed to Create file
		MessageBox("Failed to save!");
	}
	EndDialog(0);

}
BOOL CAddRuleDlg::NewFile(void)
{
/* This will open an existing file or open a new file if the file
   doesnot exists
*/
	_hFile = CreateFile("saved.rul",
						GENERIC_WRITE | GENERIC_READ,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_ALWAYS,
						NULL,
						NULL);

	/* If unable to obtain the handle values*/
	if(_hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	
	return TRUE;		// File has been opened succesfully
}
//******************************************************
// This will move the file pointer to the end of the file so that 
// it can be easily added to the file

DWORD CAddRuleDlg::GotoEnd(void)
{
	DWORD val;

	DWORD size = GetFileSize(_hFile,NULL);
	if(size == 0)
		return size;
	val = SetFilePointer(_hFile,0,NULL,FILE_END);

	/* If unable to set the file pointer to the end of the file */
 	if(val == 0)
	{
		MessageBox("Unable to set file pointer");
		return GetLastError();
	}
	return val;
}

/* This code will save the data into the file which is given by the parameter*/

DWORD CAddRuleDlg::SaveFile(char*  str)
{
	DWORD   bytesWritten;
	/* Try to write the string passed as parameter to the file and if any 
		error occurs return the appropriate values
	*/
	DWORD	_len =  strlen(str);
	if(WriteFile(_hFile, str, _len, &bytesWritten, NULL) == 0)
	{
		MessageBox("Unalbe to write to the file\n");
		return FALSE;
	}
	return TRUE;
}

/* This function will close the existing file */
BOOL CAddRuleDlg::CloseFile()
{
	if(!_hFile)
	{
	//	MessageBox("File handle does not exist");
		return FALSE;
	}

// if there is an appropriate handle then close it and return app values
    else{
		if(CloseHandle(_hFile) != 0)
		{
			return TRUE;
		}
		else 
			return FALSE;
	}
}

void CAddRuleDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CAddRuleDlg::OnKillfocusSport() 
{	
	UpdateData();
}

void CAddRuleDlg::OnKillfocusDport() 
{
	UpdateData();
}


USHORT CAddRuleDlg::ProtocolNum(int sel)
{
	switch(sel){
		case 0: return 1;	// ICMP
		case 1: return 6;	// TCP
		case 2: return 17;	// UDP
	}
}

void CAddRuleDlg::CheckValidity(CString& protocal, 
								CString& action, 
								CString& m_ssadd, 
								CString& m_sdadd, 
								CString& m_ssport, 
								CString& m_sdport, 
								CString& smask, 
								CString& dmask) {
	if(m_ssadd.IsEmpty()) {
		m_ssadd = "0.0.0.0";
		smask = "0.0.0.0";
	} else {
		smask = "255.255.255.255";
	}
	if(m_sdadd.IsEmpty()) {
		m_sdadd = "0.0.0.0";
		dmask = "0.0.0.0";
	} else {
		dmask = "255.255.255.255";
	}
	if(m_ssport.IsEmpty()) {
		m_ssport = "-1";
	}

	if(m_sdport.IsEmpty()) {
		m_sdport = "-1";
	}
}