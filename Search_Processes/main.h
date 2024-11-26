#pragma once
#define DebugPrint(...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,__VA_ARGS__)
extern "C"
{
#include <ntifs.h>
#include <ntddk.h>
#include "Search.h"



#define		CTRLCODE_BASE 0x8000

#define		MYCTRL_CODE(i) \
	CTL_CODE(FILE_DEVICE_UNKNOWN,CTRLCODE_BASE + i, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define		IOCTL_PROCWATCH		MYCTRL_CODE(0)



    UNICODE_STRING EventName;  //�¼����������
    HANDLE hEventHandle;    //�¼�����ľ��
    PKEVENT pKevent;    //ָ���¼�����




	NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);  //��ں���
	VOID DriverUnloadRoutine(IN PDRIVER_OBJECT DriverObject);	//ж�����̺���
	NTSTATUS IoctrlDispatch(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp); //IO_CONTROL���̴�����
	VOID InitEvent();	//��ʼ���¼�����




}