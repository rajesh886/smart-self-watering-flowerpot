
//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "tm4c123gh6pm.h"
#include "uart0.h"
#include "comparator0.h"
#include "wait.h"
#include "lightsensor.h"
#include "moisturesensor.h"
#include "batteryvoltage.h"
#include "motorcontroller.h"
#include "speaker.h"
#include "hibernation.h"
#include "eeprom.h"


#define MAX_CHARS 80
#define MAX_FIELDS 5

uint32_t vol = 0;

typedef struct _USER_DATA
{
    char buffer[MAX_CHARS+1];
    uint8_t fieldCount;
    uint8_t fieldPosition[MAX_FIELDS];
    char fieldType[MAX_FIELDS];
}USER_DATA;

// Initialize Hardware
void initHw()
{
    // Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    // Note UART on port A must use APB
    SYSCTL_GPIOHBCTL_R = 0;

}

void getsUart0(USER_DATA* data)
{
  uint8_t count = 0;
  while(true){

    char c;
    c = getcUart0();

    if(c==127||c==8)
    {
       if(count>0)
       {
         count--;
         continue;
       }
       else{
       continue;
       }
    }

    else if(c==10 || c==13)
    {
       data->buffer[count]='\0';
       return;
    }

    else if(c >= 32)
    {
       if(count==MAX_CHARS)
       {
         data->buffer[count]='\0';
         return;
       }

       else{
       data->buffer[count++]=c;
       continue;
       }

     }

  }
}

bool alpha(char c)
{
  if((c>=65 && c<=90) || (c>=97 && c<=122))
  {
      return true;
  }
  else{
      return false;
  }
}

bool numeric(char c)
{
    if((c<=9) || (c>=48 && c<=57) || (c>=45 && c<=46))
    {
       return true;
    }
    else{
       return false;
    }
}

void parseFields(USER_DATA* data)
{
    uint8_t Count = 0;
    uint8_t i = 0;

    while( (data->buffer[i] != 0))
    {
        if(data->fieldCount == MAX_FIELDS)
        {

            return;
        }

        if(i == 0 && ( (alpha(data->buffer[i])) || (numeric(data->buffer[i])) ) )
        {
          if( (alpha(data->buffer[i])) )
          {
             data->fieldPosition[Count] = i;
             data->fieldType[Count] = 97;
             data->fieldCount = Count+1;
             Count++;
          }
          else if( numeric(data->buffer[i]) )
          {

              data->fieldPosition[Count] = i;
              data->fieldType[Count] = 110;
              data->fieldCount = Count+1;
              Count++;

          }

        }

        if( !(alpha(data->buffer[i])) && !(numeric(data->buffer[i])) )
        {
            data->buffer[i] = 0;

            if(alpha(data->buffer[i+1]))
            {

                data->fieldPosition[Count] = i+1;
                data->fieldType[Count] = 97;
                data->fieldCount = Count+1;
                Count++;


            }

            else if(numeric(data->buffer[i+1]))
            {

                data->fieldPosition[Count] = i+1;
                data->fieldType[Count] = 110;
                data->fieldCount = Count+1;
                Count++;

            }
         }

        i++;
    }
}

char* getFieldString(USER_DATA* data, uint8_t fieldNumber)
{
    if(fieldNumber <= data->fieldCount)
    {
       if(data->fieldType[fieldNumber] == 97)
       {
          return &data->buffer[data->fieldPosition[fieldNumber]];
       }
    }

      return '\0';
}

int32_t getFieldInteger(USER_DATA* data, uint8_t fieldNumber)
{
    char num[5];
    if(fieldNumber <= data->fieldCount)
    {
        if(data->fieldType[fieldNumber] == 110)
        {
            uint8_t count = data->fieldPosition[fieldNumber];
            uint8_t pos=0;
            while(data->buffer[count] != 0)
            {
                num[pos] = data->buffer[count];
                pos++;
                count++;
            }
            int32_t value = atoi(num);
            return value;
        }
    }

    return 0;
}

