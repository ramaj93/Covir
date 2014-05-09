#ifndef READER_H
#define READER_H

#include <windows.h>
#include <fstream>
#include <iostream>
#include "helper.h"
#include "cSystem.h"

using namespace std;


#define DEF_DEF_FNAME "defn.defn"
#define NO_IGNORELN 4

using namespace std;


enum _flags
{
    INTERRUPT,
    F_IGNORE,
    CLEAR
};
enum _status{
    IGNOREL,
    DEFN_HEADER,
    PARS_FOLDER,
    PARS_DSCR,
    STRING_TABLE,
    PARS_FILE,
    PARS_REG
};

class Reader:public cThread{
    defObj* defns;
    int nDefn;
    int lnIndex,defIndex;
    string Buf;
public:
    Reader(defObj*,int);
    ~Reader();
    int getTotalWork();
    virtual DWORD run();
    string ver,owner,date;
};
#endif // READER_H
