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
#include "hook.h"
#include "log.h"

/**
 * Based on `http://www.rohitab.com/discuss/topic/37089-c-hookmsgbox/`.
 */
LPDWORD SearchIat(LPCSTR functionName)
{
    LPVOID lpMapping = GetModuleHandle(NULL);

    if (lpMapping == NULL) {
      LogError("Cannot get current module handle.");
      return NULL;
    }

    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)lpMapping;

    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
      LogError("Invalid PE DOS header.");
      return NULL;
    }

    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS) ((LPBYTE)pDosHeader + pDosHeader->e_lfanew);

    if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE) {
      LogError("Invalid PE NT header.");
      return NULL;
    }

    PIMAGE_DATA_DIRECTORY pDataDirectory = &pNtHeaders->OptionalHeader.DataDirectory[1];
    PIMAGE_IMPORT_DESCRIPTOR pImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR) ((LPBYTE)pDosHeader + pDataDirectory->VirtualAddress);
    PIMAGE_THUNK_DATA32 pOriginalFirstThunk = (PIMAGE_THUNK_DATA32)((LPBYTE)pDosHeader + pImportDescriptor->OriginalFirstThunk);

    while (pOriginalFirstThunk != 0) {
        pOriginalFirstThunk = (PIMAGE_THUNK_DATA32)((LPBYTE)pDosHeader + pImportDescriptor->OriginalFirstThunk);

        PIMAGE_THUNK_DATA32 pFirstThunk = (PIMAGE_THUNK_DATA32)((LPBYTE)pDosHeader + pImportDescriptor->FirstThunk);

        while(pOriginalFirstThunk->u1.AddressOfData != 0)
        {
            PIMAGE_IMPORT_BY_NAME pImport = (PIMAGE_IMPORT_BY_NAME)((LPBYTE)pDosHeader + (DWORD)pOriginalFirstThunk->u1.AddressOfData);

            if (!((DWORD)pOriginalFirstThunk->u1.Function & (DWORD)IMAGE_ORDINAL_FLAG32)) {
                if(strcmp(functionName, (LPCSTR)pImport->Name) == 0) {
                    return (LPDWORD)&(pFirstThunk->u1.Function);
                }
            }

            pOriginalFirstThunk++;
            pFirstThunk++;
        }
        pImportDescriptor++;
    }

    return NULL;
}

BOOL HookFunction(LPCSTR functionName, LPDWORD newFunction)
{
    LPDWORD pOldFunction = SearchIat(functionName);
    DWORD accessProtectionValue, dummyAccessProtectionValue;

    int vProtect = VirtualProtect(pOldFunction, sizeof(LPDWORD), PAGE_EXECUTE_READWRITE, &accessProtectionValue);

    if (!vProtect) {
      LogError("Failed to access virtual address space.");
      return FALSE;
    }

    *pOldFunction = (DWORD)newFunction;

    vProtect = VirtualProtect(pOldFunction, sizeof(LPDWORD), accessProtectionValue, &dummyAccessProtectionValue);

    if (!vProtect) {
      LogError("Failed to lock virtual address space.");
      return FALSE;
    }

    return TRUE;
}
