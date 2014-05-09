#include "cSystem.h"
#include <cstring>

extern cSystem *gSys;

cSystem::cSystem()
{

}
cSystem::cSystem(HWND hwnd)
{
    gHwnd = hwnd;
}

cSystem::~cSystem()
{

}
HWND cSystem::getHwnd()
{
    return gHwnd;
}
void cSystem::addTable(vstring Table)
{
    tables.push_back(Table);
}

bool cSystem::isValid(int path,char *File)
{
    int tSz = tables.size();
    if(path <  tSz)
    {
        unsigned int total = tables[path].size();
        helper mHelper;
        for(unsigned int i = 0;i<total; i++)
        {
            //TODO
            /*Some Files are renamed to different case
            So all should be initiated to a single case names
            eg EXPLORER.exe and explorer.exe*/
            const char *file = tables[path][i].c_str();
            //string file  = tables[path][i];
            if(strcmp(file,mHelper.toLower(File)) == 0)
            {
               return true;
            }
        }
    }
    return false;
}
char *cSystem::getCommonDir(int dirn,char **Buf)
{
    //memset(*Buf,0,MAX_PATH);
    char tmp[MAX_PATH];
    switch(dirn)
    {
    case SYSTEM:
        //SHGetFolderPath(NULL,CSIDL_SYSTEM, NULL,0, Buf);
        GetSystemDirectory(*Buf,MAX_PATH);
        strncat(*Buf,"\\",1);
        break;
    case WINDOWS:
        GetWindowsDirectory(*Buf,MAX_PATH);
        //SHGetFolderPath(NULL,CSIDL_WINDOWS, NULL,0,Buf);
        strncat(*Buf,"\\",1);
        break;
    case DOCUMENTS:
        SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL,0, *Buf);
        strncat(*Buf,"\\",1);
        break;
    case TEMP:
        GetTempPath(MAX_PATH,*Buf);
        break;
    case DOWNLOADS:
        SHGetFolderPath(NULL, CSIDL_PROFILE, NULL,0, *Buf);
        strncat(*Buf,"\\Downloads\\",11);
        break;
    case APPDATA:
        SHGetFolderPath(NULL, CSIDL_APPDATA, NULL,0, *Buf);
        strncat(*Buf,"\\",1);
        break;
    case PROGRAMS:
        SHGetFolderPath(NULL, CSIDL_PROGRAM_FILESX86, NULL,0, *Buf);
        strncat(*Buf,"\\",1);
        break;
    case PROGRAM64:
        SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL,0, *Buf);
        strncat(*Buf,"\\",1);
        break;
    case SYSTEM64:
        SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL,0, *Buf);
        strncat(*Buf,"\\",1);
        break;
    case DESKTOP:
        SHGetFolderPath(NULL,CSIDL_DESKTOPDIRECTORY, NULL,0, *Buf);
        strncat(*Buf,"\\",1);
        break;

    }
    return *Buf;
}

int cSystem::getDrives(Drivers *dinfo=0)
{
    char buf[MAX_DRVS];
    int len = GetLogicalDriveStrings(MAX_DRVS,buf);
    int idrv = len/4;
    if(dinfo==NULL)dinfo = sydrvs;
    dinfo->nDrv = idrv;
    dinfo->name = new std::string[idrv];
    int icount = 0;
    int irem=0;
    for(int i=0; i<idrv; i++)
    {
        for(int j=icount; j<len; j++)
        {
            char ch = buf[j];
            if(ch!='\00')
            {
                dinfo->name[i].push_back(buf[j]);
            }
            else
            {
                icount = j+1;
                irem++;
                if(irem==1)
                {
                    irem=0;
                    break;
                }
            }
        }
    }
    return idrv;
}

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

LPFN_ISWOW64PROCESS fnIsWow64Process;

bool cSystem::IsWow64()
{
    BOOL bIsWow64 = FALSE;

    fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
                           GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

    if(NULL != fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
        {
            //handle error
        }
    }
    isSixFour = bIsWow64;
    return bIsWow64;
}

