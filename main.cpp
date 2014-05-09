#include <ctime>
#include <ctype.h>
#include <math.h>

#include "Reader.h"
#include "helper.h"
#include "cSystem.h"
#include "include/resource.h"
#include "cEngine.h"
#include "payloads.h"
#include "version.h"

HINSTANCE hInst;
static HWND hList = NULL;
CRITICAL_SECTION cs;

HINSTANCE g_hinst;
char statMsg[32];
HWND hTool,hStatus,hProgbar;
char initLmsg[MAX_LEN];
int index=0;
int iToolHeight=0;
HWND hlist,g_hwnd;
string Buf;
int chBut=0,reBut=0,nDefns=0;
defObj *defs;
Reader *mReader;
dectObj *dects;
HANDLE hTread,phnd,gHmutex,gHdect,gHdefs;
char defnFname[MAX_PATH] = DEF_DEF_FNAME;
helper mainHelper;
cSystem *gSys;
cEngine *mEngine;
payLoads *mpayLoads;

long long divs = 0,incr = 0;

int addMsg(const char Msg[]);
int getNoDefn(string Def);
int changeStatMsg(const char msg[]);


BOOL CALLBACK AboutDiag(HWND hwnd,UINT Message,WPARAM wParam,LPARAM lParam)
{
    switch(Message)
    {
    case WM_INITDIALOG:
        return TRUE;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            EndDialog(hwnd,IDOK);
            break;
        case IDCANCEL:
            EndDialog(hwnd,IDCANCEL);
            break;
        }
    default:
        return FALSE;
    }
    return TRUE;
}

