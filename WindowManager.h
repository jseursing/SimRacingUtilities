#pragma once
#include <map>
#include <string>
#include <Windows.h>
#include <vector>

class Win32Sem;

/**
  * Class: WindowManager
  * Brief: Manages active window monitoring, repositioning, and resizing.
  */
class WindowManager
{
public:
  
  struct AppEntry
  {
    HWND        hWnd;
    std::string WindowTitle;
    std::string Filename;
    uint32_t    ProcessId;
  };

  struct MonitorEntry
  {
    std::string MonitorName;
    int32_t     X;
    int32_t     Y;
    uint32_t    Width;
    uint32_t    Height;
  };

  struct ResizeEntry
  {
    int32_t     X;
    int32_t     Y;
    uint32_t    W;
    uint32_t    H;
    bool        Resize;
    bool        StripBorders;
  };

  /**
   * Singleton construction and access.
   */
  static WindowManager* Instance();

  /**
   * Refreshes active windows and accompanying process.
   */
  void RefreshApplications();

  /**
   * Retrieves a list of visible windows.
   */
  std::vector<AppEntry>& GetWindowList();

  /**
   * Retrieves a list of applications belonging to visible windows.
   */
  std::vector<std::string>& GetAppList();

  /**
   * Enumerates active display hardware (monitor)
   */
  void RefreshDisplayDevices();

  /**
   * Retrieves a list of active display hardware
   */
  std::vector<MonitorEntry>& GetMonitorList();

  /**
   * Adds a resizeable application/window to be managed.
   */
  void AddResizeApp(std::string filename, int32_t x, int32_t y, uint32_t w, uint32_t h, bool resize, bool stripBorders);

  /**
   * Removes the specified resizeable application/window from being managed.
   */
  bool RemoveResizeApp(std::string filename);

  /**
   * Applies the registered sizing and position to the specified application.
   * This is invoked by manual user intervention. 
   */
  void SynchronizeWindow(std::string filename);

  /**
    * Applies the registered sizing and position all registered applications.
    * This is invoked once every 3-seconds. 
    */
  void SynchronizeWindows(bool isAutoResize = true);
  void GetResizeIterators(std::map<std::string, ResizeEntry>::const_iterator& begin,
                          std::map<std::string, ResizeEntry>::const_iterator& end) const;

  /**
    * Export window manager configurations.
    */
  void Export(std::vector<char>& data);

  /**
    * Import window manager configurations.
    */
  void Import(std::vector<char> data);

private:
  
  /**
    * Windows callback for logging visible windowed applications and display hardware.
    */
  static BOOL CALLBACK EnumWndCallback(HWND hWnd, LPARAM lParam);
  static BOOL CALLBACK EnumMonCallback(HMONITOR hMonitor, HDC hDC, LPRECT dim, LPARAM lParam);

  /**
    * CTOR
    */
  WindowManager();
    
  std::vector<AppEntry>              Windows;
  std::vector<std::string>           Applications;
  std::vector<MonitorEntry>          Monitors;
  std::map<std::string, ResizeEntry> ResizeList;
  Win32Sem*                          Semaphore;
};

