#include "IpcUdpSocket.h"
#include <WS2tcpip.h>
#include <winsock.h>

/**
* Function: Create
* Notes: See header file
*/
IpcUdpSocket* IpcUdpSocket::Create(const char* host, uint16_t port, bool receive)
{
  uintptr_t fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (INVALID_SOCKET == fd)
  {
    return nullptr;
  }

  IpcUdpSocket* ipcObj = new IpcUdpSocket(host, port);
  ipcObj->Fd = fd;

  if ((true == receive) &&
      (false == ipcObj->BindSocket()))
  {
    delete ipcObj;
    ipcObj = nullptr;
  }

  return ipcObj;
}

/**
* Function: Destroy
* Notes: See header file
*/
void IpcUdpSocket::Destroy(IpcUdpSocket*& ipcObj)
{
  if (nullptr != ipcObj)
  {
    delete ipcObj;
    ipcObj = nullptr;
  }
}

/**
* Function: BindSocket
* Notes: See header file
*/
bool IpcUdpSocket::BindSocket()
{
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(HostPort);
  inet_pton(AF_INET, HostAddress.c_str(), &(addr.sin_addr));

  return (SOCKET_ERROR != bind(Fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)));
}

/**
* Function: Send
* Notes: See header file
*/
bool IpcUdpSocket::Send(char* data, size_t length, size_t timeoutMs)
{
  sockaddr_in addr;
  int structSz = sizeof(addr);

  memset(&addr, 0, structSz);
  addr.sin_family = AF_INET;
  addr.sin_port   = htons(HostPort);
  inet_pton(AF_INET, HostAddress.c_str(), &(addr.sin_addr));

  return (SOCKET_ERROR != sendto(Fd, 
                                 data, 
                                 static_cast<int>(length), 
                                 0, 
                                 reinterpret_cast<sockaddr*>(&addr), structSz));
}

/**
* Function: Send
* Notes: See header file
*/
bool IpcUdpSocket::Send(char* data, size_t length, sockaddr_in* receiver, size_t timeoutMs)
{
  sockaddr_in addr;
  memcpy(&addr, receiver, sizeof(sockaddr_in));

  int structSz = sizeof(addr);
  return (SOCKET_ERROR != sendto(Fd, 
                                 data, 
                                 static_cast<int>(length), 
                                 0, 
                                 reinterpret_cast<sockaddr*>(&addr), structSz));
}

/**
* Function: Receive
* Notes: See header file
*/
bool IpcUdpSocket::Receive(char* data, size_t maxLen, size_t& recvLen, size_t timeoutMs, SenderAddr* addr)
{
  sockaddr_in source;
  bool success = Receive(data, maxLen, recvLen, timeoutMs, &source);

  if ((true == success) &&
      (nullptr != addr))
  {
    addr->addr.resize(16, 0);
    inet_ntop(AF_INET, &(source.sin_addr), addr->addr.data(), addr->addr.size());
    addr->port = htons(source.sin_port);
  }
 
  return success;
}

/**
* Function: Receive
* Notes: See header file
*/
bool IpcUdpSocket::Receive(char* data, size_t maxLen, size_t& recvLen, size_t timeoutMs, sockaddr_in* addr)
{
  bool success = false;
  int structSz = sizeof(sockaddr_in);

  // Configure timeout
  fd_set fd;
  FD_ZERO(&fd);
  FD_SET(Fd, &fd);
  
  timeval recvTimeout;
  recvTimeout.tv_sec = static_cast<long>(timeoutMs / 1000);
  recvTimeout.tv_usec = (timeoutMs % 1000) * 1000;
  
  recvLen = 0;
  switch (select(static_cast<int>(Fd), &fd, 0, 0, &recvTimeout))
  {
  case 0: // Timeout
    break;
  case -1: // Socket error
    break;
  default: // Success
    recvLen = recvfrom(Fd, 
                       data, 
                       static_cast<int>(maxLen), 
                       0, 
                       reinterpret_cast<sockaddr*>(addr), 
                       &structSz);
    if (SOCKET_ERROR != recvLen)
    {
      success = true;
    }
    break;
  }

  return success;
}


/**
* Function: IpcUdpSocket
* Notes: See header file
*/
IpcUdpSocket::IpcUdpSocket(const char* host, uint16_t port) :
  HostAddress(host),
  HostPort(port),
  Fd(INVALID_SOCKET)
{
}
  
/**
* Function: ~IpcUdpSocket
* Notes: See header file
*/
IpcUdpSocket::~IpcUdpSocket()
{
  if (INVALID_SOCKET != Fd)
  {
    closesocket(Fd);
  }
}