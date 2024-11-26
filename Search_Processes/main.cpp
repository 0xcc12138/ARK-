#include "main.h"


VOID InitEvent()
{
    RtlInitUnicodeString(&EventName, L"\\BaseNamedObjects\\ProcEvent");
    PKEVENT pKevent = IoCreateNotificationEvent(&EventName, &hEventHandle);
    if (pKevent)
    {
        DebugPrint("创建对象成功！");
    }
    else
    {
        DebugPrint("创建对象失败！");
    }
    KeClearEvent(pKevent);            // 设置为非受信状态   
}


//处理设备对象操作
NTSTATUS CommonDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    DeviceObject;
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0L;
    IoCompleteRequest(Irp, 0);
    return Irp->IoStatus.Status;
}


NTSTATUS IoctrlDispatch(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
    pDeviceObject;  // 这里的 pDeviceObject 是传递给函数的设备对象指针，但在函数体中并未使用。

    NTSTATUS ntStatus = STATUS_SUCCESS;  // 初始化返回的状态为 STATUS_SUCCESS，表示操作成功。
    PVOID pUserBuffer = NULL;  // 初始化指向用户空间缓冲区的指针为空。
    ULONG ulInputSize = 0;  // 初始化输入缓冲区大小为0。
    ULONG ulOutputSize = 0;  // 初始化输出缓冲区大小为0。
    PIO_STACK_LOCATION pIrpStack = NULL;  // 用于保存 IRP 堆栈位置的指针，IRP 堆栈包含了 IO 请求的参数。
    ULONG ulIoCtrlCode = 0;  // 用于保存传入的 IO 控制代码。
  

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);  // 获取当前 IRP 堆栈位置。

    pUserBuffer = pIrp->AssociatedIrp.SystemBuffer;  // 获取与 IRP 关联的系统缓冲区（用户数据）。


    ulIoCtrlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;  // 获取 IO 控制代码。
    ulInputSize = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;  // 获取输入缓冲区的大小。
    ulOutputSize = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;  // 获取输出缓冲区的大小。

    switch (ulIoCtrlCode)  // 根据传入的 IO 控制代码执行不同的处理。
    {
    case IOCTL_PROCWATCH:
    {
       
        // 确保输出缓冲区大小足够
        if (ulOutputSize < sizeof(struct pEprocessInfoArray) * count) {
            // 如果输出缓冲区不够大，返回错误
            ntStatus = STATUS_INVALID_PARAMETER;
            pIrp->IoStatus.Status = ntStatus;
            pIrp->IoStatus.Information = 0;
            IoCompleteRequest(pIrp, IO_NO_INCREMENT);
            return ntStatus;
        }
        Get_Eprocess();


        // 将 pEprocessInfoArray 数组的内容复制到用户缓冲区
        memcpy(pUserBuffer, m_pEprocessInfoArray, sizeof(struct pEprocessInfoArray) * count);
        
        if (!m_pEprocessInfoArray)
        {
            ExFreePoolWithTag(m_pEprocessInfoArray, TAG);
            m_pEprocessInfoArray = NULL;  // 防止后续不小心访问已释放的内存
        }

    
        

        // 更新输出大小，包含进程信息数组的数据
        ulOutputSize = sizeof(struct pEprocessInfoArray) * count;
        ntStatus = STATUS_SUCCESS;
        break;
    }

    default:
        // 如果 IO 控制代码不匹配，返回无效参数错误。
        ntStatus = STATUS_INVALID_PARAMETER;
        ulOutputSize = 0;
        break;
    }

    pIrp->IoStatus.Status = ntStatus;  // 将操作结果状态存储到 IRP 中。
    pIrp->IoStatus.Information = ulOutputSize;  // 将输出缓冲区大小存储到 IRP 中。

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);  // 完成 IRP 请求，并通知系统 IRP 处理完毕。


    if (pKevent)
    {
        // 完成操作后触发事件，解除等待
        KeSetEvent(pKevent, 0, FALSE);  // 设置为有信号状态
        DebugPrint("事件已触发，继续执行...\n");
    }

    return ntStatus;  // 返回处理状态，通常为 STATUS_SUCCESS 或 STATUS_INVALID_PARAMETER。
}





NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);
    KdPrint(("Create!"));

    // 驱动程序卸载例程&注册例程
    DriverObject->DriverUnload = DriverUnloadRoutine;
    //创建设备
    NTSTATUS status;
    PDEVICE_OBJECT DeviceObject = NULL;
    UNICODE_STRING DeviceName;
    RtlInitUnicodeString(&DeviceName, L"\\Device\\Search_Processes_Device");
    status = IoCreateDevice(
        DriverObject,                // 驱动程序对象
        0,                           // 设备扩展大小
        &DeviceName,                 // 设备名称
        FILE_DEVICE_UNKNOWN,         // 设备类型
        0,                           // 设备特征
        FALSE,                       // 非独占设备
        &DeviceObject                // 返回的设备对象指针
    );

    if (!NT_SUCCESS(status))
    {
        KdPrint(("Failed to create device: %X\n", status));
        return status;
    }
    KdPrint(("Device created successfully\n"));

    UNICODE_STRING symbolicLink = RTL_CONSTANT_STRING(L"\\??\\Search_Processes_Link");
    status = IoCreateSymbolicLink(&symbolicLink, &DeviceName);
    if (!NT_SUCCESS(status))
    {
        KdPrint(("Failed to create device: %X\n", status));
        return status;
    }
    KdPrint(("Device created successfully\n"));



    //注册IRP例程
    for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
    {
        DriverObject->MajorFunction[i] = CommonDispatch;
    }

    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoctrlDispatch;




    InitEvent();

    return STATUS_SUCCESS;
}









VOID DriverUnloadRoutine(IN PDRIVER_OBJECT DriverObject)
{
    if (DriverObject->DeviceObject != NULL)
    {
        UNICODE_STRING symbolicLink = RTL_CONSTANT_STRING(L"\\??\\Search_Processes_Link");
        IoDeleteSymbolicLink(&symbolicLink); //关闭符号链接
        if (hEventHandle)
        {
            ZwClose(hEventHandle); // 关闭句柄，减少引用计数
            hEventHandle = NULL;
        }
        IoDeleteDevice(DriverObject->DeviceObject); //删除设备
    }

    DbgPrint("Driver unloaded\n");
}