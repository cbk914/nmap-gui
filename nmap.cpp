#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_BUFFER_SIZE 1024

// Global variables
HWND hwndInput;
HWND hwndOutput;
HWND hwndButton;
HWND hwndCheckBox;
HWND hwndFileOutput;
HWND hwndRadioTxt;
HWND hwndRadioXml;
HWND hwndRadioHtml;
char nmapPath[MAX_PATH];
char nmapOptions[MAX_BUFFER_SIZE];
char outputBuffer[MAX_BUFFER_SIZE];

// Function prototypes
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL RunNmap();
void AddTextToOutput(char* text);
void ShowErrorMessage(char* message);

// Main function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Create the main window
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "MainWindowClass";
    RegisterClass(&wc);

    HWND hwnd = CreateWindow("MainWindowClass", "Nmap GUI",
        WS_OVERLAPPEDWINDOW, 50, 50, 700, 500, NULL, NULL, hInstance, NULL);
    if (!hwnd) {
        ShowErrorMessage("Failed to create window");
        return 1;
    }

    // Create the input edit control
    hwndInput = CreateWindowEx(0, "EDIT", NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
        10, 10, 600, 200, hwnd, NULL, hInstance, NULL);
    if (!hwndInput) {
        ShowErrorMessage("Failed to create input control");
        return 1;
    }

    // Create the output edit control
    hwndOutput = CreateWindowEx(0, "EDIT", NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        10, 220, 600, 200, hwnd, NULL, hInstance, NULL);
    if (!hwndOutput) {
        ShowErrorMessage("Failed to create output control");
        return 1;
    }

    // Create the "Run Nmap" button
    hwndButton = CreateWindow("BUTTON", "Run Nmap", WS_VISIBLE | WS_CHILD,
        10, 430, 100, 30, hwnd, NULL, hInstance, NULL);
    if (!hwndButton) {
        ShowErrorMessage("Failed to create button control");
        return 1;
    }

    // Create the "Use Vulners" checkbox
    hwndCheckBox = CreateWindow("BUTTON", "Use Vulners", WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
        140, 435, 100, 20, hwnd, NULL, hInstance, NULL);
    if (!hwndCheckBox) {
        ShowErrorMessage("Failed to create checkbox control");
        return 1;
    }

        // Create the "Output to file" checkbox
    HWND hwndCheckFileOutput = CreateWindow("BUTTON", "Output to file", WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
        260, 435, 100, 20, hwnd, NULL, hInstance, NULL);
    if (!hwndCheckFileOutput) {
        ShowErrorMessage("Failed to create checkbox control");
        return 1;
    }

    // Create the "TXT" radio button
    hwndRadioTxt = CreateWindow("BUTTON", "TXT", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP,
        380, 435, 40, 20, hwnd, NULL, hInstance, NULL);
    if (!hwndRadioTxt) {
        ShowErrorMessage("Failed to create radio button control");
        return 1;
    }

    // Create the "XML" radio button
    hwndRadioXml = CreateWindow("BUTTON", "XML", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
        430, 435, 40, 20, hwnd, NULL, hInstance, NULL);
    if (!hwndRadioXml) {
        ShowErrorMessage("Failed to create radio button control");
        return 1;
    }

    // Create the "HTML" radio button
    hwndRadioHtml = CreateWindow("BUTTON", "HTML", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
        480, 435, 50, 20, hwnd, NULL, hInstance, NULL);
    if (!hwndRadioHtml) {
        ShowErrorMessage("Failed to create radio button control");
        return 1;
    }

    // Create the "File output" edit control
    hwndFileOutput = CreateWindowEx(0, "EDIT", NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        540, 435, 200, 20, hwnd, NULL, hInstance, NULL);
    if (!hwndFileOutput) {
        ShowErrorMessage("Failed to create file output control");
        return 1;
    }

    // Get the path to Nmap
    if (!SearchPath(NULL, "nmap.exe", NULL, MAX_PATH, nmapPath, NULL)) {
        ShowErrorMessage("Could not find Nmap on your system. Please install Nmap and try again.");
        return 1;
    }

    // Run the message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND:
        if ((HWND)lParam == hwndButton && HIWORD(wParam) == BN_CLICKED) {
            // Get the Nmap options from the input control
            int inputLength = GetWindowTextLength(hwndInput);
            if (inputLength >= MAX_BUFFER_SIZE) {
                ShowErrorMessage("Input is too long.");
                return 0;
            }
            GetWindowText(hwndInput, nmapOptions, MAX_BUFFER_SIZE);

            // Add the "-oX" or "-oN" option based on the selected output format
            if (IsDlgButtonChecked(hwnd, hwndRadioXml)) {
                strcat_s(nmapOptions, " -oX ");
            }
            else if (IsDlgButtonChecked(hwnd, hwndRadioTxt)) {
                strcat_s(nmapOptions, " -oN ");
            }
            else if (IsDlgButtonChecked(hwnd, hwndRadioHtml))
            {
                strcat_s(nmapOptions, " -oX ");
            }

            // If "Output to file" is checked, add the output file path to the options
            if (IsDlgButtonChecked(hwnd, hwndCheckFileOutput)) {
                char outputFilePath[MAX_PATH];
                GetWindowText(hwndFileOutput, outputFilePath, MAX_PATH);

                // Make sure the output file path is not too long
                if (strlen(outputFilePath) >= MAX_PATH) {
                    ShowErrorMessage("Output file path is too long.");
                    return 0;
                }

                strcat_s(nmapOptions, "\"");
                strcat_s(nmapOptions, outputFilePath);
                strcat_s(nmapOptions, "\"");
            }

            // Add the "--script vulners" option if "Use Vulners" is checked
            if (IsDlgButtonChecked(hwnd, hwndCheckBox)) {
                strcat_s(nmapOptions, " --script vulners ");
            }

            // Disable the input control and "Run Nmap" button
            EnableWindow(hwndInput, FALSE);
            EnableWindow(hwndButton, FALSE);

            // Run Nmap on a separate thread
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunNmap, NULL, 0, NULL);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

