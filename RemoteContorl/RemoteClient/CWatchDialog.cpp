﻿// CWatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "CWatchDialog.h"
#include "afxdialogex.h"
#include "ClientController.h"


// CWatchDialog 对话框

IMPLEMENT_DYNAMIC(CWatchDialog, CDialog)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_WATCH, pParent)
{
	m_isFull = false;
	m_nObjWidth = -1;
	m_nObjHeight = -1;
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
	ON_BN_CLICKED(IDC_BTN_LOCK, &CWatchDialog::OnBnClickedBtnLock)
	ON_BN_CLICKED(IDC_BTN_UNLOCK, &CWatchDialog::OnBnClickedBtnUnlock)
	ON_MESSAGE(WM_SEND_PACK_ACK, CWatchDialog::OnSendPacketAck)
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序


CPoint CWatchDialog::UserPoint2RemoteScreenPoint(CPoint& point, bool isScreen)
{//800 450
	CRect clientRect;
	if (!isScreen)ClientToScreen(&point);  //转换为屏幕坐标
	m_picture.ScreenToClient(&point); //全局坐标到客户区域坐标
	//本地坐标到远程坐标
	m_picture.GetWindowRect(clientRect);
	return CPoint(point.x * m_nObjWidth / clientRect.Width(),
		point.y * m_nObjHeight / clientRect.Height());
}

BOOL CWatchDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_isFull = false;
	// TODO:  在此添加额外的初始化
	//SetTimer(0, 50, NULL); //50ms一次
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{
	//if (nIDEvent == 0)
	//{
	//	CClientController* pParent = (CClientController*)GetParent();
	//	if (m_isFull) {
	//		CRect rect;
	//		m_picture.GetWindowRect(rect);
	//		m_nObjWidth = m_image.GetWidth();
	//		m_nObjHeight = m_image.GetHeight();
	//		
	//		//pParant->GetImage().BitBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, SRCCOPY);
	//		m_image.StretchBlt(m_picture.GetDC()->GetSafeHdc(),   //缩放
	//			0, 0, rect.Width(),rect.Height(),SRCCOPY);
	//		m_picture.InvalidateRect(NULL); //重绘
	//		m_image.Destroy();
	//		m_isFull = false;
	//		TRACE("更新图片完成 %d %d\r\n", m_nObjHeight, m_nObjWidth);
	//	}
	 //}
	CDialog::OnTimer(nIDEvent);
}

//################################################################


LRESULT CWatchDialog::OnSendPacketAck(WPARAM wParam, LPARAM lParam)
{
	if ((lParam == -1) || (lParam == -2)) {
		//todo:错误处理
	}
	else if (lParam == 1) {
		//对方关闭套接字
	}
	else {
		CPacket* pPacket = (CPacket*)wParam;
		if (pPacket != NULL) {
			CPacket head = *(CPacket*)wParam;
			delete (CPacket*)wParam;
			switch (head.sCmd) {
			case 6:
			{
				CZHRTool::Bytes2Image(m_image, head.strData);
				CRect rect;
				m_picture.GetWindowRect(rect);
				m_nObjWidth = m_image.GetWidth();
				m_nObjHeight = m_image.GetHeight();

				//pParant->GetImage().BitBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, SRCCOPY);
				m_image.StretchBlt(m_picture.GetDC()->GetSafeHdc(),   //缩放
					0, 0, rect.Width(), rect.Height(), SRCCOPY);
				m_picture.InvalidateRect(NULL); //重绘
				m_image.Destroy();
				m_isFull = false;
				TRACE("更新图片完成 %d %d\r\n", m_nObjHeight, m_nObjWidth);
				break;
			}
			case 5:
				TRACE("远程端应答了鼠标操作\r\n");
				break;
			case 7:
			case 8:
			default:
				break;
			}
		}
	}
	return 0;
}

void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 2;//双击
		CClientController::getInstance()->
			SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));

	}
	CDialog::OnLButtonDblClk(nFlags, point);
}


void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1)) {
		TRACE("x=%d, y=%d\r\n", point.x, point.y);
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		TRACE("x=%d, y=%d\r\n", point.x, point.y);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 2;//按下
		CClientController::getInstance()->
			SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnLButtonDown(nFlags, point);
}


void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1)) { //
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 3;//弹起
		CClientController::getInstance()->
			SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnLButtonUp(nFlags, point);
}

//##########################################################

void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1)) {
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;//右键
		event.nAction = 1;//双击
		CClientController::getInstance()->
			SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnRButtonDblClk(nFlags, point);
}


void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1)) {
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;//右键
		event.nAction = 2;//按下 
		CClientController::getInstance()->
			SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnRButtonDown(nFlags, point);
}


void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1)) {
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;//右键
		event.nAction = 3;//弹起
		CClientController::getInstance()->
			SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnRButtonUp(nFlags, point);
}


void CWatchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1)) {
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 8;//没有
		event.nAction = 0;//双击
		CClientController::getInstance()->
			SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnMouseMove(nFlags, point);
}


void CWatchDialog::OnStnClickedWatch()  //按下
{
	if ((m_nObjHeight != -1) && (m_nObjWidth != -1)) {
		// TODO: 在此添加控件通知处理程序代码
		CPoint point;
		GetCursorPos(&point);
		CPoint remote = UserPoint2RemoteScreenPoint(point, true);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 0;//单击
		CClientController::getInstance()->
			SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
}


void CWatchDialog::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialog::OnOK();
}


void CWatchDialog::OnBnClickedBtnLock()
{
	// TODO: 在此添加控件通知处理程序代码
	CClientController::getInstance()->
		SendCommandPacket(GetSafeHwnd(), 7);
}


void CWatchDialog::OnBnClickedBtnUnlock()
{
	// TODO: 在此添加控件通知处理程序代码
	CClientController::getInstance()->
		SendCommandPacket(GetSafeHwnd(), 8);
}
