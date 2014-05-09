#include "cEngine.h"

extern cSystem *gSys;
extern CRITICAL_SECTION cs;
extern dectObj *dects;
extern HANDLE gHdefs;

cEngine::cEngine(defObj *defs,int nd)
{
    threadIndex = 0;
    threadDone  = 0;
    this->defs = defs;
    ndef = nd;
    nfid = 0;
    hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
    hDataReady = CreateEvent(NULL,FALSE,FALSE,NULL);
    hMdect = CreateMutex(NULL,FALSE,NULL);
    hMdefGfl = CreateMutex(NULL,FALSE,NULL);
    hTDone = CreateMutex(NULL,FALSE,NULL);
    files.resize(MAX_NO_FILES);
    files.clear();
}

cEngine::~cEngine()
{
    delete regs;
    delete defFolders;
}

void cEngine::fillDefFolders()
{
    nCommonFolders = gSys->nCommonDirs();
    gSys->getCommonDirList(defFolders);
    return;
}

void cEngine::addFiles(vstring *Buf, int n)
{
    for(int i=0; i<n; i++)
    {
        files.push_back(Buf->at(i));
    }
}

void cEngine::fillFiles()
{
    unsigned int sz=0,ic=0;
    vstring buf;
    WaitForSingleObject(gHdefs,INFINITE);
    for(unsigned int i=0; i<ndef; i++)
    {
        ic = defs[i].getFileList(&buf);
        addFiles(&buf,ic);
        sz+=ic;
    }
    ReleaseMutex(gHdefs);
    nFiles = sz;
}

void cEngine::fillRegs()
{

    unsigned int ndf = ndef;
    regWork = 0;
    WaitForSingleObject(gHdefs,INFINITE);
    for(unsigned int i=0; i<ndf; i++)
    {
        regWork += defs[i].processReg(&regVector);
    }
    ReleaseMutex(gHdefs);
}
void cEngine::addSlashes(char *Buf)
{
    string tmp = Buf;
    for(unsigned int i=0; i<tmp.length(); i++)
    {
        if(tmp[i]=='\\')
        {
            tmp.insert(i,"\\");
            i++;
        }
    }
    memset(Buf,0,strlen(Buf));
    strcpy(Buf,tmp.c_str());
}

void cEngine::_switch()
{
    WaitForSingleObject(hEvent,INFINITE);
    threadIndex++;
    ResetEvent(hEvent);
}

