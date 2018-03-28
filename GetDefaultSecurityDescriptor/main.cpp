#include <windows.h>
#include <sddl.h> // ConvertSecurityDescriptorToStringSecurityDescriptor

int WINAPI
WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  HANDLE Token;
  if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &Token)) {
    DWORD RequiredSize = 0;
    GetTokenInformation(Token, TokenDefaultDacl, NULL, 0, &RequiredSize);
    TOKEN_DEFAULT_DACL* DefaultDacl =
      reinterpret_cast<TOKEN_DEFAULT_DACL*>(LocalAlloc(LPTR, RequiredSize));
    if (DefaultDacl) {
      SECURITY_DESCRIPTOR Sd;
      LPTSTR StringSd;
      if (GetTokenInformation(Token, TokenDefaultDacl, DefaultDacl,
        RequiredSize, &RequiredSize) &&
        InitializeSecurityDescriptor(&Sd, SECURITY_DESCRIPTOR_REVISION) &&
        SetSecurityDescriptorDacl(&Sd, TRUE,
          DefaultDacl->DefaultDacl, FALSE) &&
        ConvertSecurityDescriptorToStringSecurityDescriptor(&Sd,
          SDDL_REVISION_1, DACL_SECURITY_INFORMATION, &StringSd, NULL)) {
        MessageBox(NULL, StringSd, TEXT("Result"), MB_OK);
        LocalFree(StringSd);
      }
      LocalFree(DefaultDacl);
    }
    CloseHandle(Token);
  }
  return 0;
}