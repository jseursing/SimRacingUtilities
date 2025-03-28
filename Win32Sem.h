#pragma once

/**
  * Class: Win32Sem
  * Brief: This is a wrapper class for window's semaphores.
  */
class Win32Sem
{
public:
  bool Give();
  bool Take(size_t waitMs);
  Win32Sem(size_t initialCount, size_t maxCount);
  ~Win32Sem();

private:
  void* SemObject;
};

