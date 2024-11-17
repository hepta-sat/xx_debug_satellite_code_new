#include "mbed.h"
#include "HEPTA_DEBUG.h"
#include "HEPTA_CDH.h"
#include "HEPTA_EPS.h"
#include "HEPTA_SENSOR.h"
#include "HEPTA_COM.h"
#include "string"  // std::stringを使用するために必要

HEPTA_CDH cdh(p5, p6, p7, p8, "sd");
HEPTA_EPS eps(p16,p26);
HEPTA_SENSOR sensor(p17,
                  p28,p27,0x19,0x69,0x13,
                  p13, p14,p25,p24);
HEPTA_COM  com(p9,p10,9600);
RawSerial  sat(USBTX,USBRX,9600);
DigitalOut myleds[]={LED1,LED2,LED3,LED4};
DigitalOut condition(LED1);

/*============== define local variables ==============*/
Timer sattime;
int rcmd = 0,cmdflag = 0; //command variable
char cmd;
char answer;
float batvol,temp,ax,ay,az,gx,gy,gz,mx,my,mz;
char typeword[100];  // Operation check log file name 
int index = 0;       
/*====================================================*/

/*======== functions for serial communication ========*/
void end_confirmation();             //confirmation
void check_all_commands();
void blinking_led();
void timer_count();
void save_letters_to_sd();
void detect_battery_voltage();
void detect_temp();
void detect_9axis();
void detect_gpgga();
void take_picture();
void xbee_communication();

/*====================================================*/


// main loop
int main() {
    sat.printf(">> Debug satellite code start \r\n");
    sat.printf(">>>> Please hit command which you wanna check from 'a' to 'j'.\r\n");
    sat.printf(">>>> If you wanna check all commands at once , please just hit 'x'\r\n");
    
    while(1){
        sat.printf(">> -------- Please select following command --------\r\n");
        sat.printf(">>>> %c: Auto Check Mode\r\n",AUTO_CHECK_MODE_CMD);
        sat.printf(">>>> %c: Blinking LEDs\r\n", BLINKING_LEDS_CMD);
        sat.printf(">>>> %c: Detect the battery voltage\r\n", DETECT_BATTERY_VOL_CMD);
        sat.printf(">>>> %c: Detect the temperature\r\n", DETECT_TEMP_CMD);
        sat.printf(">>>> %c: Detect the 9-axis data\r\n", DETECT_9AXIS_CMD);
        sat.printf(">>>> %c: Detect GPS data as GPGGA format\r\n",DETECT_GPGGA_CMD);
        sat.printf(">>>> %c: Take a snapshot with camera\r\n",TAKE_PIC_CMD);
        sat.printf(">>>> %c: Check uplink and downlink with Xbee\r\n",XBEE_LINK_CMD);
        sat.printf(">> ------------------------------------------------\r\n");
        eps.turn_on_regulator();
        cmd = sat.getc();
        switch(cmd){
            case AUTO_CHECK_MODE_CMD:
                check_all_commands();
                break;

            case BLINKING_LEDS_CMD:
                blinking_led();
                break;
            
            case SAVE_LETTERS_TO_SD_CMD:
                save_letters_to_sd();
                break;
        
            case DETECT_BATTERY_VOL_CMD:
                detect_battery_voltage();
                break;

            case DETECT_TEMP_CMD:
                detect_temp();
                break;

            case DETECT_9AXIS_CMD:
                detect_9axis();
                break;
            case DETECT_GPGGA_CMD:
                
                detect_gpgga();
                break;
                
            case TAKE_PIC_CMD:
                take_picture();
                break;
                
            case XBEE_LINK_CMD:
                xbee_communication();
                break;
            
            default:
                sat.printf(">> Hit any key from 'a' to 'j' instead..\r\n");
                break;
        } //swith end
    } //while(1) end
}


/*====================================================*/
// define functions

void blinking_led(){
    sat.printf(">> %c: Blinking LEDs MODE Start!\r\n", BLINKING_LEDS_CMD);
    myleds[0] = 0;
    myleds[1] = 0;
    myleds[2] = 0;
    myleds[3] = 0;
    for(int j=0;j<4;j++){
        sat.printf(">>>> myleds[%d] ON!\r\n", j);
        myleds[j] = 1;
        wait_ms(1000);
    }
    end_confirmation(); 
}

void timer_count(){
    sat.printf(">> %c: Check timer count \r\n", TIMER_COUNT_CMD);
    sat.printf(">>>> Timer count start\r\n");
    sattime.start();
    for(int j=0;j<10;j++){
        sat.printf("Sat Time = %f [s]\r\n",sattime.read());
        wait_ms(500);
    }
    sattime.stop();
    end_confirmation(); 
}