bool cSystem::is64()
{
    return isSixFour;
}
int cSystem::nCommonDirs()
{
    if(isSixFour)return NO_OF_COMMON_DIRS64;
    else return NO_OF_COMMON_DIRS;
}
char *cSystem::getSysVer(char *Buf)
{
    OSVERSIONINFO osvi;
    memset(&osvi,0,sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);
    wsprintf(Buf,"%d.%d",osvi.dwMajorVersion,osvi.dwMinorVersion);
    return Buf;
}
int cSystem::getCommonDirList(char *Buf[MAX_COMMON_DIRS])
{
    int iTotal;
    if(isSixFour)
    {
        iTotal = NO_OF_COMMON_DIRS64;
    }
    else
    {
        iTotal = NO_OF_COMMON_DIRS;
    }
    for(int i=0; i<iTotal; i++)
    {
        char *bf;
        bf = (char*)malloc(MAX_PATH*sizeof(char));
        memset(bf,0,MAX_PATH);
        getCommonDir(i,&bf);
        Buf[i] = (char*)malloc(strlen(bf)*sizeof(char));
        strcpy(Buf[i],bf);
        free(bf);
    }
    return iTotal;
}

char *cSystem::getUserName(char *buf)
{
    return userName;
}

char *cSystem::getRoot(char *Buf)
{
    char *tmp;
    tmp = (char*)malloc(MAX_PATH*sizeof(char));
    getCommonDir(SYSTEM,&tmp);
    Buf = strtok(tmp,"W");
    return tmp;
}
DWORD cSystem::run()
{
    //get common needed items
    IsWow64();
    DWORD sz=164;
    GetUserName(userName,&sz);
    getDrives(sydrvs);
    getSysVer(sysVer);
    getRoot(root);

    PostMessage(gSys->gHwnd,WM_USER+4,0,0); //send done message
}

defObj::defObj()
{


}

defObj::~defObj()
{
    sFolders.clear();
    sFiles.clear();
    sReg.clear();
}
void defObj::addName(string name)
{
    dectName = name;
}
long defObj::getFileNumber()
{
    return sFiles.size();
}
long defObj::getFolderNumber()
{
    return sFolders.size();
}
long defObj::getRegsNumber()
{
    return sReg.size();
}

void defObj::addFolder(string fold)
{
    if(sFolders.size()==0)
    {
        sFolders.resize(MAX_NO_DEFNS);
        sFolders.clear();
    }
    sFolders.push_back(fold);
}

void defObj::addFile(string file)
{
    if(sFiles.size()==0)
    {
        sFiles.resize(MAX_NO_DEFNS);
        sFiles.clear();
    }
    sFiles.push_back(file);
}

void defObj::addReg(string regs)
{
    if(sReg.size()==0)
    {
        sReg.resize(MAX_NO_DEFNS);
        sReg.clear();
    }
    sReg.push_back(regs);
}

char *defObj::getDectName()
{
    char tmp[128];
    strcpy(tmp,dectName.c_str());
    return tmp;
}


int defObj::getFileList(vstring *Buf)
{
    //WaitForSingleObject(gHmutex,INFINITE);
    int siz = sFiles.size();
    Buf->resize(siz);
    Buf->clear();
    for(int i=0; i<siz; i++)
    {
        int len = sFiles.at(i).length();
        Buf->push_back(sFiles.at(i).c_str());
    }
    //ReleaseMutex(gHmutex);
    return siz;
}

int defObj::getRegList(char **Buf)
{

    int sz = sReg.size();
    for(int i=0; i<sz; i++)
    {
        Buf[i] = (char*)malloc(sReg.at(i).length()*sizeof(char));
        strncpy(Buf[i],sReg.at(i).c_str(),sReg.at(i).length());
    }
    return sz;
}
fields defObj::exist(const char *what,int opt)
{
    fields tmpf;
    switch(opt)
    {
    case 0:
        for(int i=0; i<sFiles.size(); i++)
        {
            if(strcmp(what,sFiles.at(i).c_str())==0)return tmpf;
        }
        break;
    case 1:
        for(int i=0; i<sFolders.size(); i++)
        {
            if(strcmp(what,sFolders.at(i).c_str())==0)return tmpf;
        }
        break;
    case 2:
        break;
    }
    tmpf.index.push_back(-1);
    tmpf.index.push_back(0);
    return tmpf;
}

