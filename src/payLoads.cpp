#include "payLoads.h"


extern dectObj *dects;
extern CRITICAL_SECTION cs;
extern cSystem *gSys;
extern HANDLE gHdect;

payLoads::payLoads()
{
    threadIndx = 0;
    threadDone = 0;
    workDone = 0;
    hthidEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
    hThid = CreateMutex(NULL,FALSE,NULL);
    hThDone = CreateMutex(NULL,FALSE,NULL);
}

payLoads::~payLoads()
{
    //dtor
}
void payLoads::_switch()
{
    if(threadIndx == 0)SetEvent(hthidEvent); //event wasn't set
    WaitForSingleObject(hthidEvent,INFINITE);
    threadIndx++;
    ResetEvent(hthidEvent);
}

bool payLoads::isExe(const char *file)
{
    if(strstr(file,".exe")!=0)return true;
    else if(strstr(file,".com")!=0)return true;
    else return false;
}

int payLoads::createSystemRestore()
{
    EnterCriticalSection(&cs);
    HWND lhwnd = gSys->getHwnd();
    LeaveCriticalSection(&cs);

    RESTOREPOINTINFOW RestorePtInfo;
    STATEMGRSTATUS SMgrStatus;
    PFN_SETRESTOREPTW fnSRSetRestorePointW = NULL;
    DWORD dwErr = ERROR_SUCCESS;
    HMODULE hSrClient = NULL;
    BOOL fRet = FALSE;
    HRESULT hr = S_OK;


    hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
    if( FAILED( hr ) )
    {
        PostMessage(lhwnd,WM_SYSTEM_MSG,ADD_MSG,0);
        goto exit;
    }

    // Initialize COM security to enable NetworkService,
    // LocalService and System to make callbacks to the process
    // calling  System Restore. This is required for any process
    // that calls SRSetRestorePoint.

    //   fRet = InitializeCOMSecurity();
    //   if( !fRet )
    //   {
    //       wprintf( L"Unexpected error: failed to initialize COM security\n" );
    //       goto exit;
    //   }

    // Initialize the RESTOREPOINTINFO structure.
    RestorePtInfo.dwEventType=BEGIN_SYSTEM_CHANGE;
    RestorePtInfo.dwRestorePtType=APPLICATION_INSTALL;
    RestorePtInfo.llSequenceNumber=0;
    wcscpy(RestorePtInfo.szDescription,L"Sample Restore");


    // Load the DLL, which may not exist on Windows server
    hSrClient = LoadLibraryW(L"srclient.dll");
    if(NULL == hSrClient)
    {
        PostMessage(lhwnd,WM_SYSTEM_MSG,ADD_MSG,1);
        goto exit;
    }

    // If the library is loaded, find the entry point
    fnSRSetRestorePointW = (PFN_SETRESTOREPTW) GetProcAddress(
                               hSrClient, "SRSetRestorePointW");
    if (NULL == fnSRSetRestorePointW)
    {
        PostMessage(lhwnd,WM_SYSTEM_MSG,ADD_MSG,2);
        goto exit;
    }

    fRet = fnSRSetRestorePointW(&RestorePtInfo, &SMgrStatus);
    if(!fRet)
    {
        dwErr = SMgrStatus.nStatus;
        if(dwErr == ERROR_SERVICE_DISABLED)
        {
            PostMessage(lhwnd,WM_SYSTEM_MSG,ADD_MSG,3);
            goto exit;
        }
        PostMessage(lhwnd,WM_SYSTEM_MSG,ADD_MSG,4);
        goto exit;
    }
    PostMessage(lhwnd,WM_SYSTEM_MSG,ADD_MSG,5);

exit:

    if(hSrClient != NULL)
    {
        FreeLibrary(hSrClient);
        hSrClient = NULL;
    }

    return 0;
}

