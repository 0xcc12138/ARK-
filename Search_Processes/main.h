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



    UNICODE_STRING EventName;  //事件对象的名字
    HANDLE hEventHandle;    //事件对象的句柄
    PKEVENT pKevent;    //指向事件对象




	NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);  //入口函数
	VOID DriverUnloadRoutine(IN PDRIVER_OBJECT DriverObject);	//卸载例程函数
	NTSTATUS IoctrlDispatch(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp); //IO_CONTROL例程处理函数
	VOID InitEvent();	//初始化事件对象




}