/*
 *The number of mregs may not equal regs it could be less
 *mregs correspond to number of handles detected
 *handles might contain keys that may the same to other handles
 *Since there are 5 open root registry handles the number of mregs should be five exactly
 *indexed by rootHandle as
 *0-HKCR
 *1-HKCU
 *2-HKLM
 *3-HKUS
 *4-HKCC
 *construct a tmp obj and call this method
*/
fields defObj::insertRegObj(keyObj *regobj,vector<keyObj> *mregs)
{
    fields fld;
    fld.index.push_back(0);
    fld.index.push_back(0);
    int regSz = mregs->size();
    bool keyExist = false;

    if(regSz > 0) //our root contains something
    {

        for(int keyIndex = 0; keyIndex<regSz; keyIndex++) //for each key in root
        {
            /*
            compare the path name with tmp object path name
            if they match check for value sizes if they match exit else
            add the missing values
            */
            if(regobj->keyPath.compare(mregs->at(keyIndex).keyPath) == 0) //matched keyPath
            {
                //the key already exist in root
                int rootValueSz = mregs->at(keyIndex).values.size();
                int tmpValueSz = regobj->values.size();
                bool valueExist = false;
                keyExist = true;
                for(int tValIndex = 0; tValIndex<tmpValueSz; tValIndex++)
                {
                    for(int rVAlIndex = 0; rVAlIndex<rootValueSz; rVAlIndex++)
                    {
                        if(regobj->values[tValIndex].valueName.compare(mregs->at(keyIndex).values[rVAlIndex].valueName) == 0)
                        {
                            valueExist = true; //the value exist and probably updated
                            if(regobj->values[tValIndex].value.length() != mregs->at(keyIndex).values[rVAlIndex].value.length()) //the current comparison criteria is string sizes
                                mregs->at(keyIndex).values[rVAlIndex].value = regobj->values[tValIndex].value; //add/change the key if the above failed.
                            break;           //and then break
                        }
                    }
                    if(!valueExist) //the value doesnt exist ...
                    {
                        mregs->at(keyIndex).values.push_back(regobj->values[tValIndex]);
                        fld.index[1]++; //increment our number of values in the field
                    }
                }
            }
        }
        if(!keyExist)
            {
                //the key doesn't exist so just push the whole object
                mregs->push_back(*regobj);
                fld.index[0]++; //increment our number of values in the field
            }
    }
    else //the root is empty
    {
        mregs->push_back(*regobj); //so just push this tmp object wholey
    }

    return fld;
}

/*TODO&&DESCR
The model have changed there is no need for a rootHandle index, which is now been replaced by the complete registry key path
the processReg will receive the *keyObj pointer from the engine to work with ie fill it
The registry entry in the defns will look like HKMU/KEY1/KEY2:val1 = Val1Contents:val2 =  Val2Contents
the function should extract the keyPath as delimited by ':' in the first pass
if the return val of strtok is !NULL then continue to extract each time
the values contains a separator = which is used to separate the two value entities
*/
int defObj::processReg(vector<keyObj> *rg)
{
    int regEntrySz = sReg.size();
    char *result = NULL;
    int index = 0,rootHandle = 0;
    int nRegHandles = 0;
    keyObj tmp;
    helper mhelper;

    for(int i=0; i<regEntrySz; i++)
    {
        result = strtok( (char*)sReg.at(i).c_str(), ":" );
        //first case get the root key of this key
        if(result)
        {
            int handleSz = 0;

            if((strncmp(HKCR,result,strlen(HKCR)))==0)
            {
                handleSz = strlen(HKCR);
                rootHandle = 0;
                tmp.handle = rootHandle;

            }
            else if((strncmp(HKCU,result,strlen(HKCU)))==0)
            {
                handleSz = strlen(HKCU);
                rootHandle = 1;
                tmp.handle = rootHandle;
            }
            else if((strncmp(HKLM,result,strlen(HKLM)))==0)
            {
                handleSz = strlen(HKLM);
                rootHandle = 2;
                tmp.handle = rootHandle;
            }
            else if((strncmp(HKUS,result,strlen(HKUS)))==0)
            {
                handleSz = strlen(HKUS);
                rootHandle = 3;
                tmp.handle = rootHandle;
            }

            char *keyPath;
            keyPath = (char*)malloc(MAX_PATH*sizeof(char));
            memset(keyPath,0,MAX_PATH);

            mhelper.strcpyn(keyPath,result,handleSz);
            if(strlen(keyPath)!=0)
            {
                tmp.keyPath = keyPath;
            }
            index++;
        }

        valueObject obj;
        while( result = strtok( NULL, ":" ))
        {
            char *valName = NULL;
            valName = strtok(result,"=");
            if(valName != NULL)obj.valueName = valName;
            valName = NULL;
            valName = strtok(NULL,"=");
            if(valName!=NULL)obj.value = valName;

            tmp.values.push_back(obj); //we dont expect more values from this key
            index = 0;
        }

        fields fld;
        //when result is null we are done with this line and tmp construction so just insert it
        fld = insertRegObj(&tmp,rg);
        nRegHandles += fld.index[0]*fld.index[1];
        tmp.values.clear();
        //memset(result,0,strlen(result)); uncommenting this line brings an error
    }
    free(result);
    return nRegHandles;
}


