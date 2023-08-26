#include "pch.h"
#include "ZhrServer.h"
#include "ZHRTool.h"

template<ZhrOperator op>
AcceptOperlapped<op>::AcceptOperlapped()
{
	m_worker = ThreadWorker(this, (FUNCTYPE)&AcceptOperlapped<op>::AcceptWorker);
	m_operator = EAccept;
	memset(&m_overlapped, 0, sizeof(m_overlapped));
	m_buffer.resize(1024);
	m_server = NULL;
}

template<ZhrOperator op>
int AcceptOperlapped<op>::AcceptWorker() {
	INT lLength = 0, rLength = 0;
	if (*(LPDWORD)*m_client > 0) {
		GetAcceptExSockaddrs(*m_client, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
			(sockaddr**)m_client->GetLocalAddr(), &lLength, //���ص�ַ
			(sockaddr**)m_client->GetRemoteAddr(), &rLength //Զ�̵�ַ
		);
		//RECVOVERLAPPED
		int ret = WSARecv((SOCKET)*m_client, m_client->RecvWSABuffer(), 1, *m_client, &m_client->flags(), *m_client, NULL);
		if (ret == SOCKET_ERROR && (WSAGetLastError() != WSA_IO_PENDING)) {
			//TODO:����
		}

		if (!m_server->NewAccept()) { //accpet������һ��accept
			return -2;
		}
	}
	return -1;
}

template <ZhrOperator op>
inline SendOperlapped<op>::SendOperlapped()
{
	m_operator = op;
	m_worker = ThreadWorker(this, (FUNCTYPE)&SendOperlapped<op>::SendWorker);
	memset(&m_overlapped, 0, sizeof(m_overlapped));
	m_buffer.resize(1024 * 256);
}
template <ZhrOperator op>
inline RecvOperlapped<op>::RecvOperlapped()
{
	m_operator = op;
	m_worker = ThreadWorker(this, (FUNCTYPE)&RecvOperlapped<op>::RecvWorker);
	memset(&m_overlapped, 0, sizeof(m_overlapped));
	m_buffer.resize(1024 * 256);
}


ZhrClient::ZhrClient()
	:m_isbusy(false), m_flags(0),
	m_overlapped(new ACCEPTOVERLAPPED()),
	m_recv(new RECVOVERLAPPED),
	m_send(new SENDOVERLAPPED),
	m_vecSend(this,(SENDCALLBACK)&ZhrClient::SendData)
{
	m_sock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	m_buffer.resize(1024);
	memset(&m_laddr, 0, sizeof(m_laddr));
	memset(&m_raddr, 0, sizeof(m_raddr));
}

void ZhrClient::SetOverlapped(PCLIENT& ptr) {
	m_overlapped->m_client = ptr.get();
	m_recv->m_client = ptr.get();
	m_send->m_client = ptr.get();
}

ZhrClient::operator LPOVERLAPPED() {
	return &m_overlapped->m_overlapped;
}

LPWSABUF ZhrClient::RecvWSABuffer()
{
	return &m_recv->m_wsabuffer;
}

LPWSABUF ZhrClient::SendWSABuffer()
{
	return &m_send->m_wsabuffer;
}

int ZhrClient::Recv()
{
	int ret = recv(m_sock, m_buffer.data() + m_used, m_buffer.size() - m_used, 0);
	if (ret <= 0) return -1;
	m_used += (size_t)ret;
	//todo :��������
	return 0;
}

int ZhrClient::Send(void* buffer, size_t nSize)
{
	std::vector<char> data(nSize);
	memcpy(data.data(), buffer, nSize);
	if (m_vecSend.PushBack(data)) {
		return 0;
	}
	return -1;
}

int ZhrClient::SendData(std::vector<char>& data)
{
	if (m_vecSend.Size() > 0) {
		int ret = WSASend(m_sock, SendWSABuffer(), 1, &m_received, m_flags, &m_send->m_overlapped, NULL);
		if (ret != 0 && (WSAGetLastError() != WSA_IO_PENDING)) {
			CZHRTool::ShowError();
			return -1;
		}
	}
	return 0;
}

//#############################################################

ZhrServer::~ZhrServer()
{
	closesocket(m_sock);
	std::map<SOCKET, PCLIENT>::iterator it = m_client.begin();
	for (; it != m_client.end(); it++) {
		it->second.reset();
	}
	m_client.clear();
	CloseHandle(m_hIOCP);
	m_pool.Stop();
}

bool ZhrServer::StartServer()
{
	CreateSocket();
	if (bind(m_sock, (sockaddr*)&m_addr, sizeof(m_addr)) == -1) {
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		return false;
	}
	if (listen(m_sock, 3) == -1) {
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		return false;
	}
	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 4);
	if (m_hIOCP == NULL) {
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		m_hIOCP = INVALID_HANDLE_VALUE;
		return false;
	}
	CreateIoCompletionPort((HANDLE)m_sock, m_hIOCP, (ULONG_PTR)this, 0);  //msock�����Ѿ����ڵ�iocp
	m_pool.Invoke();
	m_pool.DispatchWorker(ThreadWorker(this, (FUNCTYPE)&ZhrServer::threadIocp));   //worker--threadIocp����
	if (!NewAccept()) return false;
	//m_pool.DispatchWorker(ThreadWorker(this, (FUNCTYPE)&ZhrServer::threadIocp));
	//m_pool.DispatchWorker(ThreadWorker(this, (FUNCTYPE)&ZhrServer::threadIocp));
	return true;
}

int ZhrServer::threadIocp()
{

	DWORD transfered = 0;
	ULONG_PTR CompletionKey = 0;
	OVERLAPPED* lpOverlapped = NULL;
	if (GetQueuedCompletionStatus(m_hIOCP, &transfered, &CompletionKey, &lpOverlapped, INFINITE)) {
		if (transfered > 0 && (CompletionKey != 0)) {
			ZhrOverlapped* pOverlapped = CONTAINING_RECORD(lpOverlapped, ZhrOverlapped, m_overlapped);
			switch (pOverlapped->m_operator) {
			case EAccept:
			{
				ACCEPTOVERLAPPED* pOver = (ACCEPTOVERLAPPED*)pOverlapped;
				m_pool.DispatchWorker(pOver->m_worker);
			}
			break;
			case ERecv:
			{
				RECVOVERLAPPED* pOver = (RECVOVERLAPPED*)pOverlapped;
				m_pool.DispatchWorker(pOver->m_worker);
			}
			break;
			case ESend:
			{
				SENDOVERLAPPED* pOver = (SENDOVERLAPPED*)pOverlapped;
				m_pool.DispatchWorker(pOver->m_worker);
			}
			break;
			case EError:
			{
				ERROROVERLAPPED* pOver = (ERROROVERLAPPED*)pOverlapped;
				m_pool.DispatchWorker(pOver->m_worker);
			}
			break;
			}
		}
		else {
			return -1;
		}
	}
	return 0;
}