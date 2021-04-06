#ifndef configcpp
#define configcpp
//single devic config
#define DETECT_PORT_WAIT_TIME 500
#define SERIAL_WRITE_WAIT 100
#define RESEND_TRIES 0
#define DELAY_BETWEEN_COMMAND_AND_OTHER 500
#define DEVICE_MONITOR_TIMER 1000
#define DEVICE_MONITOR_PRINTING_STAT_TIMER 10000
#define DEVICE_MONITOR_TEMPERATURE_STAT_TIMER 10000
#define DEVICE_MONITOR_END_STOPS_STAT_TIMER 10000
#define BUSY_DURATION 15000
#define ACTION_WAITING_TIME 1000
#define SAVE_DEVICE_DATA_EACH 15000
#define WAIT_BEFORE_M600 30000
#define PRINT_CONTROLLER_TIMER 1000

//camera configs
#define CAPTURE_CAMERA_FILE_NAME "capture.jpg"
#define CAPTURE_CAMERA_UPLOAD_PATH "captures/%1.jpg"
#define CAPTURE_ALL_CAMERAS_TIMER 10000

//general configs
#define REMOTE_SERVER_URL "http://185.247.117.33:8081/"//"http://185.247.117.33:8081/"
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

//gcode config
#define UPLOAD_SEND_WAIT_TIME_DEVIDER 10
#define DEFAULT_Command_No_RESPONSE_TIMEOUT 15000

#endif
