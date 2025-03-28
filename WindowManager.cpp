#include "GlobalConst.h"
#include "Interpreter.h"
#include "MemUtils.h"
#include "Win32Sem.h"
#include "WindowManager.h"
#include <TlHelp32.h>

/**
 * Function: Instance
 * Notes: See header file
 */
WindowManager* WindowManager::Instance()
{
  static WindowManager instance;
  return &instance;
}

/**
 * Function: RefreshApplications
 * Notes: See header file
 */
void WindowManager::RefreshApplications()
{
  Semaphore->Take(-1);

  // Clear list
  Windows.clear();
  Applications.clear();

  // Enumerate Windows
  EnumWindows(EnumWndCallback, 0);

  // Enumerate active processes
  std::map<uint32_t, std::string> procMap;
  MemUtils::EnumerateProcesses(procMap);

  // Build Apps list
  // Remove applications that do not have a window (UNLESS USER IS HOLDING SHIFT!)
  for (std::map<uint32_t, std::string>::const_iterator itr = procMap.begin();
       itr != procMap.end();
       ++itr)
  {
    if (true == Interpreter::DeveloperMode)
    {
      Applications.push_back(itr->second);
    }
    else
    {
      for (int32_t i = 0; i < Windows.size(); ++i)
      {
        if (Windows[i].ProcessId == itr->first)
        {
          Applications.push_back(itr->second);
        }
      }
    }
  }

  // Remove applications missing filenames
  for (int32_t i = 0; i < Windows.size(); ++i)
  {
    std::map<uint32_t, std::string>::const_iterator itr = procMap.find(Windows[i].ProcessId);
    if (procMap.end() != itr)
    {
      Windows[i].Filename = itr->second;
    }
    else
    {
      Windows.erase(Windows.begin() + i);
      --i;
    }
  }

  Semaphore->Give();
}

/**
 * Function: GetWindowList
 * Notes: See header file
 */
std::vector<WindowManager::AppEntry>& WindowManager::GetWindowList()
{
  return Windows;
}

/**
 * Function: GetAppList
 * Notes: See header file
 */
std::vector<std::string>& WindowManager::GetAppList()
{
  return Applications;
}

/**
 * Function: RefreshDisplayDevices
 * Notes: See header file
 */
void WindowManager::RefreshDisplayDevices()
{
  Monitors.clear();
  EnumDisplayMonitors(0, nullptr, EnumMonCallback, 0);

  // Sort displays by X position
  for (size_t i = 0; i < Monitors.size(); ++i)
  {
    for (size_t j = i + 1; j < Monitors.size(); ++j)
    {
      if (Monitors[i].X > Monitors[j].X)
      {
        std::swap(Monitors[i], Monitors[j]);
      }
    }
  }
}

/**
 * Function: GetMonitorList
 * Notes: See header file
 */
std::vector<WindowManager::MonitorEntry>& WindowManager::GetMonitorList()
{
  return Monitors;
}

/**
 * Function: AddResizeApp
 * Notes: See header file
 */
void WindowManager::AddResizeApp(std::string filename, 
                                 int32_t x, 
                                 int32_t y, 
                                 uint32_t w, 
                                 uint32_t h, 
                                 bool resize, 
                                 bool stripBorders)
{
  Semaphore->Take(-1);

  std::map<std::string, ResizeEntry>::iterator itr = ResizeList.find(filename.c_str());
  if (ResizeList.end() == itr)
  {
    ResizeEntry entry;
    entry.X = x;
    entry.Y = y;
    entry.W = w;
    entry.H = h;
    entry.Resize = resize;
    entry.StripBorders = stripBorders;
    ResizeList[filename.c_str()] = entry;
  }
  else
  {
    itr->second.X = x;
    itr->second.Y = y;
    itr->second.W = w;
    itr->second.H = h;
    itr->second.Resize = resize;
    itr->second.StripBorders = stripBorders;
  }

  Semaphore->Give();
}

