#include "Win32Sem.h"
#include <Windows.h>
#include <cassert>

/**
 * Function: Give
 * Notes: See header file
 */
bool Win32Sem::Give()
{
  return TRUE == ReleaseSemaphore(SemObject, 1, nullptr);
}

/**
 * Function: Take
 * Notes: See header file
 */
bool Win32Sem::Take(size_t waitMs)
{
  return TRUE == WaitForSingleObject(SemObject, waitMs);
}

/**
 * Function: Win32Sem
 * Notes: See header file
 */
Win32Sem::Win32Sem(size_t initialCount, size_t maxCount)
{
  SemObject = CreateSemaphore(nullptr, initialCount, maxCount, nullptr);
  assert(nullptr != SemObject);
}

/**
 * Function: ~Win32Sem
 * Notes: See header file
 */
Win32Sem::~Win32Sem()
{
  CloseHandle(SemObject);
}