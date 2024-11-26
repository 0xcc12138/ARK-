#pragma once
#include <windows.h>
#define		CTRLCODE_BASE 0x8000
#define		MYCTRL_CODE(i) \
	CTL_CODE(FILE_DEVICE_UNKNOWN,CTRLCODE_BASE + i, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define		IOCTL_PROCWATCH		MYCTRL_CODE(0)


struct UNICODE_STRING
{
    USHORT Length;                                                          //0x0
    USHORT MaximumLength;                                                   //0x2
    WCHAR* Buffer;                                                          //0x8
};

struct pEprocessInfoArray
{
    unsigned long long m_PEprocess = NULL;
    char ExeName[30];
};



extern "C"
{
    extern int count;
    extern pEprocessInfoArray* pArray;
}