#pragma once
#include <stdint.h>
#include <string>

struct sockaddr_in;

/**
  * Generic UDP socket class
  */
class IpcUdpSocket
{
public:

  struct SenderAddr
  {
    std::string addr;
    uint16_t    port;
  };

  static IpcUdpSocket* Create(const char* host, uint16_t port, bool receive);
  static void Destroy(IpcUdpSocket*& ipcObj);

  bool Send(char* data, size_t length, size_t timeoutMs = 0);
  bool Send(char* data, size_t length, sockaddr_in* receiver, size_t timeoutMs);
  bool Receive(char* data, size_t maxLen, size_t& recvLen, size_t timeoutMs, SenderAddr* addr);
  bool Receive(char* data, size_t maxLen, size_t& recvLen, size_t timeoutMs, sockaddr_in* sender);

private:

  bool BindSocket();
  IpcUdpSocket(const char* host, uint16_t port);
  ~IpcUdpSocket();

  std::string  HostAddress;
  uint16_t     HostPort;
  uintptr_t    Fd;
};