DWORD cEngine::run()
{

    int th = threadIndex;
    SetEvent(hEvent);
    switch(th)
    {
    case 0:
    {
        //run this on the first instance only

        fillDefFolders();
        fillFiles();
        fillRegs();


        EnterCriticalSection(&cs);
        long nfl = nFiles;
        long ncfd = nCommonFolders;
        HWND lHwnd = gSys->getHwnd();
        int nlThd = threadIndex;
        Drivers drv;
        int nd = gSys->getDrives(&drv);
        LeaveCriticalSection(&cs);

        long totalWork = 0 ;
        totalWork = ncfd*nfl + ndef*nFiles*nd + regWork; //not so sure
        PostMessage(lHwnd,WM_ENGINE_INIT_READY,0,totalWork);
        SetEvent(hDataReady);

        WIN32_FIND_DATA fd;
        HANDLE ret;

        char tmp[MAX_PATH];
        memset(tmp,0,MAX_PATH);

        for(int i=0; i<nCommonFolders; i++)
        {
            SetCurrentDirectory(defFolders[i]);
            for(int j=0; j<nfl; j++)
            {
                strcpy(tmp,files.at(j).c_str());
                ret = FindFirstFile(tmp,&fd);
                if(ret !=INVALID_HANDLE_VALUE)
                {
                    if(!gSys->isValid(i,tmp))
                        {
                        char fBuf[MAX_PATH]= {0};
                        GetFullPathName(files.at(j).c_str(),MAX_PATH,fBuf,NULL);
                        WaitForSingleObject(hMdect,INFINITE);
                        dects->addFile(fBuf);
                        dects->addName((char*)files.at(j).c_str());
                        ReleaseMutex(hMdect);
                    }
                    else continue;

                }
                memset(tmp,0,MAX_PATH);
                memset(&fd,0,sizeof(fd));
                PostMessage(lHwnd,WM_INCREMENT_WD,0,0);
            }
        }
        int locth;
        WaitForSingleObject(hTDone,INFINITE);
        threadDone++;
        locth = threadDone;
        ReleaseMutex(hTDone);

        if(nlThd>0)nlThd = nlThd-1;
        if(locth>=3)
            PostMessage(lHwnd,WM_ENGINE_DONE,0,nlThd);
    }
    break;
    case 1:
    {
        //consider switching to other drivers
        WIN32_FIND_DATA fda;
        HANDLE hfff;

        EnterCriticalSection(&cs);
        long ndx  = ndef;
        int nlthid = threadIndex;
        HWND lHwnd = gSys->getHwnd();
        Drivers drv;
        int nd = gSys->getDrives(&drv);
        LeaveCriticalSection(&cs);



        WaitForSingleObject(hDataReady,INFINITE); //wait for data from 0 to be ready
        SetEvent(hDataReady);

        for(int i=0; i<ndx; i++)
        {
            //char *buf[MAX_PATH];
            //int nf;// = defs[i].getMiscFolderList(buf);
            vstring tmp;

            WaitForSingleObject(hMdefGfl,INFINITE);
            int nfl = defs[i].getFileList(&tmp);
            ReleaseMutex(hMdefGfl);
            char *root;
            root = gSys->getRoot(root);
            cSystem lSys;
            //perform open search on all drivers using filenames first
            for(int j = 0; j < nd; j++)
            {
                char driver[4] = {'\0'};
                wsprintf(driver,drv.name[j].c_str());
                if(strncmp(root,driver,strlen(root)) != 0){//jump the root we,ve already search it
                int driveType = (GetDriveType(driver));
                DWORD type = lSys.GetDriveFormFactor(j);
                if((driveType == DRIVE_REMOVABLE)&&(type>0)&&(type<350)||(driveType == DRIVE_FIXED))
                {
                        SetCurrentDirectory(driver);
                        for(int k=0; k<nfl; k++)
                        {
                            hfff = FindFirstFile(tmp.at(k).c_str(),&fda);
                            if(hfff!=INVALID_HANDLE_VALUE)
                            {
                                char fBuf[MAX_PATH];
                                GetFullPathName(tmp.at(k).c_str(),MAX_PATH,fBuf,NULL);
                                WaitForSingleObject(hMdect,INFINITE);
                                dects->addFile(fBuf);
                                dects->addName((char*)tmp.at(k).c_str());
                                ReleaseMutex(hMdect);
                            }
                            PostMessage(lHwnd,WM_INCREMENT_WD,0,0);
                        }
                    }
                    else continue;
                }
            }
        }
        int locth;
        WaitForSingleObject(hTDone,INFINITE);
        threadDone++;
        locth = threadDone;
        ReleaseMutex(hTDone);

        if(locth>=3)
            PostMessage(lHwnd,WM_ENGINE_DONE,0,nlthid);

    }
    break;
    case 2:
    {


        EnterCriticalSection(&cs);
        int nlthid = threadIndex;
        HWND lHwnd = gSys->getHwnd();
        bool is64 = gSys->is64();
        LeaveCriticalSection(&cs);

        HKEY key = NULL;
        DWORD rokRetVal = 0;
        WaitForSingleObject(hDataReady,INFINITE);
        int regKeySz = regVector.size();
        char *tmp;
        tmp = (char*)malloc(MAX_PATH*sizeof(char));
        bool someExist = false;

        for(int i = 0; i<regKeySz; i++)
        {
                wsprintf(tmp,regVector[i].keyPath.c_str());
                rokRetVal = -1;
                switch (regVector[i].handle)
                {
                case 0:
                    rokRetVal = RegOpenKey(HKEY_CLASSES_ROOT,tmp,&key);
                    break;
                case 1:
                    rokRetVal = RegOpenKey(HKEY_CURRENT_USER,tmp,&key);
                    break;
                case 2:
                    if(is64)
                        rokRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE,tmp,0,KEY_ALL_ACCESS|KEY_WOW64_64KEY,&key);
                    else rokRetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE,tmp,0,KEY_ALL_ACCESS,&key);
                    break;
                case 3:
                    rokRetVal = RegOpenKey(HKEY_USERS,tmp,&key);
                    break;
                case 4:
                    rokRetVal = RegOpenKey(HKEY_CURRENT_CONFIG,tmp,&key);
                    break;
                default:
                    break;
                }

                DWORD rqvRetVal = 0;
                int regKeyValueSz = regVector[i].values.size();
                char *keyTmp;
                if(rokRetVal == ERROR_SUCCESS)
                {
                    for(int j=0; j<regKeyValueSz; j++)
                    {
                        keyTmp = (char*)malloc(((strlen(regVector[i].values[j].valueName.c_str()))+1)*sizeof(char));
                        wsprintf(keyTmp,regVector[i].values[j].valueName.c_str());
                        rqvRetVal = RegQueryValueExA(key,keyTmp,NULL,NULL,NULL,NULL);
                        if(rqvRetVal != ERROR_SUCCESS)
                        {
                            std::vector<valueObject>::iterator it;
                            it = regVector[i].values.begin()+j;
                            regVector[i].values.erase(it); //if the value is not present pop it out
                        }
                        else someExist = true;
                        PostMessage(lHwnd,WM_INCREMENT_WD,0,0);
                    }
                }

            if(someExist) dects->addReg(regVector[i]);   //add this keyObj to our detection list since the key exist anyways
            }
        int locth;
        WaitForSingleObject(hTDone,INFINITE);
        threadDone++;
        locth = threadDone;
        ReleaseMutex(hTDone);

        if(locth>=3)
            PostMessage(lHwnd,WM_ENGINE_DONE,0,nlthid);

    }
    //do some registry work here
    break;
    default:
        threadIndex = 0; //we are out of range reset the index
        break;
    }
}

