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
#include <stdio.h>
#include "log.h"
#include "timer.h"
#include "constants.h"
#include "config.h"

CRITICAL_SECTION g_criticalSection;

void InitializeLog()
{
  if (IsLogEnabled()) {
    InitializeCriticalSection(&g_criticalSection);

    // Purge log file.
    fclose(fopen(LOG_FILE, "w"));
  }
}

void ShutdownLog()
{
  if (IsLogEnabled()) {
    DeleteCriticalSection(&g_criticalSection);
  }
}

void Log(LPCSTR lpcsFormat, LPCSTR lpcsLevel, va_list args)
{
  EnterCriticalSection(&g_criticalSection);

  FILE *file = fopen(LOG_FILE, "a");

  fprintf(file, "[%.0lf] [%d] %s: ", GetTimerCurrentTime(), GetCurrentThreadId(), lpcsLevel);
  vfprintf(file, lpcsFormat, args);
  fprintf(file, "\n");

  fclose(file);

  LeaveCriticalSection(&g_criticalSection);
}

void LogInfo(LPCSTR lpcsFormat, ...)
{
  if (GetLogLevel() >= LOG_LEVEL_INFO) {
    va_list args;
    va_start(args, lpcsFormat);
    Log(lpcsFormat, "INFO", args);
    va_end(args);
  }
}

void LogError(LPCSTR lpcsFormat, ...)
{
  if (GetLogLevel() >= LOG_LEVEL_ERROR) {
    va_list args;
    va_start(args, lpcsFormat);
    Log(lpcsFormat, "ERROR", args);
    va_end(args);
  }
}

void LogDebug(LPCSTR lpcsFormat, ...)
{
  if (GetLogLevel() >= LOG_LEVEL_DEBUG) {
    va_list args;
    va_start(args, lpcsFormat);
    Log(lpcsFormat, "DEBUG", args);
    va_end(args);
  }
}

void LogTrace(LPCSTR lpcsFormat, ...)
{
  if (GetLogLevel() >= LOG_LEVEL_TRACE) {
    va_list args;
    va_start(args, lpcsFormat);
    Log(lpcsFormat, "TRACE", args);
    va_end(args);
  }
}
