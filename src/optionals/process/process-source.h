#define DICTU_PROCESS_SOURCE "import Process;\n" \
"\n" \
"def signalHandle(sig, handler) {\n" \
"    Process.signalHandlers[sig] = handler;\n" \
"}\n" \
"\n" \
"def watchForSignals() {\n" \
"    while (Process.currentSignal != 0) {\n" \
"        if (Process.signalHandlers[Process.currentSignal] != nil) {\n" \
"            Process.signalHandlers[Process.currentSignal]();\n" \
"        }\n" \
"    }\n" \
"}\n" \