dectObj::dectObj()
{
    file_iter = 0;
    path_iter = 0;
}

bool dectObj::NameExist(char *name)
{
    for(int i=0; i<names.size(); i++)
    {
        if(strncmp(name,names.at(i).c_str(),strlen(name))==0)return true;
    }
    return false;
}

bool dectObj::FileExist(char *file)
{
    for(int i=0; i<Filepaths.size(); i++)
    {
        if(strncmp(file,Filepaths.at(i).c_str(),strlen(file))==0)return true;
    }
    return false;
}

void dectObj::addFile(char *fname)
{
    EnterCriticalSection(&cs);
    HWND lhwnd = gSys->getHwnd();
    LeaveCriticalSection(&cs);

    if(Filepaths.size()==0)
    {
        Filepaths.resize(MAX_NO_DEFNS);
        Filepaths.clear();
    }
    if(!FileExist(fname))
    {
        Filepaths.push_back(fname);
        PostMessage(lhwnd,WM_DETECTION,path_iter,0);
        path_iter++;
    }
}

void dectObj::addReg(keyObj reg)
{
    regS.push_back(reg);
}

void dectObj::addName(char name[MAX_PATH])
{
    EnterCriticalSection(&cs);
    HWND lhwnd = gSys->getHwnd();
    LeaveCriticalSection(&cs);

    if(names.size()==0)
    {
        names.resize(MAX_NO_DEFNS);
        names.clear();
    }
    if(!NameExist(name))
    {
        names.push_back(name);
        //PostMessage(lhwnd,WM_DETECTION,file_iter,0);
        file_iter++;
    }
}

int dectObj::getNoproc()
{
    return names.size();
}

int dectObj::getNofiles()
{
    return Filepaths.size();
}

int dectObj::getNoRegs()
{
    return regS.size();
}

const char *dectObj::NextFile(int i)
{
    char ret[MAX_PATH];
    strcpy(ret,Filepaths.at(i).c_str());
    return ret;
}

const char *dectObj::NextRegs(int i)
{
    char ret[MAX_PATH];
    //strcpy(ret,regStrings.at(i).c_str());
    return ret;
}

const char*dectObj::NextName(int i)
{
    static char ret[MAX_PATH];
    wsprintf(ret,names.at(i).c_str());
    return ret;
}