void save_letters_to_sd(){
    sat.printf(">> %c: Save letters to SD Card\r\n", SAVE_LETTERS_TO_SD_CMD);
    sattime.start();
    
    char str[100];
    mkdir("/sd/mydir", 0777);
    FILE *fp = fopen("/sd/mydir/test.txt","w");
    if(fp == NULL) {
        error("Could not open file for write\r\n");
    }
    for(int i=0; i<10; i++)fprintf(fp,"Hello my name is HEPTA!\r\n");
    fclose(fp);
    fp = fopen("/sd/mydir/test.txt","r");
    for(int j = 0; j < 10; j++) {
        fgets(str,100,fp);
        sat.puts(str);
    }
    fclose(fp);
    sat.printf("Goodbye!!\r\n");
}

void detect_battery_voltage(){
    sat.printf(">> %c: Detect the battery voltage\r\n", DETECT_BATTERY_VOL_CMD);
    sattime.start();
    for(int j=0;j<10;j++){
        eps.vol(&batvol);
        sat.printf("BatVol = %.2f [V]\r\n",batvol);
        wait_ms(500);
    }
    end_confirmation(); 
}

void detect_temp(){
    sat.printf(">> %c: Detect the temperature\r\n", DETECT_TEMP_CMD);
    for(int j=0;j<10;j++){
        sensor.temp_sense(&temp);
        sat.printf("Temp = %.2f [degC]\r\n",temp);
        wait_ms(500);
    }
    end_confirmation(); 
}

void detect_9axis(){
    sat.printf(">>>> %c: Detect the 9-axis data\r\n", DETECT_9AXIS_CMD);
    for(int j=0;j<10;j++){
        sensor.sen_acc(&ax,&ay,&az);
        sensor.sen_gyro(&gx,&gy,&gz);
        sensor.sen_mag(&mx,&my,&mz);
        sat.printf("acc : %f,%f,%f\r\n",ax,ay,az);
        sat.printf("gyro: %f,%f,%f\r\n",gx,gy,gz);
        sat.printf("mag : %f,%f,%f\r\n\r\n",mx,my,mz);
        wait_ms(1000);
    }
    end_confirmation(); 
}

void detect_gpgga(){
    sat.printf(">>>> %c: Detect GPS data as GPGGA format\r\n",DETECT_GPGGA_CMD);
    sensor.gps_setting();
    for(int i=1; i<300; i++) {
        sat.putc(sensor.getc());
    }
    end_confirmation(); 
}

void take_picture(){
    sat.printf(">> %c: Take a snapshot with camera\r\n",TAKE_PIC_CMD);
    FILE *dummy = fopen("/sd/dummy.txt","w");
    if(dummy == NULL) {
        error("Could not open file for write\r\n");
    }
    fclose(dummy);
    
    sat.printf("Camera Snapshot Mode\r\n");
    sat.printf("Hit Any Key To Take Picture\r\n");
    while(!sat.readable()) {}
    sensor.Sync();
    sensor.initialize(HeptaCamera_GPS::Baud115200, HeptaCamera_GPS::JpegResolution320x240);
    sensor.test_jpeg_snapshot_picture("/sd/debug.jpg");
    end_confirmation(); 
}

void xbee_communication(){
    sat.printf(">> %c: Check uplink and downlink with Xbee\r\n",XBEE_LINK_CMD);
    while(1){
        com.xbee_receive(&rcmd,&cmdflag);
        sat.printf(">>>> Hit 'a' as an uplink command\r\n");
        if (cmdflag == 1) {
            if (rcmd == 'a') {
                sattime.start();
                sat.printf(">>>> Command Get, rcmd: %d\r\n",rcmd);
                com.printf(">>>> HEPTA Uplink OK, rcmd: %d\r\n",rcmd);
                for(int j=0;j<10;j++){
                    com.printf(">>>> Sat Time = %f [s]\r\n",sattime.read());
                    wait_ms(500);
                }
                end_confirmation(); 
                break;
            }
        }
        com.initialize();
        wait(1);
    }
}