int defObj::getMiscFolderList(char **Buf)
{
    long nf = sFolders.size();
    realloc(Buf,nf);
    for(int i=0; i<nf; i++)
    {
        Buf[i] = (char*)malloc((sFolders.at(i).length())*sizeof(char));
        memset(Buf[i],0,sFolders.at(i).length());
        strcpy(Buf[i],sFolders.at(i).c_str());
    }
    return nf;
}

/*
   The <vwin32.h> header file is part of the Windows 95 DDK.
   If you don't have the Windows 95 DDK, then use the definitions
   below.
*/

#ifndef VWIN32_DIOC_DOS_IOCTL
#define VWIN32_DIOC_DOS_IOCTL 1

typedef struct _DIOC_REGISTERS
{
    DWORD reg_EBX;
    DWORD reg_EDX;
    DWORD reg_ECX;
    DWORD reg_EAX;
    DWORD reg_EDI;
    DWORD reg_ESI;
    DWORD reg_Flags;
} DIOC_REGISTERS, *PDIOC_REGISTERS;

#endif

// Intel x86 processor status fla
#define CARRY_FLAG  0x1


#pragma pack(1)
typedef struct _DOSDPB
{
    BYTE    specialFunc;    //
    BYTE    devType;        //
    WORD    devAttr;        //
    WORD    cCyl;           // number of cylinders
    BYTE    mediaType;      //
    WORD    cbSec;          // Bytes per sector
    BYTE    secPerClus;     // Sectors per cluster
    WORD    cSecRes;        // Reserved sectors
    BYTE    cFAT;           // FATs
    WORD    cDir;           // Root Directory Entries
    WORD    cSec;           // Total number of sectors in image
    BYTE    bMedia;         // Media descriptor
    WORD    secPerFAT;      // Sectors per FAT
    WORD    secPerTrack;    // Sectors per track
    WORD    cHead;          // Heads
    DWORD   cSecHidden;     // Hidden sectors
    DWORD   cTotalSectors;  // Total sectors, if cbSec is zero
    BYTE    reserved[6];    //
} DOSDPB, *PDOSDPB;
#pragma pack()


/*
   GetDriveFormFactor returns the drive form factor.

   It returns 350 if the drive is a 3.5" floppy drive.
   It returns 525 if the drive is a 5.25" floppy drive.
   It returns 800 if the drive is a 8" floppy drive.
   It returns   0 on error.
   It returns   1 if the drive is a hard drive.
   It returns   2 if the drive is something else.

   iDrive is 1 for drive A:, 2 for drive B:, etc.

   Note that iDrive is not range-checked or otherwise validated.
   It is the responsibility of the caller only to pass values for
   iDrive between 1 and 26 (inclusive).

   Although the function, as written, returns only the drive form
   factor, it can easily be extended to return other types of
   drive information.
*/

