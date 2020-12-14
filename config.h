#ifndef configcpp
#define configcpp
//single devic config
#define DETECT_PORT_WAIT_TIME 500
#define SERIAL_WRITE_WAIT 100
#define NEXT_GCODE_COMMAND_DELAY 200
#define RESEND_TRIES 0
#define DELAY_BETWEEN_COMMAND_AND_OTHER 500
#define DEFAULT_Command_No_RESPONSE_TIMEOUT 4000
#define DEVICE_MONITOR_TIMER 5000
#define BUSY_DURATION 7000
#define ACTION_WAITING_TIME 1000
#define SAVE_DEVICE_DATA_EACH 10000
#define WAIT_BEFORE_M600 10000

//general configs
#define REMOTE_SERVER_URL "http://localhost:8081/"//"http://185.247.117.33:8081/"
#define REMOTE_SERVER_ADMIN_NAME "3dPrinters"
#define REMOTE_SERVER_ADMIN_PASS "mM41634163Mm"
#define DEVICES_TABLE "Printers"
#define TASKS_TABLE "Tasks"

//ui config
#define UPDATE_DEVICES_GUI_TIMER 500

//device file system
#define PRINTERS_FOLDER_PATH "./printers"
#define DOWNLOAD_PATH "download/"
#define DEVICE_DATA_FILE "device.json"
#define LOCALE_GCODE_PATH "files/"
#define UPLOAD_SUFFIX "G"

//tasks config
#define TASK_MANAGER_TIMER 10000

#endif
