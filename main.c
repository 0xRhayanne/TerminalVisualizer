#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <psapi.h>
#pragma comment(lib, "pdh.lib")
#else
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#endif

#define BAR_LENGTH 20
#define HISTORY_SIZE 30

// ------------------ Cross-platform sleep ------------------
void msleep(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

// ------------------ Clear screen ------------------
void clear_screen() {
    printf("\033[2J\033[H");
}

// ------------------ Bar drawing ------------------
void print_bar(int percent, int length, const char* color) {
    int filled = percent * length / 100;
    for (int i = 0; i < filled; i++) printf("%s#\033[0m", color);
    for (int i = filled; i < length; i++) printf("-");
}

// ------------------ Sparkline drawing ------------------
const char* bars_utf8[]={"▁","▂","▃","▄","▅","▆","▇","█"};

void print_sparkline(long history[], int max_value) {
    for(int j=0;j<HISTORY_SIZE;j++){
        int h = (int)((history[j] * 8.0) / max_value);
        if(h>7) h=7;
        if(h<0) h=0;
        printf("%s", bars_utf8[h]);
    }
}

// ------------------ Platform-specific data ------------------
#ifdef _WIN32
double get_cpu_total_usage() { return rand()%101; }
double get_cpu_core_usage(int core) { return rand()%101; }
int get_physical_cores() { SYSTEM_INFO s; GetSystemInfo(&s); return s.dwNumberOfProcessors / 2; }
int get_num_cores() { SYSTEM_INFO s; GetSystemInfo(&s); return s.dwNumberOfProcessors; }
long get_memory_used_mb() { return 4000 + rand()%8000; }
long get_network_kb_up() { return 50 + rand()%500; }
long get_network_kb_down() { return 100 + rand()%800; }
long get_disk_kb() { return 20 + rand()%200; }
int get_gpu_usage() { return rand()%101; }
double get_cpu_temperature() { return 40 + rand()%20; }
void get_loadavg(double *a, double *b, double *c) { *a=rand()%2; *b=rand()%2; *c=rand()%2; }
long get_swap_used_mb() { return 256 + rand()%512; }
long get_swap_total_mb() { return 2048; }
void get_disk_usage(long *used, long *total) { *used=20; *total=120; }
void get_top_process(char *name, int *cpu, int *mem) { strcpy(name,"chrome.exe"); *cpu=12; *mem=350; }
int get_battery_percent() { return 78; }
const char* get_battery_status() { return "Discharging"; }
#else
double get_cpu_total_usage() { return rand()%101; }
double get_cpu_core_usage(int core) { return rand()%101; }
int get_physical_cores() { return sysconf(_SC_NPROCESSORS_ONLN); }
int get_num_cores() { return sysconf(_SC_NPROCESSORS_ONLN); }
long get_memory_used_mb() { return 4000 + rand()%8000; }
long get_network_kb_up() { return rand()%512; }
long get_network_kb_down() { return rand()%512; }
long get_disk_kb() { return rand()%1024; }
int get_gpu_usage() { return rand()%100; }
double get_cpu_temperature() { return 40 + rand()%20; }
void get_loadavg(double *a, double *b, double *c) {
    double avg[3];
    getloadavg(avg,3);
    *a=avg[0]; *b=avg[1]; *c=avg[2];
}
long get_swap_used_mb() {
    struct sysinfo info;
    sysinfo(&info);
    return (info.totalswap - info.freeswap)/1024/1024;
}
long get_swap_total_mb() {
    struct sysinfo info;
    sysinfo(&info);
    return info.totalswap/1024/1024;
}
void get_disk_usage(long *used, long *total) {
    struct statvfs vfs;
    if(statvfs("/",&vfs)==0){
        long t=(long)(vfs.f_blocks*vfs.f_frsize/1024/1024/1024);
        long f=(long)(vfs.f_bfree*vfs.f_frsize/1024/1024/1024);
        *total=t; *used=t-f;
    } else { *used=0; *total=0; }
}
void get_top_process(char *name, int *cpu, int *mem) {
    // Simple mock (parsing /proc is complex)
    strcpy(name,"bash");
    *cpu=3; *mem=120;
}
int get_battery_percent() {
    FILE *f=fopen("/sys/class/power_supply/BAT0/capacity","r");
    if(!f) return -1;
    int val; fscanf(f,"%d",&val);
    fclose(f);
    return val;
}
const char* get_battery_status() {
    FILE *f=fopen("/sys/class/power_supply/BAT0/status","r");
    if(!f) return "N/A";
    static char buf[32];
    fscanf(f,"%31s",buf);
    fclose(f);
    return buf;
}
#endif

// ------------------ Dashboard ------------------
void draw_dashboard(long net_up_history[], long net_down_history[], int cpu_history[][HISTORY_SIZE],
                    long disk_history[], int gpu_history[], int history_size,
                    int num_cores, int phys_cores) {

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    double cpu_total = get_cpu_total_usage();
    long mem = get_memory_used_mb();
    long net_up = get_network_kb_up();
    long net_down = get_network_kb_down();
    long disk = get_disk_kb();
    int gpu = get_gpu_usage();
    double temp = get_cpu_temperature();

    double load1, load5, load15;
    get_loadavg(&load1,&load5,&load15);

    long swap_used = get_swap_used_mb();
    long swap_total = get_swap_total_mb();

    long disk_used, disk_total;
    get_disk_usage(&disk_used,&disk_total);

    char proc_name[64]; int proc_cpu, proc_mem;
    get_top_process(proc_name,&proc_cpu,&proc_mem);

    int battery_percent = get_battery_percent();
    const char* battery_status = get_battery_status();

    // Update histories
    for(int i=0;i<history_size-1;i++){
        net_up_history[i]=net_up_history[i+1];
        net_down_history[i]=net_down_history[i+1];
        disk_history[i]=disk_history[i+1];
        gpu_history[i]=gpu_history[i+1];
        for(int c=0;c<num_cores;c++) cpu_history[c][i]=cpu_history[c][i+1];
    }
    net_up_history[history_size-1]=net_up;
    net_down_history[history_size-1]=net_down;
    disk_history[history_size-1]=disk;
    gpu_history[history_size-1]=gpu;
    for(int c=0;c<num_cores;c++) cpu_history[c][history_size-1]=(int)get_cpu_core_usage(c);

    const char* colors[]={"\033[31m","\033[32m","\033[33m","\033[34m",
                          "\033[35m","\033[36m","\033[91m","\033[92m"};

    // --- Header ---
    printf("┌───────────────────────────────┐\n");
    printf("│   Terminal Visualizer (Demo)  │\n");
    printf("│   Time: %02d:%02d:%02d              │\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
    printf("└───────────────────────────────┘\n\n");

    // CPU Section
    printf("┌─────────────── CPU ───────────────┐\n");
    printf("│ %-10s : [", "CPU Total");
    print_bar((int)cpu_total,BAR_LENGTH,"\033[36m");
    printf("] %5.1f%% │\n",cpu_total);

    for(int i=0;i<num_cores;i++){
        int color_idx=i%8;
        const char* color=i>=phys_cores ? "\033[90m" : colors[color_idx];
        printf("│ CPU %-7d: [",i);
        print_bar(cpu_history[i][history_size-1],BAR_LENGTH,color);
        printf("] %3d%%   │\n", cpu_history[i][history_size-1]);
    }
    printf("└───────────────────────────────────┘\n\n");

    // Memory
    int mem_percent=mem*100/16000; if(mem_percent>100) mem_percent=100;
    const char* mem_color = mem_percent<50 ? "\033[32m":(mem_percent<80?"\033[33m":"\033[31m");
    printf("┌───────────── Memory ─────────────┐\n");
    printf("│ %-10s: [", "Memory");
    print_bar(mem_percent,BAR_LENGTH,mem_color);
    printf("] %5ld MB \n",mem);
    printf("└──────────────────────────────────┘\n\n");

    // Load Average
    printf("┌──────────── Load Avg ────────────┐\n");
    printf("│ 1m: %.2f  5m: %.2f  15m: %.2f    │\n", load1,load5,load15);
    printf("└──────────────────────────────────┘\n\n");

    // Swap
    int swap_percent=swap_total? (swap_used*100/swap_total):0;
    printf("┌────────────── Swap ──────────────┐\n");
    printf("│ Used: %4ld MB / %4ld MB (%2d%%)    │\n",swap_used,swap_total,swap_percent);
    printf("└──────────────────────────────────┘\n\n");

    // Disk
    int disk_percent = disk_total? (disk_used*100/disk_total):0;
    printf("┌────────────── Disk ──────────────┐\n");
    printf("│ /: %4ld GB / %4ld GB (%2d%%)       │\n",disk_used,disk_total,disk_percent);
    printf("└──────────────────────────────────┘\n\n");

    // Top Process
    printf("┌─────────── Top Process ──────────┐\n");
    printf("│ %s (CPU %d%%, Mem %dMB)  │\n",proc_name,proc_cpu,proc_mem);
    printf("└──────────────────────────────────┘\n\n");

    // Battery
    if(battery_percent>=0){
        printf("┌──────────── Battery ─────────────┐\n");
        printf("│ %3d%% (%s)               │\n",battery_percent,battery_status);
        printf("└──────────────────────────────────┘\n\n");
    }

    // Network
    printf("┌────────────── Network ───────────┐\n");
    printf("│ Up: %5ld KB/s  Down: %5ld KB/s │\n",net_up,net_down);
    printf("└──────────────────────────────────┘\n\n");

    // Disk I/O
    int disk_io_percent = disk*100/1024; if(disk_io_percent>100) disk_io_percent=100;
    printf("┌───────────── Disk I/O ───────────┐\n");
    printf("│ %-10s: [", "Disk I/O");
    print_bar(disk_io_percent,BAR_LENGTH,"\033[33m");
    printf("]%5ld KB \n", disk);
    printf("└──────────────────────────────────┘\n\n");

    // GPU
    int gpu_percent = gpu; if(gpu_percent>100) gpu_percent=100;
    printf("┌────────────── GPU ───────────────┐\n");
    printf("│ %-10s: [", "GPU Usage");
    print_bar(gpu_percent,BAR_LENGTH,"\033[35m");
    printf("] %3d%%   \n", gpu);
    printf("└──────────────────────────────────┘\n\n");

    // CPU Temp
    if(temp<0) printf("CPU Temp   : N/A\n");
    else {
        const char* temp_color=temp<50?"\033[32m":(temp<70?"\033[33m":"\033[31m");
        printf("CPU Temp   : %s%5.1f °C\033[0m\n",temp_color,temp);
    }
}

// ------------------ Main ------------------
int main() {
    srand((unsigned)time(NULL));
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if(hOut!=INVALID_HANDLE_VALUE){
        DWORD mode=0;
        if(GetConsoleMode(hOut,&mode)) SetConsoleMode(hOut,mode|ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#endif

    int num_cores=get_num_cores();
    int phys_cores=get_physical_cores();

    long net_up_history[HISTORY_SIZE]={0};
    long net_down_history[HISTORY_SIZE]={0};
    int cpu_history[64][HISTORY_SIZE]={{0}};
    long disk_history[HISTORY_SIZE]={0};
    int gpu_history[HISTORY_SIZE]={0};

    while(1){
        clear_screen();
        draw_dashboard(net_up_history,net_down_history,cpu_history,disk_history,gpu_history,
                       HISTORY_SIZE,num_cores,phys_cores);
        fflush(stdout);
        msleep(1000);
    }
    return 0;
}
