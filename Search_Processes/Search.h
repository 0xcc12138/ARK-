#pragma once
#define DebugPrint(...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,__VA_ARGS__)
#include <ntifs.h>
#include <ntddk.h>


#define TAG 'rPEP' // �ڴ��ʶ�������ڵ��ԣ���PEPROCESS����
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

struct _CONTROL_AREA  //����ṹ�岻��ȫ
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
	////��������PspReferenceCidTableEntry
	//unsigned char* __fastcall PspReferenceCidTableEntry(__int64, char);

	////��������PsLookupProcessByProcessId
	// 
	//NTSTATUS  PsLookupProcessByProcessId(HANDLE ProcessId, PEPROCESS* Process);

	
    // ����ȫ�ֱ���
    extern int count;

    extern struct pEprocessInfoArray* m_pEprocessInfoArray; //ָ��pEprocessInfoArray�ṹ������

	VOID* Get_PspCidTable(); //��ȡPspCidTable��ĵ�ַ��ԭ����PsLookupProcessByProcessId���������� -> PspReferenceCidTableEntry -> PspCidTable

	ULONG64 Decrypt_Pid(ULONG64* PspCidTable, ULONG64 Pid);  //ģ��δ��������ExpLookupHandleTableEntry

	bool Get_Eprocess(); //ͨ��PspCidTable���ȡȫ����EPROCESS,��Ȼ�������һЩ���У�����Ҫ���һЩ�ж�����

    bool Verify_EProcess(PEPROCESS m_pErocess); //��֤�Ƿ�Ϊ�Ϸ���EPROCESS����

     
    void ExtractFileNameFromPath(   //��ȫ·����ȡ��������
        PUNICODE_STRING FullPath,
        PUNICODE_STRING FileName
    );


    NTSTATUS UnicodeStringToCharArray(   //��UNICODE_STRINGתΪCHAR����
        const UNICODE_STRING* unicodeString,
        char* charArray,
        SIZE_T maxCharArraySize
    );

}

