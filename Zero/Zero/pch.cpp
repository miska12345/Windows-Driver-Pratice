#include "pch.h"

#define DRIVER_PREFIX "Zero"

UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\Zerox");
UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\Zerox");
PDEVICE_OBJECT deviceObj = nullptr;

void ZeroUnload(PDRIVER_OBJECT DriverObject);
NTSTATUS ZeroCreateClose(PDRIVER_OBJECT DriverObject, PIRP irp);
NTSTATUS ZeroRead(PDRIVER_OBJECT DriverObject, PIRP irp);
NTSTATUS ZeroWrite(PDRIVER_OBJECT DriverObject, PIRP irp);
NTSTATUS CompleteIrp(PIRP irp, NTSTATUS status = STATUS_SUCCESS, ULONG_PTR info = 0) {
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = info;
	IoCompleteRequest(irp, 0);
	return status;
}

extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	KdPrint(("Driver Entry\n"));
	UNREFERENCED_PARAMETER(RegistryPath);
	DriverObject->DriverUnload = ZeroUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverObject->MajorFunction[IRP_MJ_CLOSE] = (PDRIVER_DISPATCH)ZeroCreateClose;
	DriverObject->MajorFunction[IRP_MJ_READ] = (PDRIVER_DISPATCH)ZeroRead;
	DriverObject->MajorFunction[IRP_MJ_WRITE] = (PDRIVER_DISPATCH)ZeroWrite;
	auto status = STATUS_SUCCESS;
	do {
		status = IoCreateDevice(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, 0, FALSE, &deviceObj);
		if (!NT_SUCCESS(status)) {
			KdPrint(("Failed to create device\n"));
			break;
		}
		deviceObj->Flags |= DO_DIRECT_IO;

		status = IoCreateSymbolicLink(&symLink, &devName);
		if (!NT_SUCCESS(status)) {
			KdPrint(("failed to create symbolic link\n"));
			break;
		}
	} while (false);
	if (!NT_SUCCESS(status)) {
		if (deviceObj) {
			IoDeleteDevice(deviceObj);
		}
	}
	return status;
}

void ZeroUnload(PDRIVER_OBJECT DriverObject) {
	IoDeleteSymbolicLink(&symLink);
	IoDeleteDevice(deviceObj);
	UNREFERENCED_PARAMETER(DriverObject);
}

NTSTATUS ZeroCreateClose(PDRIVER_OBJECT DriverObject, PIRP irp) {
	KdPrint(("create close\n"));
	UNREFERENCED_PARAMETER(DriverObject);
	CompleteIrp(irp);
	return STATUS_SUCCESS;
}

NTSTATUS ZeroRead(PDRIVER_OBJECT DriverObject, PIRP irp) {
	
	UNREFERENCED_PARAMETER(DriverObject);
	// Set stack location
	auto stack = IoGetCurrentIrpStackLocation(irp);
	auto len = stack->Parameters.Read.Length;
	if (len == 0) {
		return CompleteIrp(irp, STATUS_INVALID_BUFFER_SIZE);
	}

	// Map user buffer to system memory
	auto buffer = MmGetSystemAddressForMdlSafe(irp->MdlAddress, NormalPagePriority);
	KdPrint(("read buffer %p\n", buffer));
	if (!buffer) {
		return CompleteIrp(irp, STATUS_INSUFFICIENT_RESOURCES);
	}
	memset(buffer, 0, len);
	return CompleteIrp(irp, STATUS_SUCCESS, len);
}

NTSTATUS ZeroWrite(PDRIVER_OBJECT DriverObject, PIRP irp) {
	KdPrint(("write\n"));
	UNREFERENCED_PARAMETER(DriverObject);
	auto stack = IoGetCurrentIrpStackLocation(irp);
	auto len = stack->Parameters.Write.Length;
	return CompleteIrp(irp, STATUS_SUCCESS, len);
}

