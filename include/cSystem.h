#ifndef CSYSTEM_H
#define CSYSTEM_H

#include <string>
#include <iostream>
#include <shlobj.h>
#include <accctrl.h>
#include <aclapi.h>
#include <objbase.h>
#include "srrestoreptapi.h"
#include "helper.h"
#include "resource.h"
#include <process.h>
#include <Tlhelp32.h>
#include <winbase.h>
#include <windows.h>
#include <commctrl.h>
#include <process.h>
#include <winioctl.h>                   // Windows NT IOCTL codes

typedef BOOL (WINAPI *PFN_SETRESTOREPTW) (PRESTOREPOINTINFOW, PSTATEMGRSTATUS);

#define MAX_DRVS 260
#define MAX_COMMON_DIRS NO_OF_COMMON_DIRS64
#define NO_OF_COMMON_DIRS 8
#define NO_OF_COMMON_DIRS64 10

#define IS_64 FALSE

/* default folders */
#define SYSTEM 0
#define WINDOWS 1
#define TEMP 2
#define PROGRAMS 3
#define DOCUMENTS 4
#define DOWNLOADS 5
#define APPDATA 6
#define DESKTOP 7
#define SYSTEM64 8
#define PROGRAM64 9

#define MAX_NO_FILES 1000
#define MAX_NO_REGS 500

/*systems build versions*/
#define WIN_81 "6.3"
#define WIN_SV_12 "6.2"
#define WIN_7 "6.1"
#define WIN_VS "6.0"
#define WIN_XP_64 "5.2"
#define WIN_XP "5.1"
#define WIN_2K "5.0"

#define HKCR "HKEY_CLASSES_ROOT"
#define HKCU "HKEY_CURRENT_USER"
#define HKLM "HKEY_LOCAL_MACHINE"
#define HKUS "HKEY_USERS"
#define HKCC "HKEY_CURRENT_CONFIG"

#define KEY_WOW64_32KEY (0x0200)
#define KEY_WOW64_64KEY (0x0100)

#undef UNICODE
using namespace std;
struct Drivers{
    int nDrv;
    std::string *name;
};

struct fields
{
    vector<int>index;
};
//TODO
/*
change regObj to KeyObject and keyObject to valueObject
KeyObject will hold the whole key path ie HKMU/KEY1/KEY2
it will also have a vector of valueObject which is a struct of value name and its contents
*/
typedef vector<string> vstring;

struct valueObject
{
    string valueName;
    string value;
};

struct keyObj
{
    int handle; //holds the open HKEY index number
    string keyPath;
    vector<valueObject>values;
};

class defObj{
    string dectName; //this will appear on the list as the name of detection
    vstring sFolders;
    vstring sFiles;
    vstring  sReg;
    bool isPresent; //flag to notify us if detection is true
public:
    defObj();
    ~defObj();

    void addName(string);
    void addFolder(string);
    void addFile(string);
    void addReg(string);

    long getFileNumber();
    long getFolderNumber();
    long getRegsNumber();
    char *getDectName();

    int getFileList(vstring *);
    int getMiscFolderList(char**);
    int getRegList(char**);
    int processReg(vector<keyObj>*);
    fields exist(const char*,int);
    fields insertRegObj(keyObj*,vector<keyObj>*);
    std::string description;
    vector<keyObj> *mregs;


};
struct sysInfo
{
    char userName[164];
    char root[MAX_PATH];
    Drivers *drivers;
    bool isSixFour;

};

class cSystem:public cThread
{
    public:
        cSystem();
        cSystem(HWND);
        ~cSystem();
        char *getCommonDir(int, char **);
        int getDrives(Drivers*);
        int getCommonDirList(char**);
        char *getUserName(char*);
        char *getSysVer(char *);
        char *getRoot(char*);
        bool isValid(int,char *);
        void addTable(vstring);
        DWORD GetDriveFormFactor(int);
        bool is64();
        HWND getHwnd();
        int nCommonDirs();
        virtual DWORD run();
    protected:
    private:
        char root[3];
        Drivers sydrvs[25];
        bool isSixFour;
        char userName[164];
        char sysVer[32];
        volatile HWND gHwnd;
        bool IsWow64();
        vector <vstring>tables;
 };

#endif // CSYSTEM_H
