#include "defReader.h"

void defObj::addName(string name)
{
    dectName = name;
}

void defObj::addFolder(string fold)
{
    sFolders.push_back(fold);
}

void defObj::addFile(string file)
{
    sFiles.push_back(file);
}

void defObj::addReg(string regs)
{
    sReg.push_back(regs);
}

Reader::Reader(defObj *defs,int siz)
{
    defns = defs;
    nDefn = siz;
    lnIndex = 1;
    defIndex = 0;
}

DWORD WINAPI Reader::flood()
{
    fstream file;
    file.open(defnFname,ios::in);
    if(file.is_open()&&file.good())
    {
        while(!file.eof())
        {
            getline(file,Buf);
            if(lnIndex==1||lnIndex==2)
            {
                status = DEFN_HEADER;
                lnIndex++;
            }
            else
            {
                char ch = Buf.at(0);
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
                        status = PARS_FOLDER;
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
                    }
                    break;
                    default:
                        switch(status)
                        {
                        case PARS_FOLDER:
                            (defns+defIndex)->addFolder(Buf);
                            break;
                        case PARS_FILE:
                            (defns+defIndex)->addFile(Buf);
                            break;
                        case PARS_REG:
                            (defns+defIndex)->addReg(Buf);
                            break;
                        }
                        break
                }
                lnIndex++;
            }
        }
    }
    else return -1;

    return defIndex;
}
