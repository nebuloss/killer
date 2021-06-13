#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

#define DICTIONARY_HASH_FUNCTION(h,c) ((h<<5)+h)+c

HINSTANCE instance;
DWORD pid_winlogon;

typedef NTSTATUS (*process_function)(HANDLE); //define a type of function

process_function SuspendProcessHandle,ResumeProcessHandle;

BOOLEAN EnableDebugPrivilege(){ //we must grant to the program debug privilege to suspend and resume process
    //https://social.msdn.microsoft.com/Forums/exchange/en-US/3d32272f-162c-4700-9a20-a179d86cfd14/openprocess-does-not-work-in-a-command-prompt-but-works-on-ps-and-debuggers?forum=windowsgeneraldevelopmentissues
    BOOL ret = FALSE;
	HANDLE hToken = NULL;
	TOKEN_PRIVILEGES tp;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) return FALSE;

	if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid)){
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		if (AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL)){
		    if (GetLastError()!= ERROR_NOT_ALL_ASSIGNED) ret=TRUE;
        }
	}

	CloseHandle(hToken);
	return ret;
}

BOOLEAN InitializeExports(){
    //we use native windows functions that are not documented but widely used
    HMODULE hNtdll = GetModuleHandle("NTDLL");
    if (!hNtdll) return FALSE;
    
    if (!(SuspendProcessHandle=(process_function)GetProcAddress(hNtdll,"NtSuspendProcess"))) return FALSE;
    if (!(ResumeProcessHandle=(process_function)GetProcAddress(hNtdll,"NtResumeProcess"))) return FALSE;
    
    return TRUE;
}

DWORD64 GetLPSTRHash(LPSTR s){
    //more efficient string comparison
    DWORD64 h=0;
    BYTE c;
    while((c=*s++)) h=(h<<5)+h+c;
    return h;
}

DWORD FindPID(char *name){ //return the pid of the program from his name (first occurence)
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    DWORD result = NULL;
    DWORD64 hn;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //get all processes running
    if (INVALID_HANDLE_VALUE == hProcessSnap) return 0;

    pe32.dwSize = sizeof(PROCESSENTRY32); 

    if (!Process32First(hProcessSnap, &pe32)){
        CloseHandle(hProcessSnap);      
        return 0;
    }

    hn=GetLPSTRHash(name);
    do{ //browse the list of processes
        if (GetLPSTRHash(pe32.szExeFile)==hn){
            result = pe32.th32ProcessID;
            break;
        }
    }while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return result;
}

NTSTATUS KillProcessHandle(HANDLE current_handle){
   return TerminateProcess(current_handle,1); 
}

DWORD ExecPIDFunction(DWORD pid,process_function pf){
    HANDLE current_handle;

    if (!pid) return pid;
    current_handle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,pid);

    if (pf(current_handle)) pid=0;
    CloseHandle(current_handle);

    return pid;
}

DWORD KillPID(DWORD pid){
    return ExecPIDFunction(pid,KillProcessHandle);
}

DWORD KillProcess(char* name){
    return KillPID(FindPID(name));
}

DWORD SuspendPID(DWORD pid){
    return ExecPIDFunction(pid,SuspendProcessHandle);
}

DWORD ResumePID(DWORD pid){
    return ExecPIDFunction(pid,ResumeProcessHandle);
}

void SuspendProcesses(){  //buttons actions
    KillProcess("explorer.exe");   
    SuspendPID(pid_winlogon);
    KillProcess("dwm.exe");
}

void ResumeProcesses(){
    ResumePID(pid_winlogon);
    if (!FindPID("explorer.exe")) ShellExecute(NULL,"open","explorer",NULL,NULL,0);
}

//very good french tutorial
//https://pub.phyks.me/sdz/sdz/apprentissage-de-l-api-windows.html


LRESULT CALLBACK WindowCallbackFunction(HWND window, UINT msg, WPARAM wParam, LPARAM lParam){
    switch (msg){
        case WM_CREATE:
            CreateWindow("BUTTON", "Suspend", WS_CHILD | WS_VISIBLE,5, 5, 383, 30, window, (HMENU)0, instance, NULL);
            CreateWindow("BUTTON", "Resume", WS_CHILD | WS_VISIBLE,5, 45, 383, 30, window, (HMENU)1, instance, NULL);
            break;   

        case WM_COMMAND:
            if (LOWORD(wParam)) ResumeProcesses();
            else SuspendProcesses();  
            break;

        case WM_DESTROY:
            ResumeProcesses();
            PostQuitMessage(0);
            break;

        case WM_KEYDOWN:
            if (wParam==VK_ESCAPE) SendMessage(window,WM_DESTROY,0,0);
            break;

        default:
            return DefWindowProc(window, msg, wParam, lParam);
    }
    return 0;
}

int WinMain (HINSTANCE current_instance, HINSTANCE previous_instance,LPSTR commandline, int displaymode){
    HWND window;
    MSG message;

    //check some infrmation before running
    if (!EnableDebugPrivilege()) return 0;
    if (!InitializeExports()) return 0;
    if (!(pid_winlogon=FindPID("winlogon.exe"))) return 0;

    WNDCLASS windowclass={
        .style=0,
        .lpfnWndProc=WindowCallbackFunction,
        .cbClsExtra=0,
        .cbWndExtra=0,
        .hInstance=NULL,
        .hIcon=LoadIcon(NULL,IDI_APPLICATION),
        .hCursor=LoadCursor(NULL,IDC_ARROW),
        .hbrBackground=(HBRUSH)(1 + COLOR_BTNFACE),
        .lpszMenuName=NULL,
        .lpszClassName="class"
    };

    if(!RegisterClass(&windowclass)) return 0;

    window = CreateWindow("class", "Killer by Nebuloss",WS_BORDER,CW_USEDEFAULT,CW_USEDEFAULT,400,110,NULL,NULL,current_instance,NULL);
    if (!window) return 0;

    instance=current_instance;

    //SetWindowPos(window,HWND_TOPMOST,500,300,0,0,SWP_NOSIZE); // cause some issues in games 
    ShowWindow(window, displaymode);
    UpdateWindow(window);

    while (GetMessage(&message,NULL,0,0)){
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
    
    return message.wParam;
}