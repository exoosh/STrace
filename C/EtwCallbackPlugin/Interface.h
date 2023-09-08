#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <subauth.h>
#include <stdint.h>
#include <evntcons.h>
#include <type_traits>

class MachineState
{
public:
	uint32_t  paramCount;
	uint32_t  regArgsSize;
	uint64_t* pStackArgs;
	uint64_t* pRegArgs;

	uint64_t read_argument(const uint32_t idx)
	{
		if (idx > paramCount)
			return 0;

		if (idx >= regArgsSize) {
			// stack array trims off the register array data, and indexes from that offset value
			return pStackArgs[idx - regArgsSize];
		}
		else {
			return pRegArgs[idx];
		}
	}

	template<typename T>
	void write_argument(const uint32_t idx, T value) {
		if (idx > paramCount)
			return;

		if (idx >= regArgsSize) {
			// stack array trims off the register array data, and indexes from that offset value
			pStackArgs[idx - regArgsSize] = (uint64_t)value;
		}
		else {
			pRegArgs[idx] = (uint64_t)value;
		}
	}

	void redirect_syscall(uint64_t pFn) {
		// The syscall pointer is stored just after the args
		pRegArgs[4] = pFn;
	}

	uint64_t read_return_value() {
		return pRegArgs[0];
	}

	template<typename T>
	void write_return_value(T value) {
		pRegArgs[0] = (uint64_t)value;
	}
};

typedef LONG NTSTATUS;
typedef bool(*tSetTlsData)(uint64_t value, uint8_t slot);
typedef bool(*tGetTlsData)(uint64_t& value, uint8_t slot);
typedef NTSTATUS(*tLogPrintApi)(uint32_t Level, const char* FunctionName, const char* Format, ...);
typedef NTSTATUS(*tEtwTraceApi)(const char* providerName, const GUID* providerGuid, const char* eventName, uint8_t eventLevel, uint8_t eventChannel, uint64_t flag, int numberOfFields, ...);
typedef NTSTATUS(*tSetCallbackApi)(const char* syscallName, ULONG64 probeId);
typedef NTSTATUS(*tUnSetCallbackApi)(const char* syscallName);
typedef NTSTATUS(*tSetEtwCallbackApi)(GUID providerGuid);
typedef NTSTATUS(*tUnSetEtwCallbackApi)();
typedef PVOID(NTAPI* tMmGetSystemRoutineAddress)(PUNICODE_STRING SystemRoutineName);
typedef BOOLEAN(*tTraceAccessMemory)(PVOID SafeAddress, ULONG_PTR UnsafeAddress, SIZE_T NumberOfBytes, SIZE_T ChunkSize, BOOLEAN DoRead);

class PluginApis {
public:
	PluginApis() = default;

	tSetTlsData pSetTlsData;
	tGetTlsData pGetTlsData;
	tLogPrintApi pLogPrint;
	tEtwTraceApi pEtwTrace;
	tSetCallbackApi pSetCallback;
	tUnSetCallbackApi pUnsetCallback;
	tSetEtwCallbackApi pEtwSetCallback;
	tUnSetEtwCallbackApi pEtwUnSetCallback;
	tMmGetSystemRoutineAddress pGetSystemRoutineAddress;
	tTraceAccessMemory pTraceAccessMemory;
};

#define MINCHAR     0x80        // winnt
#define MAXCHAR     0x7f        // winnt
#define MINSHORT    0x8000      // winnt
#define MAXSHORT    0x7fff      // winnt
#define MINLONG     0x80000000  // winnt
#define MAXLONG     0x7fffffff  // winnt
#define MAXUCHAR    0xff        // winnt
#define MAXUSHORT   0xffff      // winnt
#define MAXULONG    0xffffffff  // winnt

UNICODE_STRING WideToUnicodeString(PCWSTR SourceString);

template<typename T>
T ResolveApi(const wchar_t* name, PluginApis& apis) {
	auto ustr = WideToUnicodeString(name);
	return (T)apis.pGetSystemRoutineAddress(&ustr);
}

typedef void(*tStpInitialize)(PluginApis& pApis);
typedef void(*tStpDeInitialize)();
typedef void(*tDtEtwpEventCallback)(EVENT_HEADER* EventHeader, ULONG32 a, GUID* ProviderGuid, ULONG32 b);

typedef enum _LOG_LEVEL_OPTIONS
{
	LogLevelDebug = 0x10ul,
	LogLevelInfo = 0x20ul,
	LogLevelWarn = 0x40ul,
	LogLevelError = 0x80ul,
} LOG_LEVEL_OPTIONS;

// ETW field type definitions, see TlgIn_t and TlgOut_t in TraceLoggingProvider.h
#define ETW_FIELD(in, out)  (in | 0x80 | out << 8)

