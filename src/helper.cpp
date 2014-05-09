#include "helper.h"

helper::helper()
{
    //ctor
}

helper::~helper()
{
    //dtor
}

int helper::getInt(char Buf[])
{
    return atoi(Buf);
}

char *helper::strcpyn(char *to, const char *from,int n)
{
    int sz = strlen(from);
    //to = (char*)(malloc((sz-n)*sizeof(char)));
    memset(to,0,(sz));
    for(int i = n+1;i<sz;i++)
    {
        char ch = from[i];
        strncat(to,&ch,1);
        ch = '\0';
    }
    strcat(to,"\0");
    return to;
}

char *helper::toLower(char *str)
{
    while(*str)
    {
        if((*str >= 'a')&&(*str <= 'z'))
            *str -= ('a' - 'A');
        str++;
    }
}
char *helper::toString(int num,char *Buf)
{
    memset(Buf,0,strlen(Buf));
    itoa(num,Buf,10);
    return Buf;
}
char *helper::appendInt(char one[],int n,char *Buf)
{
    char tmp[16];
    memset(tmp,0,strlen(tmp));
    memset(Buf,0,strlen(Buf));
    toString(n,tmp);
    strcpy(Buf,one);
    strncat(Buf,tmp,strlen(tmp));
    return Buf;
}

cThread::cThread()
{
    mhThread = NULL;
    mThreadId = 0;
    ret = 0;
}

void cThread::stop()
{
    //ExitThread(1);
    TerminateThread(mhThread,1);
}
void cThread::start()
{
    mhThread = CreateThread(NULL,0,threadFunc,(void*)this,0,&mThreadId);
}

void cThread::WaitForExit()
{
    if(NULL!=mhThread)WaitForSingleObject(mhThread,INFINITE);
    CloseHandle(mhThread);
}

DWORD cThread::getRet()
{
    DWORD dRet;
    GetExitCodeThread(mhThread,&dRet);
    return dRet;
}

DWORD WINAPI cThread::threadFunc(LPVOID param)
{
    cThread *pto = (cThread*)param;
    pto->ret = pto->run();
    return pto->ret;
}
DWORD cThread::run()
{
    printf("hello from main class");
    //your should reimplement this method
    //the output above shouldnt be displayed after subclassing
}

cThread::~cThread()
{
    CloseHandle(mhThread);
}
