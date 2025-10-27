#include <windows.h>
#include <stdio.h>
#include <string.h>

#define MAX_LINE 80 // Maximum length of a command

/**
 * @brief Executes the given command line string using CreateProcess.
 * @param command_line The complete command line string to execute.
 */
void execute_command_windows(char *command_line) {
    STARTUPINFOA si; // Note: Using 'A' for ANSI version
    PROCESS_INFORMATION pi;

    // Zero out the structures
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si); // Set the size of the structure
    ZeroMemory(&pi, sizeof(pi));

    // Create the child process
    if (!CreateProcessA(
            NULL,           // No module name (use command line)
            command_line,   // The command line to execute
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            FALSE,          // Set handle inheritance to FALSE
            0,              // No creation flags
            NULL,           // Use parent's environment block
            NULL,           // Use parent's starting directory
            &si,            // Pointer to STARTUPINFO structure
            &pi             // Pointer to PROCESS_INFORMATION structure
        )) {
        // --- Error Handling ---
        // If CreateProcess fails
        DWORD error_code = GetLastError();
        char error_msg[256];
        FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            error_code,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            error_msg,
            sizeof(error_msg),
            NULL
        );
        printf("CreateProcess failed (Error %lu): %s", error_code, error_msg);
        return;
    }

    // --- Parent Process ---
    
    // 1. Wait for the child process to finish
    // This is the Windows equivalent of waitpid()
    WaitForSingleObject(pi.hProcess, INFINITE);

    // 2. Close the process and thread handles
    // This is important cleanup to avoid resource leaks
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

int main() {
    char line[MAX_LINE];  // Buffer to hold the raw input line
    int should_run = 1;   // Flag to control the main loop

    printf("Welcome to SimpleShell (Windows). Type 'exit' to quit.\n");

    while (should_run) {
        printf("> ");
        fflush(stdout); // Ensure the prompt is displayed immediately

        // Read a line of input
        if (fgets(line, MAX_LINE, stdin) == NULL) {
            // Handle EOF (e.g., Ctrl+Z)
            printf("\nExiting shell.\n");
            break;
        }
        
        // If the user just presses Enter, line[0] will be '\n'
        if (line[0] == '\n') {
            continue; // Skip and show the prompt again
        }

        // Remove the trailing newline character from fgets
        // strcspn finds the first occurrence of any character in "\n"
        line[strcspn(line, "\n")] = 0;

        // Check for empty input (e.g., only spaces)
        // Note: A more robust parser would be needed for complex cases,
        // but for simple commands, this works.
        if (line[0] == '\0') {
            continue;
        }

        // --- Handle Built-in Commands ---
        
        // Check for "exit" command
        // Note: We use strncmp to safely compare.
        if (strncmp(line, "exit", 4) == 0) {
            // Check if it's just "exit" and not "exit_something_else"
            if (line[4] == '\0' || line[4] == ' ') {
                 should_run = 0; // Set the flag to exit the loop
                 printf("Exiting shell.\n");
            }
        } 
        else {
            // If it's not a built-in, try to execute it as an external program
            execute_command_windows(line);
        }
    }

    return 0;
}
