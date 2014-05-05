#include "stdafx.h"
#include "xbf_parser.h"
#include "xbf_dumper.h"

enum dump_options {
	dumpNone = 0x0,
	dumpHeader = 0x1,
	dumpTables = 0x2,
	dumpNodes = 0x4,
	dumpNodesRaw = 0x8,
	dumpXaml = 0x10,
	dumpHelp = 0x80000000,
};

const struct { const wchar_t* arg; const wchar_t* arg_alt; const wchar_t* description; const int options; } cmd_options[] = {
    { L"?",     L"help",        L"show this help",      dumpHelp },
    { L"all",   nullptr,        L"dump all",            0xffffffff & (~dumpHelp) },
    { L"h",     L"header",      L"dump header",         dumpHeader },
    { L"t",     L"tables",      L"dump tables",         dumpTables },
    { L"n",     L"nodes",       L"dump nodes",          dumpNodes },
//     { L"nr",    L"nodesraw",    L"dump nodes raw data", dumpNodesRaw },
    { L"x",     L"xaml",        L"dump nodes as XAML",  dumpXaml },
};

void print_usage() {
    printf_s("\tUsage: XbfDump <xbffile> [options]\n\n");
    for (auto o = std::begin(cmd_options); o != std::end(cmd_options); ++o) {
        if (o->arg != nullptr) printf_s("\t-%S", o->arg); else printf_s("\t");

        int len = 0;
        if (o->arg_alt != nullptr) {
            len = wcslen(o->arg_alt);
            printf_s("\t--%S", o->arg_alt);
        } else printf_s("\t");

        if (len < 6) printf_s("\t");
        if (len < 14) printf_s("\t");
        printf_s("\t: %S\n", o->description);
    }
}

int wmain(int argc, wchar_t* argv[])
{
    int options = dumpNone;
    const wchar_t* xbffile = nullptr;
    const wchar_t* err_arg = nullptr;

    printf_s("\nXbfDump v0.1 https://github.com/WalkingCat/XbfDump\n\n");

    for (int i = 1; i < argc; i++) {
        const wchar_t* arg = argv[i];
        if ((arg[0] == '-') || ((arg[0] == '/'))) {
            bool valid = false;
            if ((arg[0] == '-') && (arg[1] == '-')) {
                for (auto o = std::begin(cmd_options); o != std::end(cmd_options); ++o) {
                    if ((o->arg_alt != nullptr) && (_wcsicmp(arg + 2, o->arg_alt) == 0)) { valid = true; options |= o->options; }
                }
            } else {
                for (auto o = std::begin(cmd_options); o != std::end(cmd_options); ++o) {
                    if ((o->arg != nullptr) && (_wcsicmp(arg + 1, o->arg) == 0)) { valid = true; options |= o->options; }
                }
            }
            if ((!valid) && (err_arg == nullptr)) err_arg = arg;
        } else { if (xbffile == nullptr) xbffile = arg; else err_arg = arg; }
    }

    if ((xbffile == nullptr) || (err_arg != nullptr) || (options & dumpHelp)) {
        if (err_arg != nullptr) printf_s("\tUnknown option: %S\n\n", err_arg);
        print_usage();
        return 0;
    }

    printf_s("Dumping XBF file: %S\n\n", xbffile);

    if (options == dumpNone) options = dumpXaml;

	easy_file file(xbffile, L"rb");
	if (file.valid()) {
		xbf_parser parser(std::move(file));
		if (parser.parse()) {
			xbf_dumper dumper(parser.get_data());

            if (options & dumpHeader)   dumper.dump_header();
            if (options & dumpTables)   dumper.dump_tables();
            if (options & dumpNodes)    dumper.dump_nodes();
            if (options & dumpXaml)     dumper.dump_xaml();
		} else printf_s(" parsing error\n");
	}

	return 0;
}

