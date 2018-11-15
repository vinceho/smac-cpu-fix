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
#include "config.h"
#include "constants.h"
#include "log.h"

Config g_config = {
  // Options
  0,
  // FixCpuUsage
  1, 10, 250, 50
};

ConfigLink configLinks[] = {
  { "Options", "Log", CVT_NUMBER, &g_config.dwLogLevel },

  { "FixCpuUsage", "MessageWaitTimeMinMs", CVT_NUMBER, &g_config.dwMessageWaitTimeMinMs },
  { "FixCpuUsage", "MessageWaitTimeMaxMs", CVT_NUMBER, &g_config.dwMessageWaitTimeMaxMs },
  { "FixCpuUsage", "MessageWaitTimeThresholdMs", CVT_NUMBER, &g_config.dwMessageWaitTimeThresholdMs },
  { "FixCpuUsage", "MessageProcessingTimeThresholdMs", CVT_NUMBER, &g_config.dwMessageProcessingTimeThresholdMs }
};

BOOL HasConfig();
BOOL ReadConfig();
BOOL WriteConfig();

BOOL InitializeConfig()
{
  if (HasConfig()) {
    return ReadConfig();
  }

  return WriteConfig();
}

DWORD GetMinMax(DWORD dwValue, DWORD dwMin, DWORD dwMax)
{
  if (dwValue < dwMin) {
    return dwMin;
  } else if (dwValue > dwMax) {
    return dwMax;
  }

  return dwValue;
}

FLOAT GetMinMaxFloat(FLOAT fValue, FLOAT fMin, FLOAT fMax)
{
  if (fValue < fMin) {
    return fMin;
  } else if (fValue > fMax) {
    return fMax;
  }

  return fValue;
}

BOOL ValidateConfig()
{
  g_config.dwLogLevel = GetMinMax(0, g_config.dwLogLevel, 4);

  g_config.dwMessageWaitTimeMinMs = GetMinMax(0, g_config.dwMessageWaitTimeMinMs, 5000);
  g_config.dwMessageWaitTimeMaxMs = GetMinMax(g_config.dwMessageWaitTimeMinMs, g_config.dwMessageWaitTimeMaxMs, 5000);
  g_config.dwMessageWaitTimeThresholdMs = GetMinMax(0, g_config.dwMessageWaitTimeThresholdMs, 3600000);
  g_config.dwMessageProcessingTimeThresholdMs = GetMinMax(0, g_config.dwMessageProcessingTimeThresholdMs, 3600000);

  return TRUE;
}

BOOL HasConfig()
{
  FILE *file = file = fopen(CONFIG_FILE, "r");

  if (file) {
    fclose(file);

    return TRUE;
  }

  return FALSE;
}

BOOL ReadConfig()
{
  DWORD dwSize = (sizeof(configLinks) / sizeof(ConfigLink));
  FILE *file = fopen(CONFIG_FILE, "r");
  DWORD dwBufferSize = 64;
  char buffer[dwBufferSize], name[dwBufferSize], value[dwBufferSize];

  // Ignore INI sections when reading for simplicity.
  if (file) {
    while (TRUE) {
      if (fgets(buffer, dwBufferSize, file) == NULL) {
        break;
      }

      if (sscanf(buffer, " %[^= ] = %s ", name, value) == 2) {
        for (DWORD i = 0; i < dwSize; i++) {
          LPCSTR lpcsName = configLinks[i].lpcsName;

          if (!strcmp(lpcsName, name)) {
            switch (configLinks[i].type) {
              case CVT_BOOL:
                *(BOOL *)(configLinks[i].lpvValue) = (BOOL)atoi(value);
                break;
              case CVT_NUMBER:
                *(DWORD *)(configLinks[i].lpvValue) = (DWORD)atoi(value);
                break;
              case CVT_FLOAT:
                *(FLOAT *)(configLinks[i].lpvValue) = (FLOAT)atof(value);
                break;
            }

            break;
          }
        }
      }

      if(feof(file)) {
        break;
      }
    }

    fclose(file);
  }

  return ValidateConfig();
}

BOOL WriteConfig()
{
  DWORD dwSize = (sizeof(configLinks) / sizeof(ConfigLink));
  FILE *file = fopen(CONFIG_FILE, "w");
  LPCSTR lpcsLastSection = NULL;

  for (DWORD i = 0; i < dwSize; i++) {
    LPCSTR lpcsCurrentSection = configLinks[i].lpcsSection;

    if (lpcsLastSection == NULL || strcmp(lpcsLastSection, lpcsCurrentSection)) {
      if (lpcsLastSection != NULL) {
        fprintf(file, "\n");
      }

      fprintf(file, "[%s]\n", lpcsCurrentSection);
    }

    lpcsLastSection = lpcsCurrentSection;

    switch (configLinks[i].type) {
      case CVT_BOOL:
      case CVT_NUMBER:
        fprintf(file, "%s=%d\n", configLinks[i].lpcsName, *(DWORD *)(configLinks[i].lpvValue));
        break;
      case CVT_FLOAT:
        fprintf(file, "%s=%f\n", configLinks[i].lpcsName, *(FLOAT *)(configLinks[i].lpvValue));
        break;
    }
  }

  fclose(file);

  return TRUE;
}

/**
 * Configuration getters.
 */
BOOL IsLogEnabled()
{
  return (g_config.dwLogLevel > 0);
}

DWORD GetLogLevel()
{
  return g_config.dwLogLevel;
}

DWORD GetMessageWaitTimeMinMs()
{
  return g_config.dwMessageWaitTimeMinMs;
}

DWORD GetMessageWaitTimeMaxMs()
{
  return g_config.dwMessageWaitTimeMaxMs;
}

DWORD GetMessageWaitTimeThresholdMs()
{
  return g_config.dwMessageWaitTimeThresholdMs;
}

DWORD GetMessageProcessingTimeThresholdMs()
{
  return g_config.dwMessageProcessingTimeThresholdMs;
}