DWORD payLoads::run()
{
    int th = threadIndx;
    SetEvent(hthidEvent);
    switch(th)
    {
    case 0:
        createSystemRestore();
        break;
    case 1:
    {
        /*
        End process if its running
        Delete the file
        Search for registry entries
        */
        EnterCriticalSection(&cs);
        int nproc = dects->getNoproc();
        int nfile = dects->getNofiles();
        HWND lhwnd = gSys->getHwnd();
        LeaveCriticalSection(&cs);


        for(int i=0; i<nfile; i++)
        {
            WaitForSingleObject(gHdect,INFINITE);
            workDone++;
            char tmp[MAX_PATH]= {0};
            if(i<nproc)strcpy(tmp,dects->NextName(i));
            char tm[MAX_PATH]= {0};
            strcpy(tm,dects->NextFile(i));
            ReleaseMutex(gHdect);

            if(isExe(tmp)&&(i<nproc))
            {
                int iskilled = killProcess(tmp);
                if(iskilled  == 0)PostMessage(lhwnd,WM_PROC_KILLED,i,0);
                else if(iskilled == 1) PostMessage(lhwnd,WM_PROC_KILLED,i,1);
                else PostMessage(lhwnd,WM_PROC_KILLED,i,2);
            }
            //Sleep(10L); //wait for file to be complete removed
            int ret = DeleteFile(tm);
            if(ret>0)PostMessage(lhwnd,WM_FILE_DEL,0,i);
            else
            {
                error = true;
                PostMessage(lhwnd,WM_FILE_DEL,1,i);
            }
        }
        WaitForSingleObject(hThDone,INFINITE);
        threadDone++;
        int indx = threadDone;
        ReleaseMutex(hThDone);
        if(indx >= 2)
        {
            if(!error||(workDone == 0))
                PostMessage(lhwnd,WM_PAYLOAD_DONE,_NO_ERROR,0);
            else PostMessage(lhwnd,WM_PAYLOAD_DONE,_ERROR,0);

        }
    }
    break;
    case 2:
    {
        EnterCriticalSection(&cs);
        int sz = dects->regS.size();
        HWND lhwnd = gSys->getHwnd();
        bool is64 = gSys->is64();
        LeaveCriticalSection(&cs);
        HKEY key;
        long Ret = 0;
        char *strKeyName,*strValueName;
        long rokRetVal = -1;

        for(int i = 0; i<sz; i++)
        {
            int handle = dects->regS[i].handle;
            strKeyName = (char*)malloc(((strlen(dects->regS.at(i).keyPath.c_str()))+1)*sizeof(char*));
            strcpy(strKeyName,dects->regS.at(i).keyPath.c_str());
            switch (handle)
            {
            case 0:
                rokRetVal = RegOpenKey(HKEY_CLASSES_ROOT,strKeyName,&key);
                break;
            case 1:
                rokRetVal = RegOpenKey(HKEY_CURRENT_USER,strKeyName,&key);
                break;
            case 2:
                if(is64)
                    rokRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE,strKeyName,0,KEY_ALL_ACCESS|KEY_WOW64_64KEY,&key);
                else rokRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE,strKeyName,0,KEY_ALL_ACCESS,&key);
                break;
            case 3:
                rokRetVal = RegOpenKey(HKEY_USERS,strKeyName,&key);
                break;
            case 4:
                rokRetVal = RegOpenKey(HKEY_CURRENT_CONFIG,strKeyName,&key);
                break;

            }
            WaitForSingleObject(gHdect,INFINITE);
            workDone++;
            int valueSz = dects->regS.at(i).values.size();
            if(valueSz>0)
            {
                for(int j=0; j<valueSz; j++)
                {
                    int handle = dects->regS.at(i).handle;
                    int keyValSz = dects->regS.at(i).values.size();
                    strValueName = (char*)malloc((strlen(dects->regS[i].values[j].valueName.c_str())+1)*sizeof(char));
                    strcpy(strValueName,dects->regS[i].values[j].valueName.c_str());
                    Ret = RegDeleteValue(key,strValueName);
                    if(Ret == ERROR_SUCCESS)PostMessage(lhwnd,WM_REGS_REMOVED,_NO_ERROR,0);
                    else
                    {
                        error = true;
                        PostMessage(lhwnd,WM_REGS_REMOVED,_ERROR,Ret);
                    }
                }
            }
            //}
            ReleaseMutex(gHdect);
        }
        //free(strKeyName);
        //free(strValueName);

        WaitForSingleObject(hThDone,INFINITE);
        threadDone++;
        int indx = threadDone;
        ReleaseMutex(hThDone);

        if(indx >= 2)
        {
            if(!error||(workDone==0))PostMessage(lhwnd,WM_PAYLOAD_DONE,_NO_ERROR,0);
            else PostMessage(lhwnd,WM_PAYLOAD_DONE,ERROR,0);
        }
    }
    break;
    }
}

int payLoads::killProcess(const char *filename)
{
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
    PROCESSENTRY32 pEntry;
    bool isKilled = false,procExist = false;

    pEntry.dwSize = sizeof (pEntry);
    BOOL hRes = Process32First(hSnapShot, &pEntry);

    while (hRes)
    {
        if (strcmp(pEntry.szExeFile, filename) == 0)
        {
            procExist = true;
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
                                          (DWORD) pEntry.th32ProcessID);
            if (hProcess != NULL)
            {
                isKilled = TerminateProcess(hProcess, 9);
                CloseHandle(hProcess);
            }
        }
        hRes = Process32Next(hSnapShot, &pEntry);
    }
    CloseHandle(hSnapShot);
    if(procExist&&isKilled)return 0;
    else if(procExist&&(!isKilled))return 1;
    else if(!procExist)return 2;
}
