#include <stdio.h>
#include <stdint.h>
#include <windows.h>

#define BUFFSIZE (512)

uint32_t main(uint32_t argc, char *argv[])
{
  UNREFERENCED_PARAMETER(argc);
  UNREFERENCED_PARAMETER(argv);

  printf("[>] Try to open named pipe...\n");

  HANDLE pipe;
  pipe = CreateFileW(
             L"\\\\.\\pipe\\g3k0nPipe",
             GENERIC_READ | GENERIC_WRITE,
             0,
             NULL,
             OPEN_EXISTING,
             0,
             NULL);

  if (pipe == INVALID_HANDLE_VALUE) {
    printf("[-] Failed to open pipe with error: %#X\n", GetLastError());
    return -1;
  }

  printf("[+] Opened\n");

  char *buf = (char *)malloc(BUFFSIZE);
  const uint32_t bufSz = snprintf(buf, BUFFSIZE, "%s", "Testing named pipe: g3k0nPipe") + 1;
  uint32_t written, read;

  uint32_t success = WriteFile(
                       pipe,
                       buf,
                       bufSz,
                       &written,
                       NULL);

  if (!success) {
    printf("[-] Failed write to pipe");
    return -2;
  }
  printf("[+] Write to pipe [%d]: %s\n", written, buf);

  
  success = ReadFile(
              pipe,
              buf,
              BUFFSIZE,
              &read,
              NULL);

  if (!success) {
    printf("[-] Failed read from pipe");
    return -2;
  }
  printf("[+] Read from pipe [%d]: %s\n", read, buf);

  CloseHandle(pipe);
  printf("[+] Close named pipe\n");

  return 0;
}
