#pragma once

// `-console_stdin`: runs every line read from standard input as a console command (results go to standard output through the
// logger). For tools that drive a game process and for scripted sessions. Polled once per frame on the main thread; no thread.
namespace ConsoleStdin
{
    void InitFromArguments();   // reads the launch arguments; no-op without -console_stdin
    void Poll();                // once per frame, before the UI update
}
