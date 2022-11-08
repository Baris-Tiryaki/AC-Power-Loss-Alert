/*
I wrote this program to specifically warn me when AC power is lost, since my battery lasts a few minutes.
Use/modify at your own risk.

Window stays hidden during normal use. It re-appears when AC power is lost.
AC Power loss is alerted with auditory beeps.
It also increases the master volume to "minVolume" level temporarily before playing sounds.

Does not allow multiple instances to run simultaneously.
Has an option to automatically start at system startup.

//%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup

*/

#include<stdio.h>
#include<iostream>
#include<windows.h>
#include<mmdeviceapi.h>
#include<endpointvolume.h>
#include<fstream>
using namespace std;

float lastVolume=0;
int lastMute=0;
float minVolume=0.30;  //Modify this value to increase notification volume

void HideConsole(){
    ShowWindow(GetConsoleWindow(), SW_HIDE);
}
void ShowConsole(){
    HWND hwnd = GetConsoleWindow();
    ShowWindow(hwnd, SW_SHOW);
}
bool IsConsoleVisible(){
    return IsWindowVisible(GetConsoleWindow()) != FALSE;
}

bool ChangeVolume(bool enable){
    CoInitialize(NULL);
    IMMDeviceEnumerator *deviceEnumerator = NULL;
    CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, 
                          __uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);
    IMMDevice *defaultDevice = NULL;

    deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
    deviceEnumerator->Release();
    deviceEnumerator = NULL;

    IAudioEndpointVolume *endpointVolume = NULL;
    defaultDevice->Activate(__uuidof(IAudioEndpointVolume), 
         CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);
    defaultDevice->Release();
    defaultDevice = NULL;
    if(enable){   //set volume
        endpointVolume->GetMasterVolumeLevelScalar(&lastVolume);
        endpointVolume->GetMute(&lastMute);
        if(minVolume > lastVolume){
            endpointVolume->SetMasterVolumeLevelScalar((float)minVolume, NULL);
            endpointVolume->SetMute(false, NULL);
        }
    }
    else{                 //restore volume
        endpointVolume->SetMasterVolumeLevelScalar((float)lastVolume, NULL);
        endpointVolume->SetMute(lastMute, NULL);
    }
    endpointVolume->Release();
    CoUninitialize();

    return FALSE;
}

bool is_file_exist(const char *fileName){
    std::ifstream infile(fileName);
    return infile.good();
}

int main(int argc, char *argv[]){
    WCHAR exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    char startupPath[MAX_PATH]= "";
    char* appDataPath = getenv("APPDATA");
    sprintf(startupPath, "%s\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\PowerWarner.exe", appDataPath);
    
    bool startup_enable=false, startup_enabled=is_file_exist(startupPath);
    if(argc==2 && strcmp(argv[1],"-s")==0)  startup_enable=true;
    
    if(startup_enable==false && startup_enabled==false){
        printf("To automatically run at startup, run with \"-s\" flag\n");
    }
    else if(startup_enable==true){
        ifstream  src(exePath,     ios::binary);
        ofstream  dst(startupPath, ios::binary);
        dst << src.rdbuf();
        src.close();
        dst.close();
        printf("Created a copy of the executable in the startup folder: \"%s\"\n",startupPath);
        printf("To disable auto-start, delete the copy in startup folder\n");
        Sleep(3000);
    }
    CreateMutex(NULL, TRUE, "Mutex_D788s56s4gDBDGU3NS565HA");
    if (GetLastError() == ERROR_ALREADY_EXISTS){
        printf("Another instance is already running\nPlease terminate PowerWarner.exe from task manager\n");
        Sleep(1000);
        return 0;
    }
    printf("Power Warner\n");
    printf("Window will be hidden\n");
    Sleep(1000);
    SYSTEM_POWER_STATUS ps;
    int counter=1;
    while(true){
        GetSystemPowerStatus(&ps);
        bool AC_connected=ps.ACLineStatus;
        int percent = ps.BatteryLifePercent;
        int sound_delay=0;
        if(AC_connected==false){
            int freq=1000;
            int duration=0;
            if(counter==0){
                ShowConsole();
                duration= 1000;
            }
            else if(counter<=10 ){
                duration=100;
            }
            else if(counter<=120 && counter%5==0 ){
                duration=100;
            }
            else if(counter<=600 && counter%60==0 ){
                duration=100;
            }
            if(duration>0){
                ChangeVolume(true);
                Sleep(50);
                Beep(freq, duration);
                Sleep(50);
                ChangeVolume(false);
                SetForegroundWindow(GetConsoleWindow());
            }
            
            counter++;
            printf("! Power Disconnected (%d%%) !\n", percent );
        }
        else{
            if(counter>0){
                ChangeVolume(true);
                Sleep(100);
                Beep(1500, 100);
                Sleep(100);
                Beep(2100, 100);
                Sleep(100);
                Beep(3000, 100);
                Sleep(100);
                ChangeVolume(false);
                HideConsole();
                counter=0;
            }
        }
        //printf("%d %d\n", AC_connected, percent);
        if(AC_connected==true){
            if(percent==100) Sleep(1000);
            else if(percent>90) Sleep(500);
            else Sleep(300);
            /*ChangeVolume(true);
            Beep(1500, 100);
            ChangeVolume(false);*/
        }
        else{
            Sleep(500-sound_delay);
        }
    }
}