/**
 * Function: RemoveResizeApp
 * Notes: See header file
 */
bool WindowManager::RemoveResizeApp(std::string filename)
{
  std::map<std::string, ResizeEntry>::iterator itr = ResizeList.find(filename.c_str());
  if (ResizeList.end() == itr)
  {
    return false;
  }

  Semaphore->Take(-1);
  ResizeList.erase(itr);
  Semaphore->Give();

  return true;
}

/**
 * Function: SynchronizeWindow
 * Notes: See header file
 */
void WindowManager::SynchronizeWindow(std::string filename)
{
  RefreshApplications();

  Semaphore->Take(-1);
  for (size_t i = 0; i < Applications.size(); ++i)
  {
    std::map<std::string, ResizeEntry>::const_iterator itr = ResizeList.find(Windows[i].Filename.c_str());
    if (itr == ResizeList.end())
    {
      continue;
    }

    RECT rect;
    if (TRUE == GetWindowRect(Windows[i].hWnd, &rect))
    {
      if ((rect.left != itr->second.X) ||
          ((rect.right - rect.left) != itr->second.W) ||
          (rect.top != itr->second.Y) ||
          ((rect.bottom - rect.top) != itr->second.H))
      {
        SetWindowPos(Windows[i].hWnd,
                     HWND_TOP,
                     itr->second.X,
                     itr->second.Y,
                     itr->second.W,
                     itr->second.H,
                     SWP_ASYNCWINDOWPOS);
      }
    }

    if (true == itr->second.StripBorders)
    {
      SetWindowLong(Windows[i].hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
    }

    break;
  }

  Semaphore->Give();
}

/**
 * Function: SynchronizeWindows
 * Notes: See header file
 */
void WindowManager::SynchronizeWindows(bool isAutoResize)
{
  if (0 != ResizeList.size())
  {
    RefreshApplications();

    Semaphore->Take(-1);
    for (size_t i = 0; i < Applications.size(); ++i)
    {
      if (i >= Windows.size()) break; // Error condition
      std::map<std::string, ResizeEntry>::const_iterator itr = ResizeList.find(Windows[i].Filename.c_str());
      if (itr == ResizeList.end())
      {
        continue;
      }

      if ((true == isAutoResize) &&
          (false == itr->second.Resize))
      {
        continue;
      }

      RECT rect;
      if (TRUE == GetWindowRect(Windows[i].hWnd, &rect))
      {
        if ((rect.left != itr->second.X) ||
            ((rect.right - rect.left) != itr->second.W) ||
            (rect.top != itr->second.Y) ||
            ((rect.bottom - rect.top) != itr->second.H))
        {
          SetWindowPos(Windows[i].hWnd, 
                       HWND_TOP, 
                       itr->second.X, 
                       itr->second.Y, 
                       itr->second.W, 
                       itr->second.H, 
                       SWP_ASYNCWINDOWPOS);
        }
      }

      if (true == itr->second.StripBorders)
      {
        SetWindowLong(Windows[i].hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
      }
    }

    Semaphore->Give();
  }
}

/**
 * Function: GetResizeIterators
 * Notes: See header file
 */
void WindowManager::GetResizeIterators(
  std::map<std::string, ResizeEntry>::const_iterator& begin,
  std::map<std::string, ResizeEntry>::const_iterator& end) const
{
  begin = ResizeList.begin();
  end   = ResizeList.end();
}

/**
 * Function: Export
 * Notes: See header file
 */
void WindowManager::Export(std::vector<char>& data)
{
  size_t dataSize = 1; // Entry Count
  size_t dataLenPos = data.size() + 1;

  data.push_back(GlobalConst::WINDOW_MGR); // MGR ID
  data.push_back(0); // Two reserve bytes for total size
  data.push_back(0);
  data.push_back(ResizeList.size());       // ENTRY Count
  for (std::map<std::string, ResizeEntry>::const_iterator itr = ResizeList.begin();
       itr != ResizeList.end();
       ++itr)
  {
    // Write length of filename, filename, length of datastruct, datastruct
    dataSize += 1 + itr->first.length() + 1 + sizeof(ResizeEntry);
    data.push_back(static_cast<char>(itr->first.length()));
    for (size_t i = 0; i < itr->first.length(); ++i)
    {
      data.push_back(itr->first[i]);
    }
    
    data.push_back(sizeof(ResizeEntry));

    char dataBuf[sizeof(ResizeEntry) + 1] = {0};
    ResizeEntry entry = itr->second;
    memcpy(dataBuf, &entry, sizeof(ResizeEntry));
    for (size_t i = 0; i < sizeof(ResizeEntry); ++i)
    {
      data.push_back(dataBuf[i]);
    }
  }

  *reinterpret_cast<short*>(&data[dataLenPos]) = dataSize;
}

/**
 * Function: Import
 * Notes: See header file
 */
void WindowManager::Import(std::vector<char> data)
{
  size_t pos = 0;
  
  size_t numEntries = data[pos++];
  for (size_t i = 0; i < numEntries; ++i)
  {
    std::string filename = "";

    if (pos > data.size()) return;
    size_t nameLen = data[pos++];

    if ((pos + nameLen) >= data.size()) return;
    filename.resize(nameLen + 1);
    memcpy(filename.data(),  & data[pos], nameLen);
    pos += nameLen + 1; // we know size of resizeentry...

    if ((pos + sizeof(ResizeEntry)) > data.size()) return;
    ResizeEntry entry;
    memcpy(&entry, &data[pos], sizeof(ResizeEntry));
    ResizeList[filename.c_str()] = entry;
    pos += sizeof(ResizeEntry);
  }
}

/**
 * Function: EnumWndCallback
 * Notes: See header file
 */
BOOL CALLBACK WindowManager::EnumWndCallback(HWND hWnd, LPARAM lParam)
{
  int32_t titleLen = GetWindowTextLength(hWnd);
  if ((0 < titleLen) &&
      (true == IsWindowVisible(hWnd)))
  {
    std::vector<char> buf(titleLen + 2, 0);
    GetWindowTextA(hWnd, buf.data(), titleLen + 1);
    
    WindowManager::AppEntry entry;
    entry.hWnd = hWnd;
    entry.WindowTitle = buf.data();

    unsigned long dwPid = 0;
    GetWindowThreadProcessId(hWnd, &dwPid);
    entry.ProcessId = dwPid;
    entry.Filename = "";
    Instance()->Windows.emplace_back(entry);
  }

  return TRUE;
}

/**
 * Function: EnumMonCallback
 * Notes: See header file
 */
BOOL CALLBACK WindowManager::EnumMonCallback(HMONITOR hMonitor, HDC hDC, LPRECT dim, LPARAM lParam)
{
  MONITORINFOEXA info;
  info.cbSize = sizeof(MONITORINFOEXA);
  if (0 != GetMonitorInfoA(hMonitor, &info))
  {
    MonitorEntry entry;
    entry.MonitorName = info.szDevice;
    size_t lastPos = entry.MonitorName.find_last_of("\\");
    if (std::string::npos != lastPos)
    {
      entry.MonitorName = entry.MonitorName.substr(lastPos + 1);
    }

    entry.X = info.rcMonitor.left;
    entry.Y = info.rcMonitor.top;
    entry.Width  = info.rcMonitor.right - info.rcMonitor.left;
    entry.Height = info.rcMonitor.bottom - info.rcMonitor.top;
    Instance()->Monitors.emplace_back(entry);
  }

  return TRUE;
}

/**
 * Function: WindowManager
 * Notes: See header file
 */
WindowManager::WindowManager() :
  Semaphore(new Win32Sem(1, 1))
{

}