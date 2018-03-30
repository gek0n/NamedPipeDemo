#include <stdio.h>
#include <stdint.h>
#include <windows.h>

#define BUFFSIZE (512)

uint32_t main(uint32_t argc, char *argv[])
{
  UNREFERENCED_PARAMETER(argc);
  UNREFERENCED_PARAMETER(argv);

  printf("[>] Creating named pipe...\n");

  HANDLE pipe;
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
             NULL);

  if (pipe == INVALID_HANDLE_VALUE) {
    printf("[-] Failed to create named pipe with error: %#X\n", GetLastError());
    return -1;
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
