/**
 * This file is part of SMAC CPU Fix.
 * Copyright (C) 2018 Vincent Ho
 *
 * SMAC CPU Fix is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SMAC CPU Fix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SMAC CPU Fix.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <math.h>
#include "constants.h"
#include "patch.h"
#include "hook.h"
#include "config.h"
#include "log.h"
#include "timer.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

// Timer State
DOUBLE g_dBeginWorkTime = 0.0;
DOUBLE g_dBeginSleepTime = 0.0;

// Config
DOUBLE g_dMessageWaitTimeThreshold = 0.0;
DOUBLE g_dMessageProcessingTimeThreshold = 0.0;
DWORD g_dwMessageWaitTimeMin = 0;
DWORD g_dwMessageWaitTimeMax = 0;
DWORD g_dwMessageWaitTime = 0;
DWORD g_dwMessageWaitTimeInc = 0;

BOOL PatchCpuUsage(HANDLE);
BOOL WINAPI PeekMessageEx(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);

BOOL PatchApplication(HANDLE hProcess)
{
  g_dMessageWaitTimeThreshold = GetMessageWaitTimeThresholdMs() * 1000.0;
  g_dMessageProcessingTimeThreshold = GetMessageProcessingTimeThresholdMs() * 1000.0;
  g_dwMessageWaitTimeMin = GetMessageWaitTimeMinMs();;
  g_dwMessageWaitTimeMax = GetMessageWaitTimeMaxMs();
  g_dwMessageWaitTime = g_dwMessageWaitTimeMin;
  g_dwMessageWaitTimeInc = MAX(1, (g_dwMessageWaitTimeMax - g_dwMessageWaitTimeMin) / 10);

  return PatchCpuUsage(hProcess);
}

BOOL PatchCpuUsage(HANDLE hProcess)
{
  return HookFunction("PeekMessageA", (LPDWORD)&PeekMessageEx);
}

/**
 * Messaging overrides.
 */
BOOL WINAPI PeekMessageEx(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
  DOUBLE dBeginTime = GetTimerCurrentTime();
  DWORD dwMsgWaitResult = MsgWaitForMultipleObjectsEx(0, 0, g_dwMessageWaitTime, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
  DOUBLE dNow = GetTimerCurrentTime();

  if (dwMsgWaitResult == WAIT_TIMEOUT) {
    // Idle again, reset work timer.
    g_dBeginWorkTime = 0.0;

    if ((g_dBeginSleepTime <= 0.0) || (dNow < g_dBeginSleepTime)) {
      g_dBeginSleepTime = dNow;
    }

    if ((dNow - g_dBeginSleepTime) >= g_dMessageWaitTimeThreshold) {
      g_dwMessageWaitTime = MIN(g_dwMessageWaitTimeMax, g_dwMessageWaitTime + g_dwMessageWaitTimeInc);
      g_dBeginSleepTime = 0.0;
    }
  } else {
    // Messages available, reset sleep timer.
    g_dBeginSleepTime = 0.0;

    if ((g_dBeginWorkTime <= 0.0) || (dNow < g_dBeginWorkTime)) {
      g_dBeginWorkTime = dNow;
    }

    if ((dNow - g_dBeginWorkTime) >= g_dMessageProcessingTimeThreshold) {
      g_dwMessageWaitTime = 0;
    } else {
      g_dwMessageWaitTime = g_dwMessageWaitTimeMin;
    }
  }

  return PeekMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
}
