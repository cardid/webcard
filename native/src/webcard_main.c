/**
 * @file "native/src/webcard_main.c"
 * WebCard Native App :: Entry Point
 */

#include "smart_cards/smart_cards.h"

#if defined(_WIN32)
  #include <stdio.h>
  #include <io.h>
  #include <fcntl.h>
#endif

/**************************************************************/

BOOL
validateInputOutputPipes(void)
{
  os_specific_stream_t stdin_stream;
  os_specific_stream_t stdout_stream;

  #if defined(_WIN32)
  {
    _setmode(_fileno(stdin),  _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);

    stdin_stream = GetStdHandle(STD_INPUT_HANDLE);
    stdout_stream = GetStdHandle(STD_OUTPUT_HANDLE);
  }
  #elif defined(__linux__) || defined(__APPLE__)
  {
    stdin_stream = STDIN_FILENO;
    stdout_stream = STDOUT_FILENO;
  }
  #endif

  return OSSpecific_validateTypesOfStreams(
    stdin_stream,
    stdout_stream);
}

/**************************************************************/

int
main(void)
{
  if (!validateInputOutputPipes())
  {
    return EXIT_FAILURE;
  }

  #if defined(_DEBUG)
  {
    OSSpecific_writeDebugMessage(
      "Starting {WebCard Native App}");

    volatile BOOL debuggerAttached = FALSE;

    #if defined(_WIN32)
    {
      MessageBoxW(
        NULL,
        L"WebCard Debugging Session started...\n\n" \
          L"Attach a debugger of your choice NOW! :)",
        L"{ WebCard Native App Debugging }",
        MB_ICONINFORMATION);
    }
    #elif defined (__linux__)
    {
      system(
        "notify-send" \
        " -i \"dialog-information\"" \
        " -t 5000" \
        " \"{ WebCard Native App Debugging }\"" \
        " \"WebCard Debugging Session started...\\n\\n" \
          "Attach a debugger (gdb) NOW! :)\"");
    }
    #elif defined (__APPLE__)
    {
      system(
        "osascript -e 'display notification" \
        " \"WebCard Debugging Session started...\\n\\n" \
          "Attach a debugger (lldb) NOW! :)\"" \
        " with title \"{ WebCard Native App Debugging }\"'");
    }
    #endif

    while (!debuggerAttached)
    {}
  }
  #endif

  WebCard_run();

  #if defined(_DEBUG)
  {
    OSSpecific_writeDebugMessage(
      "{ WebCard Native App } shutting down");
  }
  #endif

  return EXIT_SUCCESS;
}

/**************************************************************/