int openDefFile()
{
    HWND hwnd= g_hwnd;

    fstream deFile;
    deFile.open(defnFname,ios::in);
    if(deFile.is_open())
    {
        if(deFile.good())
        {
            getline(deFile,Buf);
            addMsg(Buf.c_str());
            getline(deFile,Buf);
            addMsg(Buf.c_str());
            nDefns = getNoDefn(Buf);
            deFile.close();
            chBut = TBSTATE_ENABLED;

            defs = new defObj[nDefns]; //create an array of defn object based on no of defns read from the file
            mReader = new Reader(defs,nDefns);
            mEngine = new cEngine(defs,nDefns);
            mReader->start();
        }
        else
        {
            MessageBox(hwnd,"Error reading the file","Error",MB_OK|MB_ICONERROR);
            changeStatMsg("No definitions!");
            addMsg("Version:Unknown");
        }
    }
    else
    {
        MessageBox(hwnd,"We haven't Found the definition file","Error",MB_OK|MB_ICONERROR);
        changeStatMsg("No definitions!");
        addMsg("Version:Unknown");
    }

    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
    case WM_CREATE:
    {
        gHmutex = CreateMutex(NULL,FALSE,NULL);
        gHdect = CreateMutex(NULL,FALSE,NULL);
        gHdefs = CreateMutex(NULL,FALSE,NULL);
    }
        break;
    case WM_SIZE:
    {
        HWND hStatus;

        HWND hTool;

        // Size toolbar and get height
        hTool = GetDlgItem(hwnd, IDC_MAIN_TOOL);
        SendMessage(hTool, TB_AUTOSIZE, 0, 0);

        // Size status bar and get height
        hStatus = GetDlgItem(hwnd, IDC_MAIN_STATUS);
        SendMessage(hStatus, WM_SIZE, 0, 0);
    }
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        //CloseHandle(phnd);
        DeleteCriticalSection(&cs);
        PostQuitMessage(0);
        break;
    case WM_START_READ:

        break;
    case WM_ENGINE_DONE:
    {
        SendMessage(hProgbar,PBM_SETPOS,(WPARAM)100,0);
        addMsg("Search Engine done:");
        changeStatMsg("Done Working:)");
        addMsg("Initializing payload Engine...");

        mpayLoads = new payLoads();

        HMENU hm = GetMenu(hwnd);
        HWND hTool = GetDlgItem(hwnd, IDC_MAIN_TOOL);
        EnableMenuItem(hm,IDM_REPAIR,MF_ENABLED);
        SendMessage(hTool,TB_ENABLEBUTTON,IDM_REPAIR,TRUE);

    }
        break;
    case WM_SYSTEM_MSG:
    {
        switch (wParam) {
        case ADD_MSG:
            switch (lParam) {
            case 0:
                addMsg(MSG_SYS_INI_CO);
                break;
            case 1:
                addMsg(MSG_SR_NP);
                break;
            case 2:
                addMsg(MSG_SR_FF);
                break;
            case 3:
                addMsg(MSG_SR_OFF);
                break;
            case 4:
                addMsg(MSG_SR_FAIL);
                break;
            case 5:
                addMsg(MSG_SR_SUCC);
                break;
            }
            if(lParam<5)
            {
                long ret = MessageBox(hwnd,"System Restore encountered an error\n Do you wish to continue?",
                                      "Error",MB_ICONERROR|MB_YESNO);
                if(ret == IDYES){
                    addMsg(MSG_LM_REP_START);
                    mpayLoads->_switch();
                    mpayLoads->start();
                    mpayLoads->_switch();
                    mpayLoads->start();
                }
                else addMsg(MSG_LM_REP_ABORTED);
            }
            break;
        default:
            break;
        }
    }
        break;
    case WM_REGS_REMOVED:
    {
        switch (wParam) {
        case _NO_ERROR:
            addMsg("Registry Entry Removed");
            break;
        case _ERROR:
            if(lParam == 0x2)addMsg("Registry Value Not Present");
            break;
        default:
            break;
        }

    }
        break;
    case WM_FILE_DEL:
    {
        switch(wParam)
        {
        case 0:
        {
            char mg[128] = "File deleted:";
            WaitForSingleObject(gHdect,INFINITE);
            strcat(mg,dects->NextFile(lParam));
            ReleaseMutex(gHdect);
            addMsg(mg);
        }
            break;
        case 1:
        {
            char mg[128] = "Failed to delete:";
            WaitForSingleObject(gHdect,INFINITE);
            strcat(mg,dects->NextFile(lParam));
            ReleaseMutex(gHdect);
            addMsg(mg);
        }
            break;
        }

    }
        break;
    case WM_PROC_KILLED:
    {
        WaitForSingleObject(gHdect,INFINITE);
        switch(lParam){
        case 0:
        {
            char mg[128] = "Process Killed:";
            strcat(mg,dects->NextName(wParam));
            addMsg(mg);
            free(mg);
        }
            break;
        case 1:
        {
            char mg[128] = "Failed to kill process:";
            strcat(mg,dects->NextName(wParam));
            addMsg(mg);
            free(mg);
        }
            break;
        case 2:
            addMsg("Process doesn't exist");
            break;
        }
        ReleaseMutex(gHdect);
    }
        break;
    case WM_READ_READY:
    {
        int ret = lParam;
        char ch[128];
        char ms[] = "Definitions detected from file: ";
        mainHelper.appendInt(ms,ret,ch);
        addMsg(ch);
        memset(ch,0,128);
        strcpy(ch,"File created by ");
        strcat(ch,mReader->owner.c_str());
        addMsg(ch);
        memset(ch,0,128);
        strcpy(ch,"File created on ");
        strcat(ch,mReader->date.c_str());
        addMsg(ch);
        if(ret == nDefns &&(gSys->getRet())!=STILL_ACTIVE)SendMessage(hwnd,WM_INIT_READY,0,0);
    }
        break;
    case WM_OPEN_DEF_FILE:
        openDefFile();
        break;
    case WM_INCREMENT_WD:
    {
        if(divs>0){
        if(!(incr%divs)){
                SendMessage(hProgbar,PBM_STEPIT,0,0);
                incr = 0;
            }
            else incr++;
        }
        else SendMessage(hProgbar,PBM_STEPIT,0,0);
    }
        break;
    case WM_DETECTION:
    {
        char tmp[MAX_PATH] = "Found Malware:";
        strcat(tmp,dects->NextFile(wParam));
        addMsg(tmp);
    }
        break;
    case WM_SYS_INIT_READY:

        break;
    case WM_INIT_READY:
    {
        HMENU hm = GetMenu(hwnd);
        HWND hTool = GetDlgItem(hwnd, IDC_MAIN_TOOL);
        EnableMenuItem(hm,IDM_CHECK,MF_ENABLED);
        SendMessage(hTool,TB_ENABLEBUTTON,IDM_CHECK,TRUE);
        changeStatMsg("Ready...");
    }
        break;
    case WM_PAYLOAD_DONE:
    {
        if(wParam == _NO_ERROR){
            addMsg(MSG_LM_REP_END_OK);
            changeStatMsg("Done Repairing:)");
        }
        else
        {
            addMsg(MSG_LM_REP_END_ERR);//apply error reporting
            changeStatMsg("Done Repairing:(");
        }
        HWND hTool = GetDlgItem(hwnd, IDC_MAIN_TOOL);
        SendMessage(hTool,TB_SETSTATE,IDM_REPAIR,0);

    }
        break;
    case WM_ENGINE_INIT_READY:
        {
            if(lParam > 100){
                 divs = lParam/100;
                 SendMessage(hProgbar,PBM_SETRANGE,0,MAKELPARAM(0,100));
                 SendMessage(hProgbar, PBM_SETSTEP, (WPARAM)1, 0);
            }
            else {
                    SendMessage(hProgbar,PBM_SETRANGE,0,MAKELPARAM(0,lParam));
                    SendMessage(hProgbar, PBM_SETSTEP, (WPARAM)(100/lParam), 0);
            }
        }
        break;
    case WM_GUI_INIT_READY:
        changeStatMsg("Initializing...");
        SendMessage(hwnd,WM_OPEN_DEF_FILE,0,0);
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDM_EXIT:
            PostMessage(hwnd,WM_CLOSE,0,0);
            break;
        case ID_HELP_ABOUT:
        {
            int ret = DialogBox(GetModuleHandle(NULL),
                                MAKEINTRESOURCE(IDD_ABOUT),
                                hwnd,AboutDiag);
            if(ret == IDOK)
            {
                MessageBox(hwnd,"Thank you for Using Covir","Note",MB_OK|MB_ICONINFORMATION);
            }
            else if(ret == IDCANCEL)
            {
                MessageBox(hwnd,"Good Day!","Note",MB_OK);
            }
        }
            break;
        case IDM_CHECK:
        {
            addMsg("Checking Started...");
            changeStatMsg("Working...:)");
            HWND hTool = GetDlgItem(hwnd, IDC_MAIN_TOOL);
            SendMessage(hTool,TB_SETSTATE,IDM_CHECK,0);

            mEngine->start();
            mEngine->_switch();
            mEngine->start();//second thread
            mEngine->_switch();
            mEngine->start();
        }
            break;
        case IDM_REPAIR:
        {
            long ret = MessageBox(hwnd,
                                  "Covir will now create a System Restore point so that you can \n\r Roll back in case of Error.Do you Agree?"
                                  ,"Create A System Restore Point.",MB_ICONINFORMATION|MB_YESNO);
            if(ret == IDYES)
            {
                mpayLoads->start();
            }
            else
            {
                addMsg(MSG_LM_REP_START);
                changeStatMsg("Repairing...");
                mpayLoads->_switch(); //switch mode
                mpayLoads->start();
                mpayLoads->_switch();
                mpayLoads->start();
            }

        }
            break;
        case IDM_LOAD_DEF:
        {
            OPENFILENAME opf;
            memset(&opf,0,sizeof(OPENFILENAME));
            opf.lStructSize = sizeof(OPENFILENAME);
            opf.hInstance = hInst;
            opf.hwndOwner = hwnd;
            opf.lpstrFilter = "Definition Files\0*.defn\00";;
            opf.Flags = OFN_FILEMUSTEXIST|OFN_EXPLORER;
            opf.lpstrTitle = "Select a .defn File";;
            opf.lpstrFileTitle = defnFname;
            BOOL ret = GetOpenFileName(&opf);
            changeStatMsg("Please Wait...");
            if(strlen(defnFname)!=0)addMsg("Definition file loaded.");
            else {
                strcpy(defnFname,DEF_DEF_FNAME);
                addMsg("Default file loaded");
            }
            if(ret){

                    SendMessage(hwnd,WM_OPEN_DEF_FILE,0,0);
            }

        }
            break;
        case IDM_LOAD_REP:
        {
            char ext[] = "Repair Files\0*.rep\00";
            char sTitle[] = "Select a .rep File";
            OPENFILENAME opf;
            memset(&opf,0,sizeof(opf));
            opf.lStructSize = sizeof(OPENFILENAME);
            opf.hInstance = hInst;
            opf.hwndOwner = hwnd;
            opf.lpstrFilter = ext;
            opf.Flags = OFN_FILEMUSTEXIST|OFN_EXPLORER;
            opf.lpstrTitle = sTitle;
            opf.lpstrFileTitle = defnFname;

            GetOpenFileNameA(&opf);
            if(strlen(defnFname)!=0)addMsg("Repair file loaded.");
            else strncpy(defnFname,DEF_DEF_FNAME,sizeof(DEF_DEF_FNAME));
        }
            break;
        case IDL_CLGS:
            SendMessage(hlist,LVM_DELETEALLITEMS,0,0);
            break;
        case  IDM_CANCEL:
            //warn user
            mEngine->stop();
            addMsg(MSG_LM_CHECK_ABORTED);
            break;
        }
        break;
    default:
        return DefWindowProc(hwnd,msg,wParam,lParam);
    }
    return 0;
}
int addMsg(const char msg[])
{
    int lvIndx = ListView_GetItemCount(hlist);
    if(lvIndx==0)index = 0;
    char stime[32] = {0};
    SYSTEMTIME now;
    GetLocalTime(&now);
    GetTimeFormat(LOCALE_SYSTEM_DEFAULT,LOCALE_NOUSEROVERRIDE,&now,NULL,stime,32);

    LV_ITEM lvi;
    lvi.mask = LVIF_TEXT;
    lvi.iItem = index;
    lvi.state = 0;
    lvi.iSubItem = 0;
    lvi.cchTextMax = 256;
    lvi.pszText=stime;

    SendMessage(hlist,LVM_INSERTITEM,0,(LPARAM)&lvi);
    ListView_SetItemText(hlist,index,1,(char*)msg);

    index++;
    return 0;
}
int changeStatMsg(const char msg[])
{
    HWND hstat = GetDlgItem(g_hwnd,IDC_MAIN_STATUS);
    SendMessage(hstat, SB_SETTEXT, 0, (LPARAM)msg);

}

