// bcrypt stub to allow newer versions of MakeMKV to run on Windows XP.
// Public domain, 2020 December, Michael Calkins ("qbasicmichael"). No warranty. Use at your own risk.
// Revision 2021 January 01
// Borrows from the "w64 mingw-runtime package", which is also public domain.

// mingw: ----------------------------------- (for older mingw, remove ",--nxcompat".)
// g++.exe -O3 -Wall -shared -Wl,-s,-shared,--enable-auto-image-base,-dy,--nxcompat,--kill-at -o bcrypt.dll bcrypt.cpp

// msvc++:
// cl.exe /O2 /LD /MD bcrypt.cpp bcryptvc.def user32.lib /link /NXCOMPAT
// mt.exe -manifest bcrypt.dll.manifest -outputresource:bcrypt.dll;2

// bcryptvc.def should contain the following:
// LIBRARY bcrypt.dll
// EXPORTS
// BCryptOpenAlgorithmProvider
// BCryptCloseAlgorithmProvider
// BCryptGenRandom
// ThisDllIsJustAStub

// Place the resulting "bcrypt.dll" into the MakeMKV folder. Do not put it in the system32 folder.
// You should probably delete any resulting bcrypt.a or bcrypt.lib files afterwards, so as not to interfere with linking other projects to genuine bcrypt.

#define NOCRYPT
#include <windows.h>

typedef LONG NTSTATUS;
typedef LPVOID BCRYPT_ALG_HANDLE;
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)

extern "C" __declspec(dllexport) NTSTATUS WINAPI BCryptOpenAlgorithmProvider(BCRYPT_ALG_HANDLE *phAlgorithm, LPCWSTR pszAlgId, LPCWSTR pszImplementation, DWORD dwFlags) {
 MessageBoxW(NULL, pszAlgId, L"BCryptOpenAlgorithmProvider", 0);
 return STATUS_SUCCESS;
}

extern "C" __declspec(dllexport) NTSTATUS WINAPI BCryptCloseAlgorithmProvider(BCRYPT_ALG_HANDLE hAlgorithm, ULONG dwFlags) {
 MessageBoxW(NULL, L"", L"BCryptCloseAlgorithmProvider", 0);
 return STATUS_SUCCESS;
}

extern "C" __declspec(dllexport) NTSTATUS WINAPI BCryptGenRandom(BCRYPT_ALG_HANDLE hAlgorithm, PUCHAR pbBuffer, ULONG cbBuffer, ULONG dwFlags) {
 MessageBoxW(NULL, L"", L"BCryptGenRandom", 0);
 return STATUS_SUCCESS;
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
 return TRUE;
}

extern "C" __declspec(dllexport) const char * WINAPI ThisDllIsJustAStub(void) {
 return "This bcrypt.dll is just a stub to get MakeMKV to work on WinXP.";
}