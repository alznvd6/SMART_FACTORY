#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <conio.h> 

#define PORT_NAME "COM1"
#define DB_PATH_BASE "..\\..\\..\\..\\infrastructure\\database\\worker_info"

bool Get_Worker_Name_Fast(const char* worker_id, char* out_name, size_t out_len) {
    char registry_path[260];
    snprintf(registry_path, sizeof(registry_path), "%s\\worker_ID.txt", DB_PATH_BASE);

    FILE* f_reg = NULL;
    if (fopen_s(&f_reg, registry_path, "r") != 0 || f_reg == NULL) return false;

    int target_line = atoi(worker_id) % 1000;
    char line[128];
    int current_line = 1;
    bool found = false;

    while (fgets(line, sizeof(line), f_reg)) {
        if (current_line == target_line) {
            char curr_id[20], name[50], family[50];
            if (sscanf_s(line, "%[^,],%[^,],%s", curr_id, (unsigned int)sizeof(curr_id), name, (unsigned int)sizeof(name), family, (unsigned int)sizeof(family)) == 3) {
                snprintf(out_name, out_len, "%s %s", name, family);
                found = true;
            }
            break;
        }
        current_line++;
    }
    fclose(f_reg);
    return found;
}

int Get_Worker_Group(const char* worker_id) {
    char path[260];
    FILE* f = NULL;

    snprintf(path, sizeof(path), "%s\\date\\group\\2\\%s.txt", DB_PATH_BASE, worker_id);
    if (fopen_s(&f, path, "r") == 0 && f != NULL) { fclose(f); return 2; }

    snprintf(path, sizeof(path), "%s\\date\\group\\3\\%s.txt", DB_PATH_BASE, worker_id);
    if (fopen_s(&f, path, "r") == 0 && f != NULL) { fclose(f); return 3; }

    snprintf(path, sizeof(path), "%s\\date\\group\\4\\%s.txt", DB_PATH_BASE, worker_id);
    if (fopen_s(&f, path, "r") == 0 && f != NULL) { fclose(f); return 4; }

    return 0;
}

bool Append_Worker_Bonus(const char* worker_id, float multiplier, const char* task_name) {
    char file_path[260];
    snprintf(file_path, sizeof(file_path), "%s\\date\\bonus\\%s.txt", DB_PATH_BASE, worker_id);

    FILE* f_bonus = NULL;
    if (fopen_s(&f_bonus, file_path, "a") != 0 || f_bonus == NULL) return false;

    time_t t = time(NULL);
    struct tm tm_info;
    localtime_s(&tm_info, &t);

    char date_str[20];
    snprintf(date_str, sizeof(date_str), "%d-%02d-%02d", tm_info.tm_year + 1900, tm_info.tm_mon + 1, tm_info.tm_mday);

    if (multiplier == (int)multiplier) {
        fprintf(f_bonus, "%s(%d)\t%s\n", date_str, (int)multiplier, task_name);
    }
    else {
        fprintf(f_bonus, "%s(%.2f)\t%s\n", date_str, multiplier, task_name);
    }

    fclose(f_bonus);
    return true;
}