int initGui()
{
    RECT rcl;

    TBBUTTON tbb[4];
    ZeroMemory(tbb, sizeof(tbb));

    int iNew;

    tbb[0].iBitmap = ID_FILE_CHECK;
    tbb[0].fsState = 0;
    tbb[0].fsStyle = TBSTYLE_BUTTON;
    tbb[0].idCommand = IDM_CHECK;
    tbb[0].iString = iNew;

    tbb[1].iBitmap = ID_FILE_CANCEL;
    tbb[1].fsState = TBSTATE_ENABLED;
    tbb[1].fsStyle = TBSTYLE_BUTTON;
    tbb[1].idCommand = IDM_CANCEL;
    tbb[1].iString = iNew+1;

    tbb[2].iBitmap = ID_FILE_REPAIR;
    tbb[2].fsState = 0; //DISABLED FIRST RUN
    tbb[2].fsStyle = TBSTYLE_BUTTON;
    tbb[2].idCommand = IDM_REPAIR;
    tbb[2].iString = iNew+2;

    tbb[3].iBitmap = ID_FILE_HELP;
    tbb[3].fsState = TBSTATE_ENABLED;
    tbb[3].fsStyle = TBSTYLE_BUTTON;
    tbb[3].idCommand = IDM_HELP;
    tbb[3].iString = iNew+3;

    hTool = CreateToolbarEx(g_hwnd,WS_VISIBLE|WS_CHILD|WS_BORDER|TBSTYLE_TOOLTIPS,
                            IDC_MAIN_TOOL,
                            1,hInst,IDB_BMP,
                            tbb,4,16,16,64,16,sizeof(TBBUTTON));

    iNew = SendMessage(hTool, TB_ADDSTRING,
                       (WPARAM)GetModuleHandle(NULL), (LPARAM)IDS_NEW);

    hStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL,
                             WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0,
                             g_hwnd, (HMENU)IDC_MAIN_STATUS, GetModuleHandle(NULL), NULL);
    int statwidths[] = {100, -1};
    SendMessage(hStatus, SB_SETPARTS, sizeof(statwidths)/sizeof(int),
                (LPARAM)statwidths);
    changeStatMsg(statMsg);
    hProgbar= CreateWindowEx(NULL, "msctls_progress32", NULL, WS_CHILD | WS_VISIBLE, 215,2,163,18,
                             hStatus, (HMENU)IDD_PROGRESS, hInst, NULL);
    //SendMessage(hProgbar, PBM_SETSTEP, (WPARAM)5, 0);
    //SendMessage(hProgbar, PBM_STEPIT,0, 0);
    GetClientRect(g_hwnd,&rcl);

    RECT rcTool,rcStat;
    GetWindowRect(hTool, &rcTool);
    GetWindowRect(hStatus,&rcStat);
    iToolHeight = (rcTool.bottom - rcTool.top)+SM_CYMENU;
    int iStatHeight = rcStat.bottom-rcStat.top;

    hlist = CreateWindowEx(0, (LPCSTR) WC_LISTVIEW, NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_SHOWSELALWAYS | LVS_REPORT,
                           0,iToolHeight, rcl.right-rcl.left, rcl.bottom-(rcl.top+iStatHeight+iToolHeight),
                           g_hwnd, (HMENU)IDR_LVIEW, GetModuleHandle(NULL), NULL);


    LVCOLUMN lvc;
    memset(&lvc,0,sizeof(lvc));

    lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.pszText = "Time";
    lvc.cx = 120;
    lvc.fmt = LVCFMT_LEFT;
    SendMessage(hlist,LVM_INSERTCOLUMN,0,(LPARAM)&lvc);
    lvc.cx = rcl.right-120;
    lvc.pszText = "Message";
    SendMessage(hlist,LVM_INSERTCOLUMN,1,(LPARAM)&lvc);

    return 0;
}