// Runs Nmap and writes the output to the output edit control
BOOL RunNmap() {
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
    HANDLE hChildStd_OUT_Rd = NULL;
    HANDLE hChildStd_OUT_Wr = NULL;
    if (!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &sa, 0)) {
        ShowErrorMessage("Failed to create pipe for Nmap output.");
        return FALSE;
    }

    // Set the console mode to enable ANSI escape codes
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdout == INVALID_HANDLE_VALUE) {
        ShowErrorMessage("Failed to get standard output handle.");
        return FALSE;
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(hStdout, &dwMode)) {
        ShowErrorMessage("Failed to get console mode.");
        return FALSE;
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hStdout, dwMode)) {
        ShowErrorMessage("Failed to set console mode.");
        return FALSE;
    }

    // Create the Nmap process
    STARTUPINFO si = { sizeof(si) };
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    si.hStdError = hChildStd_OUT_Wr;
    si.hStdOutput = hChildStd_OUT_Wr;

    PROCESS_INFORMATION pi = {};
    if (!CreateProcess(nmapPath, nmapOptions, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        ShowErrorMessage("Failed to create Nmap process.");
        return FALSE;
    }

    // Read the Nmap output from the pipe
    CHAR chBuf[MAX_BUFFER_SIZE];
    DWORD dwRead, dwWritten;
    BOOL bSuccess = FALSE;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

        for (;;) {
        bSuccess = ReadFile(hChildStd_OUT_Rd, chBuf, MAX_BUFFER_SIZE, &dwRead, NULL);
        if (!bSuccess || dwRead == 0) {
            break;
        }

        chBuf[dwRead] = '\0';
        AddTextToOutput(chBuf);
        memset(chBuf, 0, sizeof(chBuf));
    }

    // Wait for the Nmap process to exit
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Enable the input control and "Run Nmap" button
    EnableWindow(hwndInput, TRUE);
    EnableWindow(hwndButton, TRUE);

    // Close the handles
    CloseHandle(hChildStd_OUT_Rd);
    CloseHandle(hChildStd_OUT_Wr);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return TRUE;
}

// Adds text to the output edit control
void AddTextToOutput(char* text) {
    int textLength = GetWindowTextLength(hwndOutput);
    SendMessage(hwndOutput, EM_SETSEL, textLength, textLength);
    SendMessage(hwndOutput, EM_REPLACESEL, FALSE, (LPARAM)text);
}

// Shows an error message box
void ShowErrorMessage(char* message) {
    MessageBox(NULL, message, "Error", MB_ICONERROR | MB_OK);
}
