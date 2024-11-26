#include "Search.h"

int count = 0;
struct pEprocessInfoArray* m_pEprocessInfoArray = NULL;

VOID* Get_PspCidTable()
{
    __try {
        ////先获取到  48 8B 05 76 74 F5 FF          mov     rax, qword ptr cs:KeNumberProcessorsGroup0+12h  的地址
        //ULONG64 Target_Addr = (ULONG64)(PspReferenceCidTableEntry) + 35;

        ////获取到相对偏移
        //int Offset = *(unsigned int*)(Target_Addr + 3);

        ////获取Nt!CidTable的地址
        //return (VOID*)(Target_Addr + Offset);


        //从PsLookupProcessByProcessId获取到   E8 87 00 00 00                call    PspReferenceCidTableEntry的地址
        ULONG64 Target_Addr = (ULONG64)(PsLookupProcessByProcessId)+37;
        int Offset = 0;
        __try
        {
            //获取到相对偏移
            Offset = *(unsigned int*)(Target_Addr);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            // 如果发生异常，说明地址非法
            DebugPrint("地址不合法!");
            return 0;
        }
        //获取PspReferenceCidTableEntry的地址
        Target_Addr = Target_Addr +4+ Offset; //原来是+5，还要扣掉E8


        //获取到  48 8B 05 76 74 F5 FF          mov     rax, qword ptr cs:KeNumberProcessorsGroup0+12h  的地址
        Target_Addr = Target_Addr + 26;

        __try
        {
            //获取到相对偏移
            Offset = *(unsigned int*)(Target_Addr + 3);

        }

        __except (EXCEPTION_EXECUTE_HANDLER) {
            // 如果发生异常，说明地址非法
            DebugPrint("地址不合法!");
            return 0;
        }

        //获取Nt!CidTable的地址
        return (VOID*)(Target_Addr + 7 + Offset);


    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        // 如果发生异常，说明地址非法
        DebugPrint("地址不合法!");
        return 0;
    }

}



ULONG64 Decrypt_Pid(ULONG64* PspCidTable,ULONG64 Pid)  //模拟未导出函数ExpLookupHandleTableEntry
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
        DebugPrint("Encrypt_Pid函数出错!");
        return 0;
    }
   
}




void ExtractFileNameFromPath(  //从全路径提取出进程名
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



BOOLEAN IsAddressValid(PVOID Address)   //判断地址是否有效
{
    return MmIsAddressValid(Address);
}




void ConvertUnicodeStringToCharArray(UNICODE_STRING* pUnicodeString, char* ExeName, size_t ExeNameSize)
{
    ANSI_STRING ansiString;
    RtlZeroMemory(&ansiString, sizeof(ANSI_STRING));

    // 分配 ANSI_STRING 的缓冲区，长度根据 ExeNameSize 来调整
    ansiString.Buffer = ExeName;
    ansiString.Length = 0;
    ansiString.MaximumLength = (USHORT)(ExeNameSize - 1); // 留出一个位置给 null 字符

    // 转换 Unicode 字符串到 ANSI 字符串
    NTSTATUS status = RtlUnicodeStringToAnsiString(&ansiString, pUnicodeString, FALSE);

    if (NT_SUCCESS(status))
    {
        // 确保 null 终止符
        ExeName[ansiString.Length] = '\0';
    }
    else
    {
        // 如果转换失败，可以进行错误处理
        DebugPrint("Unicode to ANSI conversion failed\n");
        ExeName[0] = '\0';  // 清空 ExeName
    }
}



bool Verify_EProcess(PEPROCESS m_pErocess,int count) //验证是否为合法的EPROCESS对象
{

    __try
    {

        if (!IsAddressValid(m_pErocess)) //不合法的地址直接退
        {
            return false;
        }

        //EPROCESS->SectionObject(_SECTION)->ControlArea (_CONTROL_AREA)->FilePointer( _FILE_OBJECT)

        //首先获取到_SECTION，在win10 22H2版本的偏移是0x518
        _SECTION* P_SECTION = (_SECTION*)(*(ULONG64*)((ULONG64)m_pErocess + 0x518));

        //获取到ControlArea结构体
        _CONTROL_AREA* m_CONTROL_AREA = P_SECTION->u1.ControlArea;

        //找到_FILE_OBJECT结构体
        _FILE_OBJECT* FILE_OBJECT = (_FILE_OBJECT*)(((ULONG64)m_CONTROL_AREA->FilePointer.Object & 0xFFFFFFFFFFFFFFF0));



        //获取到全路径
        UNICODE_STRING* Process_PathName = &FILE_OBJECT->FileName;

        //进程名
        UNICODE_STRING FileName;

        //提取进程名
        ExtractFileNameFromPath(Process_PathName, &FileName);


        //从EPROCESS获取到进程名
        char* EProcess_FileName = (char*)((ULONG64)m_pErocess + 0x5a8);

        ANSI_STRING ansiString;

        // 初始化 ANSI_STRING
        RtlInitAnsiString(&ansiString, EProcess_FileName);

        // 将ANSI_STRING 转换为 UNICODE_STRING
        UNICODE_STRING U_EProcess_FileName;
        NTSTATUS status = RtlAnsiStringToUnicodeString(&U_EProcess_FileName, &ansiString, TRUE);
        if (!NT_SUCCESS(status))
        {
            DebugPrint("获取EProcess_FileName失败\n");
            return false;
        }


        //将进程名进行对比
        if (!RtlCompareUnicodeString(&FileName, &U_EProcess_FileName, TRUE))
        {
            m_pEprocessInfoArray[count].m_PEprocess = m_pErocess;
            ConvertUnicodeStringToCharArray(&U_EProcess_FileName, m_pEprocessInfoArray[count].ExeName, 30);
            //m_pEprocessInfoArray[count].m_Unicode_string = (UNICODE_STRING*)ExAllocatePool(NonPagedPool, sizeof(UNICODE_STRING));
            //if (m_pEprocessInfoArray[count].m_Unicode_string == NULL) {
            //    DebugPrint("Failed to allocate memory for m_Unicode_string\n");
            //    return false; // 或者继续下一个循环
            //}

            //m_pEprocessInfoArray[count].m_Unicode_string->Buffer = (PWCHAR)ExAllocatePool(NonPagedPool, U_EProcess_FileName.MaximumLength);
            //if (m_pEprocessInfoArray[count].m_Unicode_string->Buffer == NULL) {
            //    DebugPrint("Failed to allocate buffer for UNICODE_STRING\n");
            //    ExFreePool(m_pEprocessInfoArray[count].m_Unicode_string); // 释放前面分配的结构
            //    return false; // 或者继续下一个循环
            //}

            //m_pEprocessInfoArray[count].m_Unicode_string->Length = 0;
            //m_pEprocessInfoArray[count].m_Unicode_string->MaximumLength = U_EProcess_FileName.MaximumLength;

            //// 复制数据
            //RtlCopyUnicodeString(m_pEprocessInfoArray[count].m_Unicode_string, &U_EProcess_FileName);

            return true;
        }
        else
            return false;
    }
    
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        //DebugPrint("Verify_EProcess函数出错!");
        return false;
    }
}


