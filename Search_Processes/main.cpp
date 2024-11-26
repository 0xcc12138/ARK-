#include "main.h"


VOID InitEvent()
{
    RtlInitUnicodeString(&EventName, L"\\BaseNamedObjects\\ProcEvent");
    PKEVENT pKevent = IoCreateNotificationEvent(&EventName, &hEventHandle);
    if (pKevent)
    {
        DebugPrint("��������ɹ���");
    }
    else
    {
        DebugPrint("��������ʧ�ܣ�");
    }
    KeClearEvent(pKevent);            // ����Ϊ������״̬   
}


//�����豸�������
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
    pDeviceObject;  // ����� pDeviceObject �Ǵ��ݸ��������豸����ָ�룬���ں������в�δʹ�á�

    NTSTATUS ntStatus = STATUS_SUCCESS;  // ��ʼ�����ص�״̬Ϊ STATUS_SUCCESS����ʾ�����ɹ���
    PVOID pUserBuffer = NULL;  // ��ʼ��ָ���û��ռ仺������ָ��Ϊ�ա�
    ULONG ulInputSize = 0;  // ��ʼ�����뻺������СΪ0��
    ULONG ulOutputSize = 0;  // ��ʼ�������������СΪ0��
    PIO_STACK_LOCATION pIrpStack = NULL;  // ���ڱ��� IRP ��ջλ�õ�ָ�룬IRP ��ջ������ IO ����Ĳ�����
    ULONG ulIoCtrlCode = 0;  // ���ڱ��洫��� IO ���ƴ��롣
  

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);  // ��ȡ��ǰ IRP ��ջλ�á�

    pUserBuffer = pIrp->AssociatedIrp.SystemBuffer;  // ��ȡ�� IRP ������ϵͳ���������û����ݣ���


    ulIoCtrlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;  // ��ȡ IO ���ƴ��롣
    ulInputSize = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;  // ��ȡ���뻺�����Ĵ�С��
    ulOutputSize = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;  // ��ȡ����������Ĵ�С��

    switch (ulIoCtrlCode)  // ���ݴ���� IO ���ƴ���ִ�в�ͬ�Ĵ���
    {
    case IOCTL_PROCWATCH:
    {
       
        // ȷ�������������С�㹻
        if (ulOutputSize < sizeof(struct pEprocessInfoArray) * count) {
            // �����������������󣬷��ش���
            ntStatus = STATUS_INVALID_PARAMETER;
            pIrp->IoStatus.Status = ntStatus;
            pIrp->IoStatus.Information = 0;
            IoCompleteRequest(pIrp, IO_NO_INCREMENT);
            return ntStatus;
        }
        Get_Eprocess();


        // �� pEprocessInfoArray ��������ݸ��Ƶ��û�������
        memcpy(pUserBuffer, m_pEprocessInfoArray, sizeof(struct pEprocessInfoArray) * count);
        
        if (!m_pEprocessInfoArray)
        {
            ExFreePoolWithTag(m_pEprocessInfoArray, TAG);
            m_pEprocessInfoArray = NULL;  // ��ֹ������С�ķ������ͷŵ��ڴ�
        }

    
        

        // ���������С������������Ϣ���������
        ulOutputSize = sizeof(struct pEprocessInfoArray) * count;
        ntStatus = STATUS_SUCCESS;
        break;
    }

    default:
        // ��� IO ���ƴ��벻ƥ�䣬������Ч��������
        ntStatus = STATUS_INVALID_PARAMETER;
        ulOutputSize = 0;
        break;
    }

    pIrp->IoStatus.Status = ntStatus;  // ���������״̬�洢�� IRP �С�
    pIrp->IoStatus.Information = ulOutputSize;  // �������������С�洢�� IRP �С�

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);  // ��� IRP ���󣬲�֪ͨϵͳ IRP ������ϡ�


    if (pKevent)
    {
        // ��ɲ����󴥷��¼�������ȴ�
        KeSetEvent(pKevent, 0, FALSE);  // ����Ϊ���ź�״̬
        DebugPrint("�¼��Ѵ���������ִ��...\n");
    }

    return ntStatus;  // ���ش���״̬��ͨ��Ϊ STATUS_SUCCESS �� STATUS_INVALID_PARAMETER��
}





NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);
    KdPrint(("Create!"));

    // ��������ж������&ע������
    DriverObject->DriverUnload = DriverUnloadRoutine;
    //�����豸
    NTSTATUS status;
    PDEVICE_OBJECT DeviceObject = NULL;
    UNICODE_STRING DeviceName;
    RtlInitUnicodeString(&DeviceName, L"\\Device\\Search_Processes_Device");
    status = IoCreateDevice(
        DriverObject,                // �����������
        0,                           // �豸��չ��С
        &DeviceName,                 // �豸����
        FILE_DEVICE_UNKNOWN,         // �豸����
        0,                           // �豸����
        FALSE,                       // �Ƕ�ռ�豸
        &DeviceObject                // ���ص��豸����ָ��
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



    //ע��IRP����
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
        IoDeleteSymbolicLink(&symbolicLink); //�رշ�������
        if (hEventHandle)
        {
            ZwClose(hEventHandle); // �رվ�����������ü���
            hEventHandle = NULL;
        }
        IoDeleteDevice(DriverObject->DeviceObject); //ɾ���豸
    }

    DbgPrint("Driver unloaded\n");
}