#include "Search.h"

int count = 0;
struct pEprocessInfoArray* m_pEprocessInfoArray = NULL;

VOID* Get_PspCidTable()
{
    __try {
        ////�Ȼ�ȡ��  48 8B 05 76 74 F5 FF          mov     rax, qword ptr cs:KeNumberProcessorsGroup0+12h  �ĵ�ַ
        //ULONG64 Target_Addr = (ULONG64)(PspReferenceCidTableEntry) + 35;

        ////��ȡ�����ƫ��
        //int Offset = *(unsigned int*)(Target_Addr + 3);

        ////��ȡNt!CidTable�ĵ�ַ
        //return (VOID*)(Target_Addr + Offset);


        //��PsLookupProcessByProcessId��ȡ��   E8 87 00 00 00                call    PspReferenceCidTableEntry�ĵ�ַ
        ULONG64 Target_Addr = (ULONG64)(PsLookupProcessByProcessId)+37;
        int Offset = 0;
        __try
        {
            //��ȡ�����ƫ��
            Offset = *(unsigned int*)(Target_Addr);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            // ��������쳣��˵����ַ�Ƿ�
            DebugPrint("��ַ���Ϸ�!");
            return 0;
        }
        //��ȡPspReferenceCidTableEntry�ĵ�ַ
        Target_Addr = Target_Addr +4+ Offset; //ԭ����+5����Ҫ�۵�E8


        //��ȡ��  48 8B 05 76 74 F5 FF          mov     rax, qword ptr cs:KeNumberProcessorsGroup0+12h  �ĵ�ַ
        Target_Addr = Target_Addr + 26;

        __try
        {
            //��ȡ�����ƫ��
            Offset = *(unsigned int*)(Target_Addr + 3);

        }

        __except (EXCEPTION_EXECUTE_HANDLER) {
            // ��������쳣��˵����ַ�Ƿ�
            DebugPrint("��ַ���Ϸ�!");
            return 0;
        }

        //��ȡNt!CidTable�ĵ�ַ
        return (VOID*)(Target_Addr + 7 + Offset);


    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        // ��������쳣��˵����ַ�Ƿ�
        DebugPrint("��ַ���Ϸ�!");
        return 0;
    }

}



ULONG64 Decrypt_Pid(ULONG64* PspCidTable,ULONG64 Pid)  //ģ��δ��������ExpLookupHandleTableEntry
{
    ULONG64 v2 = 0;;
    ULONG64 v3 = 0;;

    __try
    {
        v2 = Pid & 0xFFFFFFFFFFFFFFFCui64;
        if (v2 >= *PspCidTable)
            return 0i64;
        v3 = *((ULONG64*)PspCidTable + 1);
        if ((v3 & 3) == 1)
            return *(ULONG64*)(v3 + 8 * (v2 >> 10) - 1) + 4 * (v2 & 0x3FF);
        if ((v3 & 3) != 0)
            return *(ULONG64*)(*(ULONG64*)(v3 + 8 * (v2 >> 19) - 2) + 8 * ((v2 >> 10) & 0x1FF)) + 4 * (v2 & 0x3FF);
        return v3 + 4 * v2;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        DebugPrint("Encrypt_Pid��������!");
        return 0;
    }
   
}




void ExtractFileNameFromPath(  //��ȫ·����ȡ��������
    PUNICODE_STRING FullPath,
    PUNICODE_STRING FileName
) {
    USHORT i;

    // Start from the end of the string and search for the last backslash
    for (i = FullPath->Length / sizeof(WCHAR); i > 0; i--) {
        if (FullPath->Buffer[i - 1] == L'\\') {
            break;
        }
    }

    // Set the FileName string to the part after the last backslash
    FileName->Buffer = &FullPath->Buffer[i];
    FileName->Length = FullPath->Length - i * sizeof(WCHAR);
    FileName->MaximumLength = FileName->Length;
}



BOOLEAN IsAddressValid(PVOID Address)   //�жϵ�ַ�Ƿ���Ч
{
    return MmIsAddressValid(Address);
}