bool isCommand(USER_DATA* data, const char strCommand[], uint8_t minArguments)
{
  if(data->buffer[data->fieldPosition[0]] == *strCommand)
  {
      if((data->fieldCount)-1 >= minArguments)
      {
          return true;
      }

  }

  return false;
}

bool isWateringAllowed(int32_t sec1, int32_t sec2)
{
   int32_t seconds = getCurrentSeconds();
   if(sec1<seconds<sec2)
   {
       return true;
   }
   else
       return false;
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{
    char str[10], str1[100], str2[100], str3[100] ;
    float lightpercentage, moisturepercentage, batteryvoltage;
    int32_t moisturethreshold =0, level;
    int32_t time_elapsed, LLsec = 0, ULsec = 0;

    USER_DATA data;

    initHw();
    initUart0();

    initCOMP();
    initDEINT();
    initTIMER1();

    initMOTORHW();
    initSPEAKERHW();

    initHibernationHW();

    uint32_t errorval = eeprominit();
    uint32_t blockcnt = EEPROM_EESIZE_R & EEPROM_EESIZE_BLKCNT_M;  //counting the number of blocks
    //uint32_t write_blocknum = 0;
    uint32_t write_offset = 0;

    setUart0BaudRate(115200, 40e6);

    while(true)
    {

      if(kbhitUart0())
      {
          getsUart0(&data);

          putsUart0(data.buffer);

          putsUart0("\r\n");

          parseFields(&data);

          uint8_t i = 0;

          for(i = 0 ; i < data.fieldCount ; i++)
          {
              putcUart0(data.fieldType[i]);
              putsUart0("\r\n");
              putsUart0(&data.buffer[data.fieldPosition[i]]);
              putsUart0("\r\n");
          }

          putsUart0("\r\n");

          bool valid = false;

          if(isCommand(&data, "status", 0))
          {
             valid = true;
             vol = getVolume();
             lightpercentage = getLightPercentage();
             moisturepercentage = getMoisturePercentage();
             batteryvoltage = getBatteryVoltage();

             sprintf(str, "Volume inside the container : %lu ml.\r\n", vol);
             putsUart0(str);
             sprintf(str1, "Light percentage :           %4.1f\r\n", lightpercentage);
             putsUart0(str1);
             sprintf(str2, "Moisture percentage :        %4.1f\r\n", moisturepercentage);
             putsUart0(str2);
             sprintf(str3, "Battery Voltage :            %4.1fV\r\n", batteryvoltage);
             putsUart0(str3);

          }

          if(isCommand(&data, "pump", 1))
          {
              char* pumpstr1 = getFieldString(&data,1);
              char* pumpstr2 = getFieldString(&data,2);

              if(strcmp(pumpstr1,"ON") == 0)
              {
                enablePump();
              }

              waitMicrosecond(10000000);

              if(strcmp(pumpstr2,"OFF") == 0)
              {
                disablePump();
              }


              valid = true;
          }

          if(isCommand(&data, "alert", 1))
          {
             valid = true;
             level = getFieldInteger(&data, 2);
             vol = getVolume();
             lightpercentage = getLightPercentage();

             if(level < lightpercentage && batteryvoltage < 2)
             {
                 playBatteryLowAlert();
                 waitMicrosecond(2000000);
                 playBatteryLowAlert();
                 waitMicrosecond(2000000);
             }
             if(level < lightpercentage && vol < 150)
             {
                 playWaterLowAlert();
                 waitMicrosecond(2000000);
                 playWaterLowAlert();
                 waitMicrosecond(2000000);
             }
          }

          if(isCommand(&data, "time", 1))
          {
              valid = true;
              int32_t time1 = getFieldInteger(&data, 1);
              int32_t time2 = getFieldInteger(&data, 2);
              int32_t seconds = time1 * 60 * 60 + time2 * 60;
              while(!(HIB_CTL_R & 0x80000000));
              HIB_RTCLD_R = seconds;
              while(!(HIB_CTL_R & 0x80000000));

          }

          if(isCommand(&data, "water", 4))
          {
              valid = true;
              int32_t hour1 = getFieldInteger(&data, 1);
              int32_t min1 = getFieldInteger(&data, 2);
              int32_t hour2 = getFieldInteger(&data, 3);
              int32_t min2 = getFieldInteger(&data, 4);
              LLsec = hour1 * 60 * 60 + min1 * 60;
              ULsec = hour2 * 60 * 60 + min2 * 60;
          }

          if(isCommand(&data, "level", 1))
          {
              valid = true;
              moisturethreshold = getFieldInteger(&data, 1);
              char str[100];
              sprintf(str, "%u\r\n", moisturethreshold);
              putsUart0(str);
          }

          if(isCommand(&data, "clocktime", 0))
          {
            valid = true;
            time_elapsed = getCurrentSeconds();
            uint32_t hour1 = time_elapsed/3600;
            uint32_t min1 = (time_elapsed%3600)/60;
            char str[200];
            sprintf(str, "seconds:%u    time in hr and min=> %u:%u\r\n",time_elapsed,hour1, min1);
            putsUart0(str);
           }

          if(isCommand(&data, "history", 0))
          {
              valid = true;
              //uint32_t read_block = 0;
              uint32_t read_offset = 0;
              char str[100];
              while(read_offset != 14)
              {
                 uint32_t sensordata1= readeeprom(0,read_offset);
                 uint32_t sensordata2= readeeprom(1,read_offset);
                 uint32_t sensordata3= readeeprom(2,read_offset);
                 read_offset++;
                 sprintf(str,"Water: %u light: %u moisture: %u\r\n",sensordata1,sensordata2,sensordata3);
                 putsUart0(str);
              }
          }

          if(isCommand(&data,"erase",0))
          {
              valid = true;
              uint32_t erase_offset = 0;
              char str[100];
              while(erase_offset != 14)
              {
                 eraseeeprom(0,erase_offset);
                 eraseeeprom(1,erase_offset);
                 eraseeeprom(2,erase_offset);
                 erase_offset++;

              }
              write_offset =0;
              uint32_t read_offset1 = 0;

              while(read_offset1 != 14)
              {
                 uint32_t sensordata1= readeeprom(0,read_offset1);
                 uint32_t sensordata2= readeeprom(1,read_offset1);
                 uint32_t sensordata3= readeeprom(2,read_offset1);
                 read_offset1++;
                 sprintf(str,"Water: %u light: %u moisture: %u\r\n",sensordata1,sensordata2,sensordata3);
                 putsUart0(str);
              }
          }

          if(!valid)
              putsUart0("Invalid Command\r\n");

      }

      else
      {

          vol = getVolume();
          lightpercentage = getLightPercentage();
          batteryvoltage = getBatteryVoltage();
          moisturepercentage = getMoisturePercentage();

          writeeeprom(0,write_offset,vol);
          writeeeprom(1,write_offset,lightpercentage);
          writeeeprom(2,write_offset,moisturepercentage);
          write_offset++;
          if(write_offset == 14)
          {
             write_offset =0;
          }

          if((moisturepercentage < moisturethreshold) && isWateringAllowed(LLsec,ULsec))
          {
              waitMicrosecond(1000);
              enablePump();
              waitMicrosecond(5000000);
              disablePump();
              waitMicrosecond(30000000);
          }

          batteryvoltage = getBatteryVoltage();
          if(level < lightpercentage && batteryvoltage < 3)
          {
             playBatteryLowAlert();
             waitMicrosecond(2000000);
             playBatteryLowAlert();
             waitMicrosecond(2000000);
          }
          vol = getVolume();
          if(level < lightpercentage && vol < 150)
          {
             playWaterLowAlert();
             waitMicrosecond(2000000);
             playWaterLowAlert();
             waitMicrosecond(2000000);
          }
      }

    }
}
