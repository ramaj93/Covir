#ifndef CENGINE_H
#define CENGINE_H

#include "helper.h"
#include "cSystem.h"

/**
*--Engine theory--*
*defs object contain detection objects containing filenames,misc folder-names,and registry entries
*The engine should load default search folders from cSystem
*cSystem contains few default infos as default on its construction
*cEngine should build its default folder listing by calling the global cSystem object from main
*The engine will search for all detections in all the default folders then it'll search for file in
*miscellaneous folders
-1:There should be 3 threads one dealing with default folders and other with misc folders
-when the two threads are done the last thread will search for signature registry entries
-The threads are dispatched simultaneously by calling start each time
-DETECTIONS
-The engine doesn't optimize for now a detection of a single entity is enough to declare a malware
-A detection object contains collections of files and registries that are to be deleted
-A detection name can be derived later from name of files or processes
-To be on the safe side the engine will only delete values and not keys
*/


class dectObj
{
public:
    dectObj();
    ~dectObj(){}

    void addFile(char[MAX_PATH]);
    void addReg(keyObj);
    void addName(char[MAX_PATH]);
    const char*NextFile(int);
    const char*NextName(int);
    const char*NextRegs(int);
    int getNoproc();
    int getNofiles();
    int getNoRegs();
    bool NameExist(char*);
    bool FileExist(char*);
    vector<keyObj>regS;
private:
    int file_iter,path_iter;
    vstring names;
    vstring Filepaths;
    int processRegs();
};

class cEngine:public cThread
{
public:
    cEngine(defObj*,int);
    cEngine(){}
    ~cEngine();
    void _switch();
    vstring files;
    virtual DWORD run();
    vector<keyObj>regVector;
protected:
private:
    unsigned int workIndex,nCommonFolders,ndef,nfid;
    long nFiles,nRegs,regWork,fileWork;
    volatile int threadIndex,threadDone;
    defObj *defs; //pointer to our defs obj
    char *regs[MAX_NO_REGS];
    char *defFolders[MAX_COMMON_DIRS];


    void addSlashes(char*);
    void fillDefFolders();
    void fillFiles();
    void fillRegs();
    void addFiles(vstring*,int);
    HANDLE hEvent,hDataReady,hMdect,hMdefGfl,hTDone;
};

#endif // CENGINE_H