void check_all_commands() {
    sattime.start();

    //Create logfile
    sat.printf("Type 'date', 'HEPTA kit number', 'Any version number' according to below format.\r\n");
    sat.printf("YearMonthDay-KitNumber-VersionNumber : ex) 20240101-4015-1\r\n");
    sat.printf("When you have finished entering that, press 'Enter'.\r\n");
    while (true) {
        if (sat.readable()) {  
            char c = sat.getc(); 
            sat.printf("%c",c);
            if (c == '\r') {  
                typeword[index] = '\0'; 
                break;  
            } else {
                typeword[index] = c; 
                index++;
            }
        }
    }
    std::string filename = typeword;  
    
    
    char str[5000];
    mkdir("/sd/", 0777);
    std::string logfile = "/sd/" + filename + ".txt"; 
    
    FILE *fp = fopen(logfile.c_str(), "w");
    if(fp == NULL) {
        error("Could not open file for write\r\n");
    }
    
    //LED
    sat.printf(">> %c: Blinking LEDs MODE Start!\r\n", BLINKING_LEDS_CMD);
    myleds[0] = 0;
    myleds[1] = 0;
    myleds[2] = 0;
    myleds[3] = 0;
    for(int j=0;j<4;j++){
        sat.printf(">>>> myleds[%d] ON!\r\n", j);
        myleds[j] = 1;
        wait_ms(1000);
    }
    end_confirmation(); 
    fprintf(fp,"Time = %f, LED Check OK!\r\n",sattime.read());
    
    //Battery
    for(int j=0;j<5;j++){
        eps.vol(&batvol);
        sat.printf("BatVol = %.2f [V]\r\n",batvol);
        fprintf(fp,"Time = %f, BatVol = %.2f [V]\r\n",sattime.read(),batvol);
        wait_ms(500);
    }
    end_confirmation(); 
    
    //Tempture
    sat.printf(">> %c: Detect the temperature\r\n", DETECT_TEMP_CMD);
    for(int j=0;j<5;j++){
        sensor.temp_sense(&temp);
        sat.printf("Temp = %.2f [degC]\r\n",temp);
        fprintf(fp,"Time = %f, Temp = %.2f [V]\r\n",sattime.read(),temp);
        wait_ms(500);
    }
    end_confirmation(); 

    //9axis
    sat.printf(">>>> %c: Detect the 9-axis data\r\n", DETECT_9AXIS_CMD);
    for(int j=0;j<5;j++){
        sensor.sen_acc(&ax,&ay,&az);
        sensor.sen_gyro(&gx,&gy,&gz);
        sensor.sen_mag(&mx,&my,&mz);
        sat.printf("acc : %f,%f,%f\r\n",ax,ay,az);
        sat.printf("gyro: %f,%f,%f\r\n",gx,gy,gz);
        sat.printf("mag : %f,%f,%f\r\n\r\n",mx,my,mz);
        fprintf(fp,"Time = %f, acc : %f,%f,%f\r\n",sattime.read(),ax,ay,az);
        fprintf(fp,"Time = %f, gyro: %f,%f,%f\r\n",sattime.read(),gx,gy,gz);
        fprintf(fp,"Time = %f, mag : %f,%f,%f\r\n\r\n",sattime.read(),mx,my,mz);
        wait_ms(500);
    }
    end_confirmation(); 
    
    //GPS
    sat.printf(">>>> %c: Detect GPS data as GPGGA format\r\n",DETECT_GPGGA_CMD);
    fprintf(fp,"Time = %f\r\n",sattime.read());
    sensor.gps_setting();
    char gpsdata[500];
    for(int i=1; i<300; i++) {
        gpsdata[i]=sensor.getc();
        //fprintf(fp,"Time = %f, acc : %f,%f,%f\r\n",sattime.read(),ax,ay,az);
    }
    for(int i=1; i<300; i++) {
        sat.putc(gpsdata[i]);
    }
    for(int i=1; i<300; i++) {
        fprintf(fp,"%c",gpsdata[i]);
    }
    fprintf(fp,"\r\n");
    end_confirmation(); 
    
    //SD check
    fclose(fp);
    fp = fopen(logfile.c_str(),"r");
    for(int i=1; i<50; i++) {
        fgets(str,100,fp);
        sat.puts(str);
    }
    
    fclose(fp);
    
    //Camera
    sat.printf(">> %c: Take a snapshot with camera\r\n",TAKE_PIC_CMD);
    FILE *dummy = fopen("/sd/dummy.txt","w");
    if(dummy == NULL) {
        error("Could not open file for write\r\n");
    }
    fclose(dummy);
    
    sat.printf("Camera Snapshot Mode\r\n");
    sat.printf("Hit Any Key To Take Picture\r\n");
    while(!sat.readable()) {}
    sensor.Sync();
    sensor.initialize(HeptaCamera_GPS::Baud115200, HeptaCamera_GPS::JpegResolution320x240);
    sensor.test_jpeg_snapshot_picture("/sd/debug.jpg");
    end_confirmation(); 

    //Xbee
    sat.printf(">> %c: Check uplink and downlink with Xbee\r\n",XBEE_LINK_CMD);
    while(1){
        com.xbee_receive(&rcmd,&cmdflag);
        sat.printf(">>>> Hit 'a' as an uplink command\r\n");
        if (cmdflag == 1) {
            if (rcmd == 'a') {
                sattime.start();
                sat.printf(">>>> Command Get, rcmd: %d\r\n",rcmd);
                com.printf(">>>> HEPTA Uplink OK, rcmd: %d\r\n",rcmd);
                for(int j=0;j<10;j++){
                    com.printf(">>>> Sat Time = %f [s]\r\n",sattime.read());
                    wait_ms(500);
                }
                end_confirmation(); 
                break;
            }
        }
        com.initialize();
        wait(1);
    }

}

void end_confirmation(){
    for(int j=0;j<2;j++){
        myleds[0] = 1;
        myleds[1] = 1;
        myleds[2] = 1;
        myleds[3] = 1;
        wait_ms(100);
        myleds[0] = 0;
        myleds[1] = 0;
        myleds[2] = 0;
        myleds[3] = 0;
        wait_ms(100);
    }
    sat.printf(">>>> Finish!! \r\n\r\n");
}
/*====================================================*/