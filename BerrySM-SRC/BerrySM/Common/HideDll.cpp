#include "stdafx.h"
#include "HideDll.h"
#include "ntdll.h"

typedef
NTSTATUS
(NTAPI
 *_NtQueryInformationProcess)(
	IN HANDLE			hProcess,
	IN PROCESSINFOCLASS	ProcessInformationClass,
	OUT PVOID			pProcessInformation,
	IN ULONG			uProcessInformationLength,
	OUT PULONG			puReturnLength OPTIONAL
	);

void HideDll(HANDLE hModule)
{
	_NtQueryInformationProcess NtQueryInformationProcess = (_NtQueryInformationProcess)
		GetProcAddress(GetModuleHandle("ntdll.dll"), "NtQueryInformationProcess");
	
	PROCESS_BASIC_INFORMATION processInfo = {0};
	
	DWORD dwReturnLength = 0;
	NtQueryInformationProcess(GetCurrentProcess(), ProcessBasicInformation, &processInfo, sizeof(processInfo), &dwReturnLength);

	PLIST_ENTRY ListHead, Current;
	
	PLDR_DATA_TABLE_ENTRY pstEntry = NULL;
	
	__try
	{
		ListHead = &( processInfo.PebBaseAddress->Ldr->InLoadOrderModuleList);
		Current = ListHead->Flink;
		while ( Current != ListHead)
		{
			pstEntry = CONTAINING_RECORD( Current, LDR_DATA_TABLE_ENTRY, InLoadOrderModuleList);
			if ( pstEntry->DllBase == hModule)
			{
				pstEntry->InLoadOrderModuleList.Flink->Blink = pstEntry->InLoadOrderModuleList.Blink;
				pstEntry->InLoadOrderModuleList.Blink->Flink = pstEntry->InLoadOrderModuleList.Flink;
				break;
			}
			Current = pstEntry->InLoadOrderModuleList.Flink;
		}
		
		ListHead = &( processInfo.PebBaseAddress->Ldr->InMemoryOrderModuleList);
		Current = ListHead->Flink;
		while ( Current != ListHead)
		{
			pstEntry = CONTAINING_RECORD( Current, LDR_DATA_TABLE_ENTRY, InMemoryOrderModuleList);
			if ( pstEntry->DllBase == hModule)
			{
				pstEntry->InMemoryOrderModuleList.Flink->Blink = pstEntry->InMemoryOrderModuleList.Blink;
				pstEntry->InMemoryOrderModuleList.Blink->Flink = pstEntry->InMemoryOrderModuleList.Flink;
				break;
			}
			Current = pstEntry->InMemoryOrderModuleList.Flink;
		}
		
		ListHead = &( processInfo.PebBaseAddress->Ldr->InInitializationOrderModuleList);
		Current = ListHead->Flink;
		while ( Current != ListHead)
		{
			pstEntry = CONTAINING_RECORD( Current, LDR_DATA_TABLE_ENTRY, InInitializationOrderModuleList);
			if ( pstEntry->DllBase == hModule)
			{
				pstEntry->InInitializationOrderModuleList.Flink->Blink = pstEntry->InInitializationOrderModuleList.Blink;
				pstEntry->InInitializationOrderModuleList.Blink->Flink = pstEntry->InInitializationOrderModuleList.Flink;
				break;
			}
			Current = pstEntry->InInitializationOrderModuleList.Flink;
		}
	}
	__except(1)	
	{
	}
}
