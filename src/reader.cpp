#include "Reader.h"
#include "cSystem.h"

_status status;
_flags flag;
extern cSystem *gSys;
extern HANDLE gHmutex;
extern CRITICAL_SECTION cs;




Reader::Reader(defObj *defs,int siz)
{
    defns = defs;
    nDefn = siz;
    lnIndex = 1;
    defIndex = 0;
}

DWORD  Reader::run()
{
    EnterCriticalSection(&cs);
    HWND lhwnd = gSys->getHwnd();
    LeaveCriticalSection(&cs);

    vstring tmp;
    flag = CLEAR;
    fstream file;
    file.open(DEF_DEF_FNAME,ios::in);
    if(file.is_open()&&file.good())
    {
        while(!file.eof())
        {
            Buf.clear();
            getline(file,Buf);
            char ch = Buf[0];
            if(ch==' ')continue;
            if(lnIndex<=NO_IGNORELN)
            {
                switch(lnIndex)
                {
                case 1:
                    ver = Buf;
                    break;
                case 2:
                    break;
                case 3:
                    owner = Buf;
                    break;
                case 4:
                    date = Buf;
                    break;
                }
                status = DEFN_HEADER;
                lnIndex++;
            }
            else
            {
                switch(ch)
                {
                case '"':
                    if(status = DEFN_HEADER)(defns+defIndex)->addName(Buf);
                    break;
                case '}':
                    status = DEFN_HEADER;
                    defIndex++;
                    break;
                case '{':
                    if(status = DEFN_HEADER)
                    {
                        status = PARS_DSCR;
                    }
                    break;
                case ' ':
                    continue;
                    break;
                case '$':
                    if(Buf.compare(1,5,"FLAGS") == 0)
                    {
                        status = STRING_TABLE;
                    }
                    else if(status == STRING_TABLE)
                    {
                            gSys->addTable(tmp);
                            tmp.clear();
                            flag = CLEAR;
                    }
                    break;
                case ';':
                    switch(status)
                    {
                    case PARS_FOLDER:
                        status = PARS_FILE;
                        break;
                    case PARS_FILE:
                        status = PARS_REG;
                        break;
                    case PARS_DSCR:
                        status = PARS_FOLDER;
                        break;
                    }
                    break;
                default:
                    switch(status)
                    {
                    case PARS_FOLDER:
                        if(Buf.length()>0)(defns+defIndex)->addFolder(Buf);
                        break;
                    case PARS_FILE:
                        if(Buf.length()>0)(defns+defIndex)->addFile(Buf);
                        break;
                    case PARS_REG:
                        if(Buf.length()>0)(defns+defIndex)->addReg(Buf);
                        break;
                    case PARS_DSCR:
                        if(Buf.length()>0)(defns+defIndex)->description = Buf;;
                        break;
                    case STRING_TABLE:
                        if(flag != INTERRUPT)
                        {
                            tmp.push_back(Buf);
                        }
                        break;
                    }
                    break;
                }
                lnIndex++;
            }
        }
    }
    else return -1;
    PostMessage(lhwnd,WM_USER+2,0,defIndex); //post a message we are finished
    return defIndex;
}

Reader::~Reader()
{
    delete defns;
}

int Reader::getTotalWork()
{
    return defIndex*NO_OF_COMMON_DIRS;
}
