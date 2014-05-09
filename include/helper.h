#ifndef HELPER_H
#define HELPER_H

#include <stdio.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <windows.h>

#define MAX_LEN 128
#define MAX_NO_DEFNS 1000

class cThread
{
    public:
        DWORD ret;
        cThread();
        ~cThread();
        void start();
        static DWORD WINAPI threadFunc(LPVOID param);
        void WaitForExit();
        void stop();
        DWORD getRet();
    protected:
        virtual DWORD run();

        HANDLE mhThread;
        DWORD mThreadId;
};

class helper
{
    public:
        helper();
        ~helper();
        char *appendInt(char one[],int n,char*);
        char *toLower(char *);
        int getInt(char Buf[]);
        char *toString(int,char*);
        char *strcpyn(char*to, const char*from, int n);
    protected:
    private:
};

#endif // HELPER_H
