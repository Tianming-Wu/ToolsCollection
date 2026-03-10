#define UNICODE
#define _UNICODE

#include <windows.h>

int wmain() {
    MessageBox(NULL, L"点击确定跳过上午", L"趴下5分钟后自动确定", MB_OK | MB_ICONINFORMATION);
    return 0;
}