﻿// CWatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "CWatchDialog.h"
#include "afxdialogex.h"
#include "RemoteClientDlg.h"


// CWatchDialog 对话框

IMPLEMENT_DYNAMIC(CWatchDialog, CDialog)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_WATCH, pParent)
{

}

CWatchDialog::~CWatchDialog()
{
}

void CWatchDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WATCH, m_picture);
}


BEGIN_MESSAGE_MAP(CWatchDialog, CDialog)
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_STN_CLICKED(IDC_WATCH, &CWatchDialog::OnStnClickedWatch)
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序


CPoint CWatchDialog::UserPoint2RemoteScreenPoint(CPoint& point)
{//800 450
	CRect clientRect;
	ScreenToClient(&point); //全局坐标到客户区域坐标
	//本地坐标到远程坐标
	m_picture.GetWindowRect(clientRect);
	return CPoint(point.x * 1920 / clientRect.Width(),
				  point.y * 1080 / clientRect.Height());
}

BOOL CWatchDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	// TODO:  在此添加额外的初始化
	SetTimer(0, 50, NULL); //50ms一次
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 0)
	{
		CRemoteClientDlg* pParant = (CRemoteClientDlg*)GetParent();
		if (pParant->isFull()) {
			CRect rect;
			m_picture.GetWindowRect(rect);
			//pParant->GetImage().BitBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, SRCCOPY);
			pParant->GetImage().StretchBlt(m_picture.GetDC()->GetSafeHdc(),   //缩放
				0, 0, rect.Width(),rect.Height(),SRCCOPY);
			m_picture.InvalidateRect(NULL); //重绘
			pParant->GetImage().Destroy();
			pParant->SetImageStatus(); //timer置为false了
		}
	}
	CDialog::OnTimer(nIDEvent);
}

//################################################################


void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	//坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 2;//双击
	CClientSocket* pClient=CClientSocket::getInstance();
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	pClient->Send(pack);

	CDialog::OnLButtonDblClk(nFlags, point);
}


void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 3;//双击
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	pClient->Send(pack);

	CDialog::OnLButtonDown(nFlags, point);
}


void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	//CPoint remote = UserPoint2RemoteScreenPoint(point);
	////封装
	//MOUSEEV event;
	//event.ptXY = remote;
	//event.nButton = 0;//左键
	//event.nAction = 4;//弹起
	//CClientSocket* pClient = CClientSocket::getInstance();
	//CPacket pack(5, (BYTE*)&event, sizeof(event));
	//pClient->Send(pack);

	CDialog::OnLButtonUp(nFlags, point);
}

//##########################################################

void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 2;//右键
	event.nAction = 2;//双击
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	pClient->Send(pack);

	CDialog::OnRButtonDblClk(nFlags, point);
}


void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 3;//按下 TODO：服务端要做对应修改
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	pClient->Send(pack);

	CDialog::OnRButtonDown(nFlags, point);
}


void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 4;//弹起
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	pClient->Send(pack);

	CDialog::OnRButtonUp(nFlags, point);
}


void CWatchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 1;//双击
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	pClient->Send(pack);

	CDialog::OnMouseMove(nFlags, point);
}


void CWatchDialog::OnStnClickedWatch()  //按下
{
	// TODO: 在此添加控件通知处理程序代码
	CPoint point;
	GetCursorPos(&point);
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 3;//双击
	CClientSocket* pClient = CClientSocket::getInstance();
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	pClient->Send(pack);
}