bool Check_Task_Eligibility(const char* task_target, int worker_group, unsigned long long elapsed_min, float* out_multiplier) {
    if (strcmp(task_target, "Bonus_G2") == 0) {
        if (elapsed_min < 1 && worker_group == 2) { *out_multiplier = 1.0f; return true; }
        if (elapsed_min >= 1 && elapsed_min < 2 && (worker_group == 2 || worker_group == 3)) {
            *out_multiplier = (worker_group == 2) ? 1.0f : 2.0f; return true;
        }
        if (elapsed_min >= 2 && elapsed_min < 3 && (worker_group == 2 || worker_group == 3 || worker_group == 4)) {
            if (worker_group == 2) *out_multiplier = 1.0f;
            if (worker_group == 3) *out_multiplier = 2.0f;
            if (worker_group == 4) *out_multiplier = 3.0f;
            return true;
        }
    }
    else if (strcmp(task_target, "Bonus_G3") == 0) {
        if (elapsed_min < 1 && worker_group == 3) { *out_multiplier = 1.0f; return true; }
        if (elapsed_min >= 1 && elapsed_min < 2 && (worker_group == 3 || worker_group == 4)) {
            *out_multiplier = (worker_group == 3) ? 1.0f : 2.0f; return true;
        }
        if (elapsed_min >= 2 && elapsed_min < 3 && (worker_group == 3 || worker_group == 4 || worker_group == 2)) {
            if (worker_group == 3) *out_multiplier = 1.0f;
            if (worker_group == 4) *out_multiplier = 2.0f;
            if (worker_group == 2) *out_multiplier = 0.75f;
            return true;
        }
    }
    else if (strcmp(task_target, "Bonus_G4") == 0) {
        if (elapsed_min < 1 && worker_group == 4) { *out_multiplier = 1.0f; return true; }
        if (elapsed_min >= 1 && elapsed_min < 2 && (worker_group == 4 || worker_group == 3)) {
            *out_multiplier = (worker_group == 4) ? 1.0f : 0.75f; return true;
        }
        if (elapsed_min >= 2 && elapsed_min < 3 && (worker_group == 4 || worker_group == 3 || worker_group == 2)) {
            if (worker_group == 4) *out_multiplier = 1.0f;
            if (worker_group == 3) *out_multiplier = 0.75f;
            if (worker_group == 2) *out_multiplier = 0.50f;
            return true;
        }
    }
    return false;
}

HANDLE init_serial(const char* port_name) {
    HANDLE hSerial = CreateFileA(port_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) return INVALID_HANDLE_VALUE;

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) return INVALID_HANDLE_VALUE;

    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    dcbSerialParams.fOutxCtsFlow = FALSE;
    dcbSerialParams.fOutxDsrFlow = FALSE;
    dcbSerialParams.fDtrControl = DTR_CONTROL_DISABLE;
    dcbSerialParams.fRtsControl = RTS_CONTROL_DISABLE;

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 10;
    timeouts.ReadTotalTimeoutConstant = 10;
    timeouts.ReadTotalTimeoutMultiplier = 1;
    SetCommTimeouts(hSerial, &timeouts);

    if (!SetCommState(hSerial, &dcbSerialParams)) return INVALID_HANDLE_VALUE;
    return hSerial;
}

void Show_Main_Menu() {
    printf("\n========================================================\n");
    printf("    Smart Factory - Generation-Fair Central System       \n");
    printf("========================================================\n");
    printf(" [1] Create & Inject New Extra Task\n");
    printf(" [2] Clear Screen\n");
    printf(" [0] Exit System\n");
    printf("--------------------------------------------------------\n");
    printf(" Choose an option or wait for incoming serial data: ");
}