DWORD   cSystem::GetDriveFormFactor(int iDrive)
{
    HANDLE h;
    TCHAR tsz[8];
    DWORD dwRc;

    if ((int)GetVersion() < 0)
    {
        // Windows 95

        /*
           On Windows 95, use the technique described in
           MSDN under "Windows 95 Guide to Programming", "Using Windows 95 features", "Using VWIN32 to Carry Out MS-DOS Functions".
        */
        h = CreateFileA("\\\\.\\VWIN32", 0, 0, 0, 0,
                        FILE_FLAG_DELETE_ON_CLOSE, 0);

        if (h != INVALID_HANDLE_VALUE)
        {
            DWORD          cb;
            DIOC_REGISTERS reg;
            DOSDPB         dpb;

            dpb.specialFunc = 0;  // return default type; do not hit disk

            reg.reg_EBX   = iDrive;       // BL = drive number (1-based)
            reg.reg_EDX   = (DWORD)&dpb;  // DS:EDX -> DPB
            reg.reg_ECX   = 0x0860;       // CX = Get DPB
            reg.reg_EAX   = 0x440D;       // AX = Ioctl
            reg.reg_Flags = CARRY_FLAG;   // assume failure

            // Make sure both DeviceIoControl and Int 21h succeeded.
            if (DeviceIoControl (h, VWIN32_DIOC_DOS_IOCTL, &reg,
                                 sizeof(reg), &reg, sizeof(reg),
                                 &cb, 0)
                    && !(reg.reg_Flags & CARRY_FLAG))
            {
                switch (dpb.devType)
                {
                case 0: // 5.25 360K floppy
                case 1: // 5.25 1.2MB floppy
                    dwRc = 525;
                    break;

                case 2: // 3.5  720K floppy
                case 7: // 3.5  1.44MB floppy
                case 9: // 3.5  2.88MB floppy
                    dwRc = 350;
                    break;

                case 3: // 8" low-density floppy
                case 4: // 8" high-density floppy
                    dwRc = 800;
                    break;

                case 5: // hard drive
                    dwRc = 1;
                    break;

                case 6: // tape drive
                case 8: // optical disk
                    dwRc = 2;
                    break;

                default: // unknown
                    dwRc = 0;
                    break;
                }
            }
            else
                dwRc = 0;


            CloseHandle(h);
        }
        else
            dwRc = 0;

    }
    else
    {
        // Windows NT

        /*
           On Windows NT, use the technique described in the Knowledge
           Base article Q115828 and in the "FLOPPY" SDK sample.
        */
        wsprintf(tsz, TEXT("\\\\.\\%c:"), TEXT('@') + iDrive);
        h = CreateFile(tsz, 0, FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
        if (h != INVALID_HANDLE_VALUE)
        {
            DISK_GEOMETRY Geom[20];
            DWORD cb;

            if (DeviceIoControl (h, IOCTL_DISK_GET_MEDIA_TYPES, 0, 0,
                                 Geom, sizeof(Geom), &cb, 0)
                    && cb > 0)
            {
                switch (Geom[0].MediaType)
                {
                case F5_1Pt2_512: // 5.25 1.2MB floppy
                case F5_360_512:  // 5.25 360K  floppy
                case F5_320_512:  // 5.25 320K  floppy
                case F5_320_1024: // 5.25 320K  floppy
                case F5_180_512:  // 5.25 180K  floppy
                case F5_160_512:  // 5.25 160K  floppy
                    dwRc = 525;
                    break;

                case F3_1Pt44_512: // 3.5 1.44MB floppy
                case F3_2Pt88_512: // 3.5 2.88MB floppy
                case F3_20Pt8_512: // 3.5 20.8MB floppy
                case F3_720_512:   // 3.5 720K   floppy
                    dwRc = 350;
                    break;

                case RemovableMedia:
                    dwRc = 2;
                    break;

                case FixedMedia:
                    dwRc = 1;
                    break;

                default:
                    dwRc = 0;
                    break;
                }
            }
            else
                dwRc = 0;

            CloseHandle(h);
        }
        else
            dwRc = 0;
    }

    /*
       If you are unable to determine the drive type via ioctls,
       then it means one of the following:

       1.  It is hard drive and we do not have administrator privilege
       2.  It is a network drive
       3.  The drive letter is invalid

       GetDriveType can distinguish these three cases.
    */
    if (dwRc == 0)
    {
        wsprintf(tsz, TEXT("%c:\\"), TEXT('@') + iDrive);

        switch (GetDriveType(tsz))
        {
        case DRIVE_FIXED:
            dwRc = 1;
            break;
        case DRIVE_REMOVABLE:
        case DRIVE_REMOTE:
        case DRIVE_CDROM:
        case DRIVE_RAMDISK:
            dwRc = 2;
            break;
        default:
            dwRc = 0;
        }
    }
    return dwRc;
}