void ConvertUnicodeStringToCharArray(UNICODE_STRING* pUnicodeString, char* ExeName, size_t ExeNameSize)
{
    ANSI_STRING ansiString;
    RtlZeroMemory(&ansiString, sizeof(ANSI_STRING));

    // ���� ANSI_STRING �Ļ����������ȸ��� ExeNameSize ������
    ansiString.Buffer = ExeName;
    ansiString.Length = 0;
    ansiString.MaximumLength = (USHORT)(ExeNameSize - 1); // ����һ��λ�ø� null �ַ�

    // ת�� Unicode �ַ����� ANSI �ַ���
    NTSTATUS status = RtlUnicodeStringToAnsiString(&ansiString, pUnicodeString, FALSE);

    if (NT_SUCCESS(status))
    {
        // ȷ�� null ��ֹ��
        ExeName[ansiString.Length] = '\0';
    }
    else
    {
        // ���ת��ʧ�ܣ����Խ��д�����
        DebugPrint("Unicode to ANSI conversion failed\n");
        ExeName[0] = '\0';  // ��� ExeName
    }
}



bool Verify_EProcess(PEPROCESS m_pErocess,int count) //��֤�Ƿ�Ϊ�Ϸ���EPROCESS����
{

    __try
    {

        if (!IsAddressValid(m_pErocess)) //���Ϸ��ĵ�ֱַ����
        {
            return false;
        }

        //EPROCESS->SectionObject(_SECTION)->ControlArea (_CONTROL_AREA)->FilePointer( _FILE_OBJECT)

        //���Ȼ�ȡ��_SECTION����win10 22H2�汾��ƫ����0x518
        _SECTION* P_SECTION = (_SECTION*)(*(ULONG64*)((ULONG64)m_pErocess + 0x518));

        //��ȡ��ControlArea�ṹ��
        _CONTROL_AREA* m_CONTROL_AREA = P_SECTION->u1.ControlArea;

        //�ҵ�_FILE_OBJECT�ṹ��
        _FILE_OBJECT* FILE_OBJECT = (_FILE_OBJECT*)(((ULONG64)m_CONTROL_AREA->FilePointer.Object & 0xFFFFFFFFFFFFFFF0));



        //��ȡ��ȫ·��
        UNICODE_STRING* Process_PathName = &FILE_OBJECT->FileName;

        //������
        UNICODE_STRING FileName;

        //��ȡ������
        ExtractFileNameFromPath(Process_PathName, &FileName);


        //��EPROCESS��ȡ��������
        char* EProcess_FileName = (char*)((ULONG64)m_pErocess + 0x5a8);

        ANSI_STRING ansiString;

        // ��ʼ�� ANSI_STRING
        RtlInitAnsiString(&ansiString, EProcess_FileName);

        // ��ANSI_STRING ת��Ϊ UNICODE_STRING
        UNICODE_STRING U_EProcess_FileName;
        NTSTATUS status = RtlAnsiStringToUnicodeString(&U_EProcess_FileName, &ansiString, TRUE);
        if (!NT_SUCCESS(status))
        {
            DebugPrint("��ȡEProcess_FileNameʧ��\n");
            return false;
        }


        //�����������жԱ�
        if (!RtlCompareUnicodeString(&FileName, &U_EProcess_FileName, TRUE))
        {
            m_pEprocessInfoArray[count].m_PEprocess = m_pErocess;
            ConvertUnicodeStringToCharArray(&U_EProcess_FileName, m_pEprocessInfoArray[count].ExeName, 30);
            //m_pEprocessInfoArray[count].m_Unicode_string = (UNICODE_STRING*)ExAllocatePool(NonPagedPool, sizeof(UNICODE_STRING));
            //if (m_pEprocessInfoArray[count].m_Unicode_string == NULL) {
            //    DebugPrint("Failed to allocate memory for m_Unicode_string\n");
            //    return false; // ���߼�����һ��ѭ��
            //}

            //m_pEprocessInfoArray[count].m_Unicode_string->Buffer = (PWCHAR)ExAllocatePool(NonPagedPool, U_EProcess_FileName.MaximumLength);
            //if (m_pEprocessInfoArray[count].m_Unicode_string->Buffer == NULL) {
            //    DebugPrint("Failed to allocate buffer for UNICODE_STRING\n");
            //    ExFreePool(m_pEprocessInfoArray[count].m_Unicode_string); // �ͷ�ǰ�����Ľṹ
            //    return false; // ���߼�����һ��ѭ��
            //}

            //m_pEprocessInfoArray[count].m_Unicode_string->Length = 0;
            //m_pEprocessInfoArray[count].m_Unicode_string->MaximumLength = U_EProcess_FileName.MaximumLength;

            //// ��������
            //RtlCopyUnicodeString(m_pEprocessInfoArray[count].m_Unicode_string, &U_EProcess_FileName);

            return true;
        }
        else
            return false;
    }
    
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        //DebugPrint("Verify_EProcess��������!");
        return false;
    }
}


