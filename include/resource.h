#ifndef IDC_STATIC
#define IDC_STATIC (-1)
#endif
/*100-199*/
#define IDR_MYMENU 101
#define IDR_LVIEW 102
#define IDD_PROGRESS 103

/*200-299*/
#define IDI_MYICON 201


#define ID_FILE_CHECK 0
#define ID_FILE_REPAIR 2
#define ID_FILE_CANCEL 1
#define ID_FILE_HELP 3

#define IDI_LIB_ADD 2001
#define IDI_CLGS 2010

#define IDD_ABOUT 3000
#define ID_HELP_ABOUT 3001

#define IDC_MAIN_TOOL 5000
#define	IDM_CHECK 8000
#define IDM_CANCEL 8001
#define IDM_REPAIR 8002
#define IDM_HELP 8003
#define IDM_EXIT 8004

#define IDM_LOAD_DEF 8010
#define IDM_LOAD_REP 8011

#define IDL_LOG_OPEN 8020
#define IDL_CLGS 8021

#define IDB_BMP 2001

#define WM_COUNT                (WM_USER+0)
#define WM_START_READ           (WM_USER+1)
#define WM_READ_READY           (WM_USER+2)
#define WM_INIT_READY           (WM_USER+3)
#define WM_SYS_INIT_READY       (WM_USER+4)
#define WM_OPEN_DEF_FILE        (WM_USER+5)
#define WM_INCREMENT_WD         (WM_USER+6)
#define WM_DETECTION            (WM_USER+7)
#define WM_GUI_INIT_READY       (WM_USER+8)
#define WM_ENGINE_DONE          (WM_USER+9)
#define WM_FILE_DEL             (WM_USER+10)
#define WM_PROC_KILLED          (WM_USER+11)
#define WM_REGS_REMOVED         (WM_USER+12)
#define WM_SYSTEM_MSG           (WM_USER+13)
#define WM_PAYLOAD_DONE         (WM_USER+14)
#define WM_ENGINE_INIT_READY    (WM_USER+15)

#define ADD_MSG 0x0
#define _NO_ERROR 0x0
#define _ERROR 0x1

#define BMP_CHECK 0
#define BMP_CANCEL 1
#define BMP_REPAIR 2
#define BMP_HELP 3

#define IDS_NEW 3001
#define IDC_MAIN_STATUS 4000

/*System Messages*/
#define MSG_SYS_INI_CO  "Unexpected error: CoInitializeEx() failed with 0x%08"      //0
#define MSG_SR_NP       "System Restore is not present"                             //1
#define MSG_SR_FF       "Failed to find required function for System Restore."      //2
#define MSG_SR_OFF      "System Restore is turned off."                             //3
#define MSG_SR_FAIL     "Failed to create a restore point."                         //4
#define MSG_SR_SUCC     "Restore point created Successfully."                       //5

/*Local Messages*/
#define MSG_LM_REP_START        "Starting Repair Operation..."
#define MSG_LM_REP_ABORTED      "Repair Operation Aborted!"
#define MSG_LM_CHECK_START      "Checking Started..."
#define MSG_LM_CHECK_ABORTED    "Checking Aborted!"
#define MSG_LM_CHECK_END_OK     "Checking Engine Ended Succesfully."
#define MSG_LM_CHECK_END_ERR    "Checking Encountered some Errors!"
#define MSG_LM_REP_END_OK       "Repair Engine Ended Successfully."
#define MSG_LM_REP_END_ERR      "Repair Engine Encountered Some Errors!"