int getNoDefn(string sdef)
{
    int len = sdef.length();
    string stmp;
    for(int i=0; i<len; i++)
    {
        if(isdigit(sdef[i]))stmp.push_back(sdef[i]);
    }

    return atoi(stmp.c_str());
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    InitCommonControls();
    InitializeCriticalSection(&cs);

    memset(&wc,0,sizeof(WNDCLASSEX));
    hInst = hInstance;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_MYICON));
    wc.hCursor = LoadCursor(NULL,IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MYMENU);
    wc.lpszClassName = "CovirWClass";
    wc.hIconSm = (HICON)LoadImage(GetModuleHandle(NULL),
                                  MAKEINTRESOURCE(IDI_MYICON),IMAGE_ICON,16,16,0);

    if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL,"Window registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
        return 0;
    }

    hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,
                          "CovirWClass",
                          "Covir",
                          WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          400,400,
                          NULL,NULL,hInstance,NULL);

    if(hwnd == NULL)
    {
        MessageBox(NULL,"Window CREATION Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
        return 0;
    }
    g_hwnd = hwnd;
    gSys = new cSystem(hwnd);
    gSys->start();
    dects = new dectObj();



    //initialize the gui
    initGui();
    SendMessage(hwnd,WM_GUI_INIT_READY,0,0);

    ShowWindow(hwnd,nCmdShow);
    UpdateWindow(hwnd);

    while(GetMessage(&Msg,NULL,0,0))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    return Msg.wParam;
}