NTSTATUS UnicodeStringToCharArray(
    const UNICODE_STRING* unicodeString,
    char* charArray,
    SIZE_T maxCharArraySize
) {
    ANSI_STRING ansiString;

    // ����������
    if ((IsAddressValid((PVOID)unicodeString) == FALSE ) || (IsAddressValid(unicodeString->Buffer)==FALSE)|| charArray == NULL || maxCharArraySize == 0) {
        DebugPrint("Invalid input parameters\n");
        return STATUS_INVALID_PARAMETER;
    }



    // ��鵱ǰ IRQL
    if (KeGetCurrentIrql() > APC_LEVEL) {
        DebugPrint("Invalid IRQL: %d. Function cannot be executed at this IRQL.\n", KeGetCurrentIrql());
        return STATUS_INVALID_DEVICE_STATE;
    }
    // ��ʼ�� ANSI_STRING
    ansiString.Buffer = charArray;
    ansiString.MaximumLength = (USHORT)maxCharArraySize;
    ansiString.Length = 0;



    __try {
        // �� UNICODE_STRING ת��Ϊ ANSI_STRING
        NTSTATUS status = RtlUnicodeStringToAnsiString(&ansiString, unicodeString, FALSE);
        if (!NT_SUCCESS(status)) {
            DebugPrint("Failed to convert UNICODE to ANSI, Status: 0x%X\n", status);
            return status;
        }

        // ȷ��������ַ������� '\0' ��β
        if (ansiString.Length < ansiString.MaximumLength - 1) {
            ansiString.Buffer[ansiString.Length] = '\0';
        }
        else {
            ansiString.Buffer[ansiString.MaximumLength - 1] = '\0';
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        DebugPrint("Exception occurred in UnicodeStringToCharArray: 0x%X\n", GetExceptionCode());
        return STATUS_UNHANDLED_EXCEPTION;
    }

    return STATUS_SUCCESS;
}



bool Get_Eprocess() //ͨ��PspCidTable���ȡȫ����EPROCESS,��Ȼ�������һЩ���У�����Ҫ���һЩ�ж�����
{
    count = 0;
    // ÿ������Ԫ�صĴ�С�� sizeof(PEPROCESS)
    SIZE_T arraySize = PEPROCESS_NUM * sizeof(PEPROCESS);

    // ʹ�÷�ҳ�ط����ڴ�
    m_pEprocessInfoArray = (struct pEprocessInfoArray*)ExAllocatePoolWithTag(NonPagedPool, arraySize, TAG);

    
    for (int pid = 0; pid < 65535; pid+=4) //��������EPROCESS
    {
        PEPROCESS m_EProcess = NULL;
        __try
        {

            ULONG64* Temp = (ULONG64*)Decrypt_Pid((ULONG64*)(*(ULONG64*)Get_PspCidTable()), pid);
            m_EProcess = (PEPROCESS)((((*(long long*)Temp) >> 16)) & 0xFFFFFFFFFFFFFFF0);
        }

        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            //DebugPrint("Encrypt_Pid��������!");
            continue;
        }

        if (!IsAddressValid(m_EProcess))
        {
            continue;
        }


        if (Verify_EProcess(m_EProcess,count))
        {
            count++;
        }
    }



    //for (int i = 0; i < count; i++)
    //{
    //    DebugPrint("��%d��EPROCESS�ṹ��:%llx ",i, (ULONG64)(m_pEprocessInfoArray[i].m_PEprocess));
    //    char charArray[256] = { 0 };
    //    
    //    DebugPrint("������Ϊ%s\n", m_pEprocessInfoArray[i].ExeName);
    //}



    return true;
}