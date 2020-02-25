#include <ntddk.h>

#define DRIVER_TAG 'feeb'

UNICODE_STRING g_RegistryPath;

void PrintSysInfo();
void Unload(_In_ PDRIVER_OBJECT DriverObject);

extern "C"
NTSTATUS
DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	DriverObject->DriverUnload = Unload;
	KdPrint(("Driver Loaded!"));
	PrintSysInfo();

	// Save RegistryPath to g_RegistryPath
	g_RegistryPath.Buffer = (WCHAR*)ExAllocatePoolWithTag(PagedPool, RegistryPath->Length, DRIVER_TAG);
	if (g_RegistryPath.Buffer == nullptr) {
		KdPrint(("Failed to allocate memory\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	g_RegistryPath.MaximumLength = RegistryPath->Length;
	RtlCopyUnicodeString(&g_RegistryPath, (PUNICODE_STRING)RegistryPath);
	KdPrint(("Copied path: %wZ\n", &g_RegistryPath));

	return STATUS_SUCCESS;
}

void Unload(_In_ PDRIVER_OBJECT DriverObject) {
	UNREFERENCED_PARAMETER(DriverObject);
	ExFreePool(g_RegistryPath.Buffer);
	KdPrint(("Driver Unloaded"));
}

void PrintSysInfo() {
	OSVERSIONINFOW osInfo;
	if (NT_SUCCESS(RtlGetVersion(&osInfo))) {
		KdPrint(("System Info\n"));
		KdPrint(("Major: %ld", osInfo.dwMajorVersion));
		KdPrint(("Minor: %ld", osInfo.dwMinorVersion));
		KdPrint(("Build: %ld", osInfo.dwBuildNumber));
	}
}