NTSTATUS UnicodeStringToCharArray(
    const UNICODE_STRING* unicodeString,
    char* charArray,
    SIZE_T maxCharArraySize
) {
    ANSI_STRING ansiString;

    // 输入参数检查
    if ((IsAddressValid((PVOID)unicodeString) == FALSE ) || (IsAddressValid(unicodeString->Buffer)==FALSE)|| charArray == NULL || maxCharArraySize == 0) {
        DebugPrint("Invalid input parameters\n");
        return STATUS_INVALID_PARAMETER;
    }



    // 检查当前 IRQL
    if (KeGetCurrentIrql() > APC_LEVEL) {
        DebugPrint("Invalid IRQL: %d. Function cannot be executed at this IRQL.\n", KeGetCurrentIrql());
        return STATUS_INVALID_DEVICE_STATE;
    }
    // 初始化 ANSI_STRING
    ansiString.Buffer = charArray;
    ansiString.MaximumLength = (USHORT)maxCharArraySize;
    ansiString.Length = 0;



    __try {
        // 将 UNICODE_STRING 转换为 ANSI_STRING
        NTSTATUS status = RtlUnicodeStringToAnsiString(&ansiString, unicodeString, FALSE);
        if (!NT_SUCCESS(status)) {
            DebugPrint("Failed to convert UNICODE to ANSI, Status: 0x%X\n", status);
            return status;
        }

        // 确保输出的字符数组以 '\0' 结尾
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



bool Get_Eprocess() //通过PspCidTable表获取全部的EPROCESS,当然，会存在一些误判，所以要结合一些判断条件
{
    count = 0;
    // 每个数组元素的大小是 sizeof(PEPROCESS)
    SIZE_T arraySize = PEPROCESS_NUM * sizeof(PEPROCESS);

    // 使用分页池分配内存
    m_pEprocessInfoArray = (struct pEprocessInfoArray*)ExAllocatePoolWithTag(NonPagedPool, arraySize, TAG);

    
    for (int pid = 0; pid < 65535; pid+=4) //暴力搜索EPROCESS
    {
        PEPROCESS m_EProcess = NULL;
        __try
        {

            ULONG64* Temp = (ULONG64*)Decrypt_Pid((ULONG64*)(*(ULONG64*)Get_PspCidTable()), pid);
            m_EProcess = (PEPROCESS)((((*(long long*)Temp) >> 16)) & 0xFFFFFFFFFFFFFFF0);
        }

        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            //DebugPrint("Encrypt_Pid函数出错!");
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
    //    DebugPrint("第%d个EPROCESS结构体:%llx ",i, (ULONG64)(m_pEprocessInfoArray[i].m_PEprocess));
    //    char charArray[256] = { 0 };
    //    
    //    DebugPrint("进程名为%s\n", m_pEprocessInfoArray[i].ExeName);
    //}



    return true;
}