#pragma once
#define DebugPrint(...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,__VA_ARGS__)
#include <ntifs.h>
#include <ntddk.h>


#define TAG 'rPEP' // 内存标识符，用于调试，存PEPROCESS数组
#define PEPROCESS_NUM 0x20000



struct pEprocessInfoArray
{
    PEPROCESS m_PEprocess = NULL;
    /*UNICODE_STRING* m_Unicode_string = NULL;*/
    char ExeName[30];
};


//0x40 bytes (sizeof)
struct _SECTION
{
    struct _RTL_BALANCED_NODE SectionNode;                                  //0x0
    ULONGLONG StartingVpn;                                                  //0x18
    ULONGLONG EndingVpn;                                                    //0x20
    union
    {
        struct _CONTROL_AREA* ControlArea;                                  //0x28
        struct _FILE_OBJECT* FileObject;                                    //0x28
        ULONGLONG RemoteImageFileObject : 1;                                  //0x28
        ULONGLONG RemoteDataFileObject : 1;                                   //0x28
    } u1;                                                                   //0x28
    ULONGLONG SizeOfSection;                                                //0x30
    union
    {
        ULONG LongFlags;                                                    //0x38
    } u;                                                                    //0x38
    ULONG InitialPageProtection : 12;                                         //0x3c
    ULONG SessionId : 19;                                                     //0x3c
    ULONG NoValidationNeeded : 1;                                             //0x3c
};

struct _EX_FAST_REF
{
    union
    {
        VOID* Object;                                                       //0x0
        ULONGLONG RefCnt : 4;                                                 //0x0
        ULONGLONG Value;                                                    //0x0
    };
};

struct _CONTROL_AREA  //这里结构体不完全
{
    struct _SEGMENT* Segment;                                               //0x0
    union
    {
        struct _LIST_ENTRY ListHead;                                        //0x8
        VOID* AweContext;                                                   //0x8
    };
    ULONGLONG NumberOfSectionReferences;                                    //0x18
    ULONGLONG NumberOfPfnReferences;                                        //0x20
    ULONGLONG NumberOfMappedViews;                                          //0x28
    ULONGLONG NumberOfUserReferences;                                       //0x30
    union
    {
        ULONG LongFlags;                                                    //0x38

    } u;                                                                    //0x38
    union
    {
        ULONG LongFlags;                                                    //0x3c

    } u1;                                                                   //0x3c
    struct _EX_FAST_REF FilePointer;
};





extern "C"
{
	////导出函数PspReferenceCidTableEntry
	//unsigned char* __fastcall PspReferenceCidTableEntry(__int64, char);

	////导出函数PsLookupProcessByProcessId
	// 
	//NTSTATUS  PsLookupProcessByProcessId(HANDLE ProcessId, PEPROCESS* Process);

	
    // 声明全局变量
    extern int count;

    extern struct pEprocessInfoArray* m_pEprocessInfoArray; //指向pEprocessInfoArray结构体数组

	VOID* Get_PspCidTable(); //获取PspCidTable表的地址，原理是PsLookupProcessByProcessId（被导出） -> PspReferenceCidTableEntry -> PspCidTable

	ULONG64 Decrypt_Pid(ULONG64* PspCidTable, ULONG64 Pid);  //模拟未导出函数ExpLookupHandleTableEntry

	bool Get_Eprocess(); //通过PspCidTable表获取全部的EPROCESS,当然，会存在一些误判，所以要结合一些判断条件

    bool Verify_EProcess(PEPROCESS m_pErocess); //验证是否为合法的EPROCESS对象

     
    void ExtractFileNameFromPath(   //从全路径提取出进程名
        PUNICODE_STRING FullPath,
        PUNICODE_STRING FileName
    );


    NTSTATUS UnicodeStringToCharArray(   //将UNICODE_STRING转为CHAR数组
        const UNICODE_STRING* unicodeString,
        char* charArray,
        SIZE_T maxCharArraySize
    );

}