int main() {
    HANDLE hComm = init_serial(PORT_NAME);
    if (hComm == INVALID_HANDLE_VALUE) {
        printf("[ERROR] Failed to open %s.\n", PORT_NAME);
        system("pause");
        return 1;
    }

    char rx_buffer[128];
    char tx_buffer[128];
    DWORD bytes_read, bytes_written;

    bool is_task_active = false;
    char current_task_name[50] = { 0 };
    char current_task_target[20] = { 0 };

    unsigned long long task_accumulated_time = 0;
    unsigned long long last_tick = 0;              
    bool is_frozen = false;                        
    int last_sent_phase = 0;                       

    Show_Main_Menu();

    while (1) {
        if (_kbhit()) {
            char choice = _getch();
            if (choice == '1') {
                if (is_task_active) {
                    printf("\n[WARNING] A task is already active! Wait for timeout or worker claim.\n");
                }
                else {
                    printf("\n\n--- CREATE NEW EXTRA TASK ---\n");
                    printf("Enter Task Name (spaces allowed, e.g., Boiler Repair): ");

                    char temp_name[50] = { 0 };
                    fgets(temp_name, sizeof(temp_name), stdin);
                    temp_name[strcspn(temp_name, "\n")] = '\0';
                    strcpy_s(current_task_name, sizeof(current_task_name), temp_name);

                    int target_group = 0;
                    printf("Select Target Age Group (2, 3 or 4): ");
                    scanf_s("%d", &target_group);
                    while (getchar() != '\n'); 

                    if (target_group == 2 || target_group == 3 || target_group == 4) {
                        is_task_active = true;
                        is_frozen = false;
                        task_accumulated_time = 0;
                        last_tick = GetTickCount64();
                        last_sent_phase = 1;
                        snprintf(current_task_target, sizeof(current_task_target), "Bonus_G%d", target_group);

                        printf("\n[SUCCESS] Extra Task '%s' active for Group %d! Timer started.\n", current_task_name, target_group);

                        snprintf(tx_buffer, sizeof(tx_buffer), "[CENTRAL_TASK] Task:%s|Target:%s|Phase:1\r\n", current_task_name, current_task_target);
                        WriteFile(hComm, tx_buffer, (DWORD)strlen(tx_buffer), &bytes_written, NULL);
                    }
                    else {
                        printf("\n[ERROR] Invalid Age Group! Task creation aborted.\n");
                    }
                }
                Show_Main_Menu();
            }
            else if (choice == '2') {
                system("cls");
                Show_Main_Menu();
            }
            else if (choice == '0') {
                printf("\nShutting down Central System...\n");
                break;
            }
        }

        if (is_task_active) {
            unsigned long long current_tick = GetTickCount64();

            if (!is_frozen) {
                task_accumulated_time += (current_tick - last_tick);
            }
            last_tick = current_tick; 

            unsigned long long elapsed_min = task_accumulated_time / 60000; 
            int current_phase = 1;

            if (elapsed_min < 1) current_phase = 1;
            else if (elapsed_min >= 1 && elapsed_min < 2) current_phase = 2;
            else if (elapsed_min >= 2 && elapsed_min < 3) current_phase = 3;
            else current_phase = 4;

            if (current_phase != last_sent_phase) {
                last_sent_phase = current_phase;
                snprintf(tx_buffer, sizeof(tx_buffer), "[CENTRAL_TASK] Task:%s|Target:%s|Phase:%d\r\n", current_task_name, current_task_target, current_phase);
                WriteFile(hComm, tx_buffer, (DWORD)strlen(tx_buffer), &bytes_written, NULL);
                printf("\n[PHASE CHANGE -> MCU]: %s", tx_buffer);
            }

            if (elapsed_min >= 3) {
                printf("\n\n[TIMEOUT] Task '%s' expired! Randomly assigning...\n", current_task_name);

                char lucky_worker[20] = "1001";
                if (strcmp(current_task_target, "Bonus_G3") == 0) strcpy_s(lucky_worker, sizeof(lucky_worker), "1004");

                char worker_name[100] = { 0 };
                Get_Worker_Name_Fast(lucky_worker, worker_name, sizeof(worker_name));

                Append_Worker_Bonus(lucky_worker, 1.0f, current_task_name);

                snprintf(tx_buffer, sizeof(tx_buffer), "[CENTRAL] TIMEOUT_ASSIGNED:%s:%s(Multiplier:1.0)\r\n", current_task_name, worker_name);
                WriteFile(hComm, tx_buffer, (DWORD)strlen(tx_buffer), &bytes_written, NULL);
                printf("[RESPONDED]: %s", tx_buffer);

                is_task_active = false;
                Show_Main_Menu();
            }
        }

        if (ReadFile(hComm, rx_buffer, sizeof(rx_buffer) - 1, &bytes_read, NULL) && bytes_read > 0) {
            rx_buffer[bytes_read] = '\0';
            while (bytes_read > 0 && (rx_buffer[bytes_read - 1] == '\r' || rx_buffer[bytes_read - 1] == '\n')) {
                rx_buffer[--bytes_read] = '\0';
            }

            printf("\n\n[SERIAL INCOMING]: \"%s\"\n", rx_buffer);

            if (strcmp(rx_buffer, "CMD_TIMER:FREEZE") == 0) {
                is_frozen = true;
                last_tick = GetTickCount64(); 
                printf("[SYSTEM STAT] Alarm Active. Central Timer is FROZEN.\n");
            }
            else if (strcmp(rx_buffer, "CMD_TIMER:RESUME") == 0) {
                is_frozen = false;
                last_tick = GetTickCount64(); 
                printf("[SYSTEM STAT] Alarm Cleared. Central Timer RESUMED.\n");

                if (is_task_active) {
                    snprintf(tx_buffer, sizeof(tx_buffer), "[CENTRAL_TASK] Task:%s|Target:%s|Phase:%d\r\n", current_task_name, current_task_target, last_sent_phase);
                    WriteFile(hComm, tx_buffer, (DWORD)strlen(tx_buffer), &bytes_written, NULL);
                }
            }
            else {
                char parsed_id[20] = { 0 };
                char parsed_action[50] = { 0 };

                if (sscanf_s(rx_buffer, "%19[^:]: %49s", parsed_id, (unsigned int)sizeof(parsed_id), parsed_action, (unsigned int)sizeof(parsed_action)) == 2 ||
                    sscanf_s(rx_buffer, "%19[^:]:%49s", parsed_id, (unsigned int)sizeof(parsed_id), parsed_action, (unsigned int)sizeof(parsed_action)) == 2) {

                    if (is_task_active && strcmp(parsed_action, current_task_name) == 0) {
                        unsigned long long elapsed_min = task_accumulated_time / 60000;

                        if (elapsed_min >= 3) {
                            snprintf(tx_buffer, sizeof(tx_buffer), "[CENTRAL_REJECT] Selection blocked in Phase 4!\r\n");
                            WriteFile(hComm, tx_buffer, (DWORD)strlen(tx_buffer), &bytes_written, NULL);
                            printf("[RESPONDED]: %s", tx_buffer);
                        }
                        else {
                            int w_group = Get_Worker_Group(parsed_id);
                            float final_multiplier = 0.0f;

                            if (Check_Task_Eligibility(current_task_target, w_group, elapsed_min, &final_multiplier)) {
                                char worker_name[100] = { 0 };
                                Get_Worker_Name_Fast(parsed_id, worker_name, sizeof(worker_name));

                                Append_Worker_Bonus(parsed_id, final_multiplier, current_task_name);

                                snprintf(tx_buffer, sizeof(tx_buffer), "[CENTRAL_OK] Task:%s|Winner:%s|Multiplier:%.2f\r\n", current_task_name, worker_name, final_multiplier);
                                WriteFile(hComm, tx_buffer, (DWORD)strlen(tx_buffer), &bytes_written, NULL);
                                printf("[CENTRAL RESPONDED]: %s", tx_buffer);

                                is_task_active = false; 
                            }
                            else {
                                snprintf(tx_buffer, sizeof(tx_buffer), "[CENTRAL_REJECT] Not your group priority yet!\r\n");
                                WriteFile(hComm, tx_buffer, (DWORD)strlen(tx_buffer), &bytes_written, NULL);
                                printf("[RESPONDED]: %s", tx_buffer);
                            }
                        }
                    }
                    else if (strcmp(parsed_action, "BONUS") == 0) {
                        if (!is_task_active) {
                            snprintf(tx_buffer, sizeof(tx_buffer), "[CENTRAL_REJECT] No active task available!\r\n");
                            WriteFile(hComm, tx_buffer, (DWORD)strlen(tx_buffer), &bytes_written, NULL);
                            printf("[RESPONDED]: %s", tx_buffer);
                        }
                        else {
                            unsigned long long elapsed_min = task_accumulated_time / 60000;
                            int w_group = Get_Worker_Group(parsed_id);
                            float final_multiplier = 0.0f;

                            if (Check_Task_Eligibility(current_task_target, w_group, elapsed_min, &final_multiplier)) {
                                char worker_name[100] = { 0 };
                                Get_Worker_Name_Fast(parsed_id, worker_name, sizeof(worker_name));

                                Append_Worker_Bonus(parsed_id, final_multiplier, current_task_name);

                                snprintf(tx_buffer, sizeof(tx_buffer), "[CENTRAL_OK] Task:%s|Winner:%s|Multiplier:%.2f\r\n", current_task_name, worker_name, final_multiplier);
                                WriteFile(hComm, tx_buffer, (DWORD)strlen(tx_buffer), &bytes_written, NULL);
                                printf("[CENTRAL RESPONDED]: %s", tx_buffer);

                                is_task_active = false;
                            }
                            else {
                                snprintf(tx_buffer, sizeof(tx_buffer), "[CENTRAL_REJECT] Not your group priority yet!\r\n");
                                WriteFile(hComm, tx_buffer, (DWORD)strlen(tx_buffer), &bytes_written, NULL);
                                printf("[RESPONDED]: %s", tx_buffer);
                            }
                        }
                    }
                }
            }
            Show_Main_Menu();
        }
        Sleep(50);
    }

    CloseHandle(hComm);
    return 0;
}