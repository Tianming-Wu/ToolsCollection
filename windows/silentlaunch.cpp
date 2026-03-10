#include <windows.h>
#include <string>
#include <SharedCppLib2/stringlist.hpp>
#include <iostream>

int posterr(std::string msg, int ret = 0) {
	std::cerr << "Error: " << msg << std::endl;
	return ret;
}

int main(int argc, char** argv) {
	std::stringlist args(argc, argv);

	STARTUPINFOA si = {sizeof(si)};
	PROCESS_INFORMATION pi;

	std::string cmdLine, progName;
	bool useprog = false;

	if(args.size() == 1) {
		std::cout << "Usage:\n\tsilentlaunch [command]\n\tsilentlaunch -p [program] -c [command]" << std::endl;
		return 0;
	} else if (args.size() == 2) {
		cmdLine = args[1];
	} else {
		size_t pp = 1;
		while(pp < args.size()) {
			const std::string &arg = args[pp];
			if(arg == "-c") {
				if(args.size() <= pp+1) {
					if(!cmdLine.empty()) return posterr("Duplicated -c option.", 1);
					cmdLine = args[pp+1];
					pp += 2;
				} else return posterr("option -c must have a parameter", 1);
			} else if (arg == "-p") {
				if(args.size() <= pp+1) {
					if(useprog) return posterr("Duplicated -p option.", 1);
					useprog = true;
					progName = args[pp+1];
					pp += 2;
				} else return posterr("option -p must have a parameter", 1);
			} else return posterr("Unknown option/argument: " + arg, 1);
		}
	}

	bool success = CreateProcessA( useprog?progName.data():NULL, cmdLine.data(), NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL, &si, &pi);

	return success? 0 : GetLastError();
}
