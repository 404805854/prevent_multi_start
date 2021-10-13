#include <errno.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#if defined(_WIN32)
#include <windows.h>
#endif

#if defined(__linux__)
typedef int multi_handle;
#define multi_handle_close(handle) close(handle)
#elif defined(_WIN32)
typedef HANDLE multi_handle;
#define multi_handle_close(handle) CloseHandle(handle)
#endif

bool prevent_multi_start(multi_handle *handle, std::string &lock_name) {
  handle = NULL;
#if defined(__linux__)
  if (ret.find("/") == ret.npos) {
    lock_name = "/tmp/" + lock_name + ".lock";
  }
  int lock_file = open(lock_name.c_str(), O_CREAT | O_RDWR, 0666);
  int rc = flock(lock_file, LOCK_EX | LOCK_NB);
  if (rc) {
    if (EWOULDBLOCK == errno) {
      return true;
    }
  } else {
    char buffer[64] = {0};
    sprintf(buffer, "pid:%d\n", getpid());
    write(lock_file, buffer, strlen(buffer));
    handle = &lock_file;
    return false;
  }
#elif defined(_WIN32)
  HANDLE m_hMutex = CreateMutex(NULL, TRUE, lock_name.c_str());
  DWORD dwRet = GetLastError();
  if (m_hMutex) {
    if (ERROR_ALREADY_EXISTS == dwRet) {
      CloseHandle(m_hMutex);
      return true;
    }
  } else {
    CloseHandle(m_hMutex);
    return true;
  }
  handle = &m_hMutex;
  return false;
#endif
}

int main() {
  multi_handle handle;
  std::string lock_name = "single_proc";
  if (prevent_multi_start(&handle, lock_name)) {
    printf("program is running\n");
  } else {
    printf("start , wait input : ");
    std::string input;
    std::cin >> input;
    printf("exit\n");
    multi_handle_close(handle);
  }
  return 0;
}