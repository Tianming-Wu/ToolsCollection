#include <Windows.h>
#include <wchar.h>
#include <Psapi.h>

#include <string>
#include <iostream>

#include <SharedCppLib2/ansiio.hpp>
#include <SharedCppLib2/logt.hpp>

int main(int argc, char** argv)
{
    HWND last_fore = NULL;
    wchar_t name[MAX_PATH];
    DWORD size = MAX_PATH;

    std::wstring last_name;
    int count = 0;

    logt::stdcout(true);
    logt::claim("Main");
    logt::enableSuperTimestamp(true);

    if(argc > 1 && std::string(argv[1]) == "--log") {
        std::string logfile = (argc > 2) ? argv[2] : "whoareyou.log";
        logt::addfile(logfile, true);
    }

    LOGT_LOCAL("Main");

    while(true)
    {
        HWND fore;
        if ((fore = GetForegroundWindow()) != last_fore)
        {
            last_fore = fore;
            DWORD processid;
            GetWindowThreadProcessId(fore, &processid);
            HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION, NULL, processid);
            GetProcessImageFileNameW(process, name, size);

            std::wstring full_name(name);

            if(full_name != last_name)
            {
                logt.info() << "Process: " << full_name;
                last_name = full_name;
                count = 0;
            } else {
                logt.info() << "Process: " << full_name << "(" << count << ")";
            }
        }

        Sleep(10);
    }
    
    logt::shutdown();
    return 0;
}