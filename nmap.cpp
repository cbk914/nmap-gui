#include <Windows.h>
#include <CommCtrl.h> // for common controls

// Declare the name of the window class
const char g_szClassName[] = "myWindowClass";
const int g_nWidth = 400;
const int g_nHeight = 250;

// Step 1: Registering the Window Class
WNDCLASSEX wc;
ZeroMemory(&wc, sizeof(wc));
wc.cbSize = sizeof(wc);
wc.style = 0;
wc.lpfnWndProc = WndProc;
wc.cbClsExtra = 0;
wc.cbWndExtra = 0;
wc.hInstance = hInstance;
wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
wc.hCursor = LoadCursor(NULL, IDC_ARROW);
wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
wc.lpszMenuName = NULL;
wc.lpszClassName = g_szClassName;
wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
if (!RegisterClassEx(&wc)) {
    MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
    return 0;
}

// Step 2: Creating the Window
HWND hwnd = CreateWindowEx(
    WS_EX_CLIENTEDGE,
    g_szClassName,
    "Nmap GUI",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, g_nWidth, g_nHeight,
    NULL, NULL, hInstance, NULL);

if (hwnd == NULL) {
    MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
    return 0;
}

// Step 3: Show the Window
ShowWindow(hwnd, nCmdShow);
UpdateWindow(hwnd);


