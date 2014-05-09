#ifndef PAYLOADS_H
#define PAYLOADS_H

#include "cSystem.h"
#include "helper.h"
#include "cEngine.h"

class payLoads:public cThread
{
    public:
        payLoads();
        ~payLoads();
        virtual DWORD run();
        void _switch();
    protected:
    private:
        int killProcess(const char*);
        bool isExe(const char*);
        int threadIndx,threadDone;
        int createSystemRestore();
        long workDone;
        bool error;
        HANDLE hThid,hThDone,hthidEvent;
};

#endif // PAYLOADS_H
