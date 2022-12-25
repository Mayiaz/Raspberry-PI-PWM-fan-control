#include <pigpio.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <iostream>

#define PWM_PIN 18
#define MIN_TEMP 30
#define MAX_TEMP 80


#define FILE_NAME "/sys/class/thermal/thermal_zone0/temp"

volatile int terminating = 0;

void terminate(int sign) {
  signal(SIGINT, SIG_DFL);
  terminating = 1;
}

unsigned getDutycycle(unsigned currentTemp){
  if(currentTemp/1000 <= MIN_TEMP)
    return 0;
  if(currentTemp/1000 >=MAX_TEMP)
    return 1000000;
  return (unsigned)(((currentTemp/1000.0 - MIN_TEMP)/(MAX_TEMP - MIN_TEMP)) * 1000000);
}

void safeStop(){
  gpioSetMode(PWM_PIN,PI_INPUT);
  gpioSetPullUpDown(PWM_PIN, PI_PUD_DOWN);
  gpioTerminate();
}

unsigned getCurrentTemp(){
  FILE *temperatureFile;
  unsigned T;
  temperatureFile = fopen (FILE_NAME, "r");
  if (temperatureFile == NULL)
    return MAX_TEMP;
  fscanf (temperatureFile, "%u", &T);
  fclose (temperatureFile);
  return T;
}

int main(int argc, char **argv){

  int pi = gpioInitialise();
  signal(SIGINT, terminate);
  if(pi < 0) return 1;

  if(gpioSetMode(PWM_PIN,PI_ALT0) != 0){
    safeStop();
    return 1;
  }
  while(!terminating){
    if(gpioHardwarePWM(PWM_PIN, 25000, getDutycycle(getCurrentTemp())) != 0) {
      safeStop();
      return 1;
    }
    sleep(5);
  }
  safeStop();
  return 0;
}