// Step 4: the Window Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndInput;
    static HWND hwndButton;
    static HWND hwndOutput;
    static HWND hwndProgressBar;
    static HANDLE hChildProcess;
    static DWORD dwChildThreadId;
    static STARTUPINFO si;
    static PROCESS_INFORMATION pi;

    switch (msg)
    {
    case WM_CREATE:
        // Create the input field
        hwndInput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL, 10, 10, g_nWidth - 40, 50, hwnd, NULL, GetModuleHandle(NULL), NULL);
        SetWindowText(hwndInput, "nmap -sS -T4 -A -v");

        // Create the run button
        hwndButton = CreateWindow("BUTTON", "Run", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, g_nWidth / 2 - 50, 70, 100, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);

        // Create the output field
        hwndOutput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY, 10, 110, g_nWidth - 40, g_nHeight - 160, hwnd, NULL, GetModuleHandle(NULL), NULL);

        // Create the progress bar
        hwndProgressBar = CreateWindowEx(0, PROGRESS_CLASS, "", WS_CHILD | WS_VISIBLE | PBS_SMOOTH, 10, g_nHeight - 40, g_nWidth - 40, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
        SendMessage(hwndProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
        SendMessage(hwndProgressBar, PBM_SETSTEP, (WPARAM)1, 0);

        // Initialize the process information structure
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES;

        // Redirect output to our window
        si.hStdOutput = (HANDLE)_get_osfhandle(_open_osfhandle((intptr_t)CreateFile("CONOUT$", GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL), _O_TEXT));
        si.hStdError = si.hStdOutput;

        break;
    case WM_COMMAND:
        if ((HWND)lParam == hwndButton) {
            // Get the command line text
            char szCommandLine[1024];
            GetWindowText(hwndInput, szCommandLine, sizeof(szCommandLine));

            // Disable the button
            EnableWindow(hwndButton, FALSE);

            // Clear the output field
            SetWindowText(hwndOutput, "");

            // Reset the progress bar
            SendMessage(hwndProgressBar, PBM_SETPOS, 0, 0);

            // Create the child process
            if (CreateProcess(NULL, szCommandLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi
        )) {
            hChildProcess = pi.hProcess;
            dwChildThreadId = pi.dwThreadId;

            // Start a loop to read the child process output
            char szOutput[1024];
            DWORD dwBytesRead;
            DWORD dwTotalBytesRead = 0;
            while (TRUE) {
                if (!ReadFile(si.hStdOutput, szOutput, sizeof(szOutput), &dwBytesRead, NULL) || dwBytesRead == 0) {
                    break;
                }

                szOutput[dwBytesRead] = 0;
                dwTotalBytesRead += dwBytesRead;

                // Display the output in the output field
                int nTextLength = GetWindowTextLength(hwndOutput);
                SendMessage(hwndOutput, EM_SETSEL, (WPARAM)nTextLength, (LPARAM)nTextLength);
                SendMessage(hwndOutput, EM_REPLACESEL, 0, (LPARAM)szOutput);

                // Update the progress bar
                int nProgress = ((float)dwTotalBytesRead / (float)pi.dwProcessId) * 100.0;
                SendMessage(hwndProgressBar, PBM_SETPOS, nProgress, 0);
            }

            // Close the handles to the child process
            CloseHandle(hChildProcess);
            CloseHandle(pi.hThread);
        }

        // Enable the button
        EnableWindow(hwndButton, TRUE);
    }

    break;
case WM_CLOSE:
    // Terminate the child process if it is still running
    if (hChildProcess) {
        TerminateProcess(hChildProcess, 0);
    }

    // Destroy the window
    DestroyWindow(hwnd);

    break;
case WM_DESTROY:
    PostQuitMessage(0);

    break;
default:
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

// Step 5: The Message Loop
MSG msg;
while (GetMessage(&msg, NULL, 0, 0) > 0)
{
    TranslateMessage
(&msg);
DispatchMessage(&msg);
}

return (int)msg.wParam;
}

// Function to get the nmap command line parameters
void GetNmapParameters(HWND hwnd, char *szParams, int nMaxParams)
{
char szBuffer[1024];
// Get the target IP address or hostname from the input field
GetWindowText(hwndTarget, szBuffer, sizeof(szBuffer));
if (strlen(szBuffer) > 0) {
    strcat_s(szParams, nMaxParams, szBuffer);
}

// Get the port range from the input field
GetWindowText(hwndPortRange, szBuffer, sizeof(szBuffer));
if (strlen(szBuffer) > 0) {
    strcat_s(szParams, nMaxParams, " -p ");
    strcat_s(szParams, nMaxParams, szBuffer);
}

// Get the scan type from the combo box
int nSelectedIndex = SendMessage(hwndScanType, CB_GETCURSEL, 0, 0);
if (nSelectedIndex != CB_ERR) {
    int nSelectedType = SendMessage(hwndScanType, CB_GETITEMDATA, nSelectedIndex, 0);
    if (nSelectedType != CB_ERR) {
        switch (nSelectedType) {
        case SCAN_TYPE_TCP:
            strcat_s(szParams, nMaxParams, " -sT");
            break;
        case SCAN_TYPE_UDP:
            strcat_s(szParams, nMaxParams, " -sU");
            break;
        case SCAN_TYPE_SYN:
            strcat_s(szParams, nMaxParams, " -sS");
            break;
        }
    }
}
}

// Function to run the nmap command
BOOL RunNmapCommand(HWND hwndOutput, HWND hwndProgressBar, char *szParams)
{
// Create the command line to run nmap
char szCommandLine[2048];
strcpy_s(szCommandLine, sizeof(szCommandLine), "nmap.exe ");
strcat_s(szCommandLine, sizeof(szCommandLine), szParams);
// read the child process's stdout and stderr and display the output in the output window
char szOutput[1024];
DWORD dwBytesRead;
BOOL bSuccess;
while (1) {
// Check for messages in the message queue
MSG msg;
while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
TranslateMessage(&msg);
DispatchMessage(&msg);
}
    // Check if the child process has exited
    DWORD dwExitCode;
    if (GetExitCodeProcess(pi.hProcess, &dwExitCode)) {
        if (dwExitCode != STILL_ACTIVE) {
            break;
        }
    }

    // Read from the child process's stdout
    bSuccess = ReadFile(hChildStdoutRead, szOutput, sizeof(szOutput) - 1, &dwBytesRead, NULL);
    if (bSuccess && dwBytesRead > 0) {
        szOutput[dwBytesRead] = '\0';
        SendMessage(hwndOutput, EM_REPLACESEL, 0, (LPARAM)szOutput);
    }

    // Read from the child process's stderr
    bSuccess = ReadFile(hChildStderrRead, szOutput, sizeof(szOutput) - 1, &dwBytesRead, NULL);
    if (bSuccess && dwBytesRead > 0) {
        szOutput[dwBytesRead] = '\0';
        SendMessage(hwndOutput, EM_REPLACESEL, 0, (LPARAM)szOutput);
    }

    // Update the progress bar based on the child process's CPU usage
    FILETIME ftCreationTime;
    FILETIME ftExitTime;
    FILETIME ftKernelTime;
    FILETIME ftUserTime;
    if (GetProcessTimes(pi.hProcess, &ftCreationTime, &ftExitTime, &ftKernelTime, &ftUserTime)) {
        ULARGE_INTEGER liKernelTime;
        liKernelTime.LowPart = ftKernelTime.dwLowDateTime;
        liKernelTime.HighPart = ftKernelTime.dwHighDateTime;
        ULARGE_INTEGER liUserTime;
        liUserTime.LowPart = ftUserTime.dwLowDateTime;
        liUserTime.HighPart = ftUserTime.dwHighDateTime;
        ULONGLONG ullTotalTime = liKernelTime.QuadPart + liUserTime.QuadPart;
        ULONGLONG ullElapsedTime = GetTickCount() - dwStartTime;
        int nProgress = (int)(ullTotalTime * 100 / ullElapsedTime);
        SendMessage(hwndProgressBar, PBM_SETPOS, nProgress, 0);
    }
}

// Read any remaining output from the child process's stdout and stderr
while (1) {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DWORD dwBytesRead;
    BOOL bSuccess = ReadFile(hChildStdoutRead, szOutput, sizeof(szOutput) - 1, &dwBytesRead, NULL);
    if (bSuccess && dwBytesRead > 0) {
        szOutput[dwBytesRead] = '\0';
        SendMessage(hwndOutput, EM_REPLACESEL, 0, (LPARAM)szOutput);
    }

    bSuccess = ReadFile(hChildStderrRead, szOutput, sizeof(szOutput) - 1, &dwBytesRead, NULL);
    if (bSuccess && dwBytesRead > 0) {
        szOutput[dwBytesRead] = '\0';
        SendMessage(hwndOutput, EM_REPLACESEL, 0, (LPARAM)szOutput);
    }

    // Exit the loop when there is no more output to read
    if (!
    // Check if the child process has exited
    DWORD dwExitCode;
    if (GetExitCodeProcess(pi.hProcess, &dwExitCode)) {
        if (dwExitCode != STILL_ACTIVE) {
            break;
        }
    }
}

// Disable the progress bar and enable the "Run" button
EnableWindow(hwndProgressBar, FALSE);
EnableWindow(hwndButton, TRUE);

// Close the handles to the child process and pipes
CloseHandle(pi.hProcess);
CloseHandle(pi.hThread);
CloseHandle(hChildStdinRead);
CloseHandle(hChildStdinWrite);
CloseHandle(hChildStdoutRead);
CloseHandle(hChildStdoutWrite);
CloseHandle(hChildStderrRead);
CloseHandle(hChildStderrWrite);
}

// Window procedure for the main window
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
static HWND hwndButton;
static HWND hwndOutput;
static HWND hwndProgressBar;
static HANDLE hProcess;
switch (msg)
{
case WM_CREATE:
    // Create the "Run" button
    hwndButton = CreateWindowEx(0, L"BUTTON", L"Run", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        10, 10, 100, 25, hwnd, (HMENU)IDC_RUN_BUTTON, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

    // Create the output window
    hwndOutput = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
        10, 45, 480, 300, hwnd, (HMENU)IDC_OUTPUT_WINDOW, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

    // Create the progress bar
    hwndProgressBar = CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE,
        10, 355, 480, 20, hwnd, (HMENU)IDC_PROGRESS_BAR, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

    // Set the range of the progress bar to 0-100
    SendMessage(hwndProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

    break;

case WM_COMMAND:
    if (LOWORD(wParam) == IDC_RUN_BUTTON && HIWORD(wParam) == BN_CLICKED) {
        // Disable the "Run" button and enable the progress bar
        EnableWindow(hwndButton, FALSE);
        EnableWindow(hwndProgressBar, TRUE);

        // Start the child process
        TCHAR szCommandLine[1024];
        GetDlgItemText(hwnd, IDC_COMMAND_LINE, szCommandLine, sizeof(szCommandLine) / sizeof(TCHAR));
        StartChildProcess(szCommandLine, hwndOutput, hwndProgressBar, hwndButton);

        // Clear the command line text
        SetDlgItemText(hwnd, IDC_COMMAND_LINE, L"");
    }
    break;

case WM_DESTROY:
    // Terminate the child process if it is still running
    if (hProcess != NULL) {
        TerminateProcess(hProcess, 1);
        CloseHandle(hProcess);
    }

    // Quit the application
    PostQuitMessage(0);
    break;

default:
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

return 0;
}

// Entry point for the application
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
// Register the window class
const wchar_t CLASS_NAME
= L"NmapGUI";

WNDCLASS wc = { };
wc.lpfnWndProc = WndProc;
wc.hInstance = hInstance;
wc.lpszClassName = CLASS_NAME;

RegisterClass(&wc);

// Create the main window
HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"NmapGUI", WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, 520, 420, NULL, NULL, hInstance, NULL);

if (hwnd == NULL) {
    return 0;
}

// Create the "Command line" label
CreateWindow(L"STATIC", L"Command line:", WS_CHILD | WS_VISIBLE,
    10, 385, 100, 20, hwnd, NULL, hInstance, NULL);

// Create the "Command line" text box
CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
    110, 385, 380, 20, hwnd, (HMENU)IDC_COMMAND_LINE, hInstance, NULL);

// Show the main window
ShowWindow(hwnd, nCmdShow);

// Run the message loop
MSG msg = { };
while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
}

return 0;
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
switch (msg)
{
case WM_CREATE:
// Create the "Scan" button
CreateWindow(L"BUTTON", L"Scan", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
10, 10, 80, 25, hwnd, (HMENU)IDC_SCAN_BUTTON, GetModuleHandle(NULL), NULL);
    // Create the "Host or network" label
    CreateWindow(L"STATIC", L"Host or network:", WS_CHILD | WS_VISIBLE,
        10, 50, 100, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Create the "Host or network" text box
    CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        110, 50, 380, 20, hwnd, (HMENU)IDC_HOST_BOX, GetModuleHandle(NULL), NULL);

    // Create the "Port range" label
    CreateWindow(L"STATIC", L"Port range:", WS_CHILD | WS_VISIBLE,
        10, 80, 100, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Create the "Port range" text box
    CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        110, 80, 380, 20, hwnd, (HMENU)IDC_PORT_BOX, GetModuleHandle(NULL), NULL);

    // Create the "Output file" label
    CreateWindow(L"STATIC", L"Output file:", WS_CHILD | WS_VISIBLE,
        10, 110, 100, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);

    // Create the "Output file" text box
    CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        110, 110, 380, 20, hwnd, (HMENU)IDC_OUTPUT_BOX, GetModuleHandle(NULL), NULL);

    // Create the "Verbose output" check box
    CreateWindow(L"BUTTON", L"Verbose output", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        10, 140, 100, 20, hwnd, (HMENU)IDC_VERBOSE_BOX, GetModuleHandle(NULL), NULL);

    break;

case WM_COMMAND:
    if (LOWORD(wParam) == IDC_SCAN_BUTTON && HIWORD(wParam) == BN_CLICKED)
    {
        // Get the input values from the text boxes and check box
        WCHAR host[1024];
        WCHAR port[1024];
        WCHAR output[1024];
        BOOL verbose = SendMessage(GetDlgItem(hwnd, IDC_VERBOSE_BOX), BM_GETCHECK, 0, 0) == BST_CHECKED;

        GetWindowText(GetDlgItem(hwnd, IDC_HOST_BOX), host, 1024);
        GetWindowText(GetDlgItem(hwnd, IDC_PORT_BOX), port, 1024);
        GetWindowText(GetDlgItem(hwnd, IDC_OUTPUT_BOX), output, 1024);

        // Build the Nmap command line
        WCHAR commandLine[4096];
        swprintf_s(commandLine, 4096, L"nmap.exe %s %s -oN %s %s",
            host, port, output, verbose ? L"-v" : L"");

        // Run the Nmap command
        STARTUPINFO si = { sizeof(si) };
        PROCESS_INFORMATION pi;

        if (CreateProcess(NULL, command
        Line, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        }
        else
        {
            MessageBox(hwnd, L"Error running Nmap command.", L"Error", MB_OK | MB_ICONERROR);
        }
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
LPSTR lpCmdLine, int nCmdShow)
{
// Register the window class
WNDCLASSEX wc = { sizeof(wc) };
wc.style = CS_HREDRAW | CS_VREDRAW;
wc.lpfnWndProc = WndProc;
wc.hInstance = hInstance;
wc.hCursor = LoadCursor(NULL, IDC_ARROW);
wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
wc.lpszClassName = L"NmapGUIWindowClass";
RegisterClassEx(&wc);
// Create the window
HWND hwnd = CreateWindowEx(0, L"NmapGUIWindowClass", L"Nmap GUI",
    WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 520, 220,
    NULL, NULL, hInstance, NULL);

if (!hwnd)
    return 1;

ShowWindow(hwnd, nCmdShow);
UpdateWindow(hwnd);

// Run the message loop
MSG msg = { 0 };
while (GetMessage(&msg, NULL, 0, 0))
{
    TranslateMessage(&msg);
    DispatchMessage(&msg);
}

return (int)msg.wParam;
}
