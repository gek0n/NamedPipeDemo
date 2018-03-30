#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include <AccCtrl.h>
#include <Aclapi.h>

#define BUFFSIZE (512)

int main(int argc, char *argv[])
{
  HANDLE pipe;

  SID_IDENTIFIER_AUTHORITY SIDAuthWorld;
  PSID pEveryoneSID;
  if (!AllocateAndInitializeSid(&SIDAuthWorld, 1,
    SECURITY_WORLD_RID,
    0, 0, 0, 0, 0, 0, 0,
    &pEveryoneSID)) {
    printf("AllocateAndInitializeSid Error %#X\n", GetLastError());
    return FALSE;
  }

  EXPLICIT_ACCESS_A ea;
  ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
  ea.grfAccessPermissions = FILE_READ_DATA | FILE_WRITE_DATA;;
  ea.grfAccessMode = SET_ACCESS;
  ea.grfInheritance = NO_INHERITANCE;
  ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
  ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
  ea.Trustee.ptstrName = (LPSTR)pEveryoneSID;

  PACL pACL;
  DWORD dwRes;
  dwRes = SetEntriesInAclA(2, &ea, NULL, &pACL);

  PSECURITY_DESCRIPTOR pSD;
  pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
  if (NULL == pSD) {
    printf("LocalAlloc Error %#X\n", GetLastError());
  }

  if (!InitializeSecurityDescriptor(pSD,
    SECURITY_DESCRIPTOR_REVISION)) {
    printf("InitializeSecurityDescriptor Error %#X\n", GetLastError());
  }

  if (!SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE))
  {
    printf("SetSecurityDescriptorDacl Error %#X\n", GetLastError());
  }

  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof(SECURITY_ATTRIBUTES);
  sa.lpSecurityDescriptor = pSD;
  sa.bInheritHandle = FALSE;

  printf("[>] Creating named pipe...\n");
  pipe = CreateNamedPipeA(
           "\\\\.\\pipe\\g3k0nPipe",
           PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
           PIPE_TYPE_MESSAGE |
           PIPE_READMODE_MESSAGE |
           PIPE_WAIT,
           PIPE_UNLIMITED_INSTANCES,
           BUFFSIZE,
           BUFFSIZE,
           0,
           &sa);

  if (pipe == INVALID_HANDLE_VALUE)
  {
    printf("[-] Failure\n");
    return -1;
  }
  else {
    printf("[+] Success\n");
  }

  printf("[+] Created\n");

  printf("[>] Waiting connection to pipe...\n");

  const uint32_t connected = ConnectNamedPipe(pipe, NULL);
  if (!connected) {
    CloseHandle(pipe);
    printf("[-] Failed to connect to named pipe\n");
    return -2;
  }

  printf("[+] Connected\n");

  const uint32_t bufSz = BUFFSIZE;
  char *buf = (char *)malloc(bufSz);
  uint32_t success;
  uint32_t read = 0;

  success = ReadFile(
              pipe,
              buf,
              bufSz,
              &read,
              NULL);

  if (!success || read == 0) {
    printf("[-] ReadFile failed with %#X\n", GetLastError());
    return -3;
  }

  printf("[+] Read from pipe [%d]: %s\n", read, buf);

  uint32_t written = 0;

  success = WriteFile(
              pipe,
              buf,
              read,
              &written,
              NULL);

  if (!success || written != read) {
    printf("[-] WriteFile failed with %d\n", GetLastError());
    return -4;
  }

  printf("[+] Write to pipe [%d]: %s\n", written, buf);

  CloseHandle(pipe);
  printf("[+] Close named pipe\n");

  return 0;
}