typedef enum _ETW_IN_FIELD_TYPE
{
	EtwInNull,
	EtwInUnicodeString,
	EtwInAnsiString,
	EtwInInt8,
	EtwInUInt8,
	EtwInInt16,
	EtwInUInt16,
	EtwInInt32,
	EtwInUInt32,
	EtwInInt64,
	EtwInUInt64,
	EtwInFloat,
	EtwInDouble,
	EtwInBool32,
	EtwInBinary,
	EtwInGuid,
	EtwInPointer,
	EtwInFiletime,
	EtwInSystemTime,
	EtwInSid,
	EtwInHexInt32,
	EtwInHexInt64,
	EtwInCountedString,
	EtwInCountedAnsiString,
} ETW_IN_FIELD_TYPE;

typedef enum _ETW_OUT_FIELD_TYPE
{
	EtwOutNull,
	EtwOutNoPrint,
	EtwOutString,
	EtwOutBoolean,
	EtwOutHex,
	EtwOutPid,
	EtwOutTid,
	EtwOutPort,
	EtwOutIpV4,
	EtwOutIpV6,
	EtwOutSocketAddress,
	EtwOutXml,
	EtwOutJson,
	EtwOutWin32Error,
	EtwOutNtstatus,
	EtwOutHresult,
	EtwOutFiletime,
	EtwOutSigned,
	EtwOutUnsigned,
} ETW_OUT_FIELD_TYPE;

typedef enum _ETW_FIELD_TYPE
{
	EtwFieldInt8 = EtwInInt8,
	EtwFieldUInt8 = EtwInUInt8,
	EtwFieldInt16 = EtwInInt16,
	EtwFieldUInt16 = EtwInUInt16,
	EtwFieldInt32 = EtwInInt32,
	EtwFieldUInt32 = EtwInUInt32,
	EtwFieldInt64 = EtwInInt64,
	EtwFieldUInt64 = EtwInUInt64,
	EtwFieldFloat32 = EtwInFloat,
	EtwFieldFloat64 = EtwInDouble,
	EtwFieldBool = EtwInBool32,
	EtwFieldGuid = EtwInGuid,
	EtwFieldPointer = EtwInPointer,
	EtwFieldFiletime = EtwInFiletime,
	EtwFieldSystemTime = EtwInSystemTime,
	EtwFieldHexInt8 = ETW_FIELD(EtwInUInt8, EtwOutHex),
	EtwFieldHexUInt8 = ETW_FIELD(EtwInUInt8, EtwOutHex),
	EtwFieldHexInt32 = EtwInHexInt32,
	EtwFieldHexUInt32 = EtwInHexInt32,
	EtwFieldHexInt64 = EtwInHexInt64,
	EtwFieldHexUInt64 = EtwInHexInt64,
	EtwFieldWChar = ETW_FIELD(EtwInUInt16, EtwOutString),
	EtwFieldChar = ETW_FIELD(EtwInUInt8, EtwOutString),
	EtwFieldBoolean = ETW_FIELD(EtwInUInt8, EtwOutBoolean),
	EtwFieldHexInt16 = ETW_FIELD(EtwInUInt16, EtwOutHex),
	EtwFieldHexUInt16 = ETW_FIELD(EtwInUInt16, EtwOutHex),
	EtwFieldPid = ETW_FIELD(EtwInUInt32, EtwOutPid),
	EtwFieldTid = ETW_FIELD(EtwInUInt32, EtwOutTid),
	EtwFieldPort = ETW_FIELD(EtwInUInt16, EtwOutPort),
	EtwFieldWinError = ETW_FIELD(EtwInUInt32, EtwOutWin32Error),
	EtwFieldNtstatus = ETW_FIELD(EtwInUInt32, EtwOutNtstatus),
	EtwFieldHresult = ETW_FIELD(EtwInInt32, EtwOutHresult),
	EtwFieldString = EtwInAnsiString,
	EtwFieldWideString = EtwInUnicodeString,
	EtwFieldCountedString = EtwInCountedAnsiString,
	EtwFieldCountedWideString = EtwFieldCountedString,
	EtwFieldAnsiString = EtwInCountedAnsiString,
	EtwFieldUnicodeString = EtwInCountedString,
	EtwFieldBinary = EtwInBinary,
	EtwFieldSocketAddress = ETW_FIELD(EtwInBinary, EtwOutSocketAddress),
	EtwFieldSid = EtwInSid,
} ETW_FIELD_TYPE;

// Assert a function is the same type as a function pointer typedef, or throw msg as a compiler error
#define ASSERT_INTERFACE_IMPLEMENTED(Implementer, tFnTypeDef, msg) static_assert(std::is_same_v<decltype(&Implementer), tFnTypeDef>, msg); 