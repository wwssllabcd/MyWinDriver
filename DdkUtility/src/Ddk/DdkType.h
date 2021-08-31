#pragma once

#include <windows.h>          //HMODULE, LoadLibrary(), FreeLibrary() ... etc
#include <bcrypt.h>           //NTSTATUS


typedef struct _LSA_UNICODE_STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

typedef struct _OBJDIR_INFORMATION
{
    UNICODE_STRING ObjectName;
    UNICODE_STRING ObjectTypeName;
    BYTE Data[1];
} OBJDIR_INFORMATION, * POBJDIR_INFORMATION;

typedef struct _OBJECT_ATTRIBUTES
{
    ULONG Length;
    HANDLE RootDirectory;
    UNICODE_STRING* ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;
    PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES;

typedef NTSTATUS(WINAPI* NTQUERYDIRECTORYOBJECT)(HANDLE,
    OBJDIR_INFORMATION*,
    DWORD,
    DWORD,
    DWORD,
    DWORD*,
    DWORD*);

typedef NTSTATUS(WINAPI* NTOPENDIRECTORYOBJECT)(HANDLE*,
    DWORD,
    OBJECT_ATTRIBUTES*);

typedef NTSTATUS(WINAPI* NTCLOSE)(HANDLE);

