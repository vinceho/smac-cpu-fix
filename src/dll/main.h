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
#ifndef MAIN_H
#define MAIN_H

#include <windows.h>

#ifdef BUILD_DLL
#define PATCH_API __declspec(dllexport)
#else
#define PATCH_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

BOOL PATCH_API WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvReserved);

#ifdef __cplusplus
}
#endif

#endif // MAIN_H
