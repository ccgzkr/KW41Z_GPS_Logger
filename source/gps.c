#include "gps.h"

static EWEEKDAY uiGpsWeekdayCalc(int32_t * time);
static int32_t uiGpsDayCalc(int32_t * time);
static uint32_t uiIntCalc(uint8_t *ch, uint32_t len, uint32_t *result);
static uint32_t uiFractCalc(uint8_t *ch, uint32_t len, double *result);
static uint32_t uiIsLeap(uint32_t year);
static uint32_t uiIsDigit(const uint8_t ch);
static uint32_t uiTimeValidator(int32_t * time);
static uint32_t uiGpsUtcTimCon(int32_t timezone, int32_t * time);
static uint8_t ucCrcCh2Dig(uint8_t *str);


int32_t uiDate[2][13] = {
        {365, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}, 
        {366, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

//uint8_t digit[16] = (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);


/*
**  GENERAL
**
*/

uint32_t uiGpsCmd(uint32_t cmd)
{
    
    return 0;
}

static uint32_t uiIntCalc(uint8_t *ch, uint32_t len, uint32_t *result)
{
    uint32_t i;
    uint8_t temp;
    uint32_t re = 0;
    for(i = 0; i < len; i++)
    {
        temp = *(ch + i);
        if(uiIsDigit(temp))
        {
            re *= 10;
            re += temp - '0';
        }
        else
        {
            return 1;
        }
    }
    *result = re;
    return 0;
}

static uint32_t uiFractCalc(uint8_t *ch, uint32_t len, double *result)
{
    uint32_t i;
    uint8_t temp;
    double re = 0;
    for(i = len - 1; i > 0; i--)
    {
        temp = *(ch + i);
        if(uiIsDigit(temp))
        {
            re /= 10;
            re += (temp - '0') / 10;
        }
        else
        {
            return 1;
        }
    }
    *result = re;
    return 0;
}

static uint32_t uiIsLeap(uint32_t year)
{
    if(year % 400 == 0)
    {
        return 1;
    }
    else if(year % 100 == 0)
    {
        return 0;
    }
    else if(year % 4 == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static uint32_t uiIsDigit(const uint8_t ch)
{
    if(ch >= '0' && ch <= '9')
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static uint32_t uiTimeValidator(int32_t * time)
{
    if(time[1] > 12 || time[1] < 1 || \
       time[2] > uiDate[uiIsLeap(time[0])][time[1]] || time[2] < 1 || \
       time[3] >= 24 || time[3] < 0 || \
       time[4] > 60 || time[4] < 0 || \
       time[5] > 60 || time[5] < 0)
    {
        return 0;
    }
    else 
    {
        return 1;
    }
}

/*
**  timezone 
**  time pointer to time array, int32_t time[6] = {0:yy, 1:mm, 2:dd, 3:hh, 4:mm, 5:ss}
**
*/
static uint32_t uiGpsUtcTimCon(int32_t timezone, int32_t * time)
{
    if(timezone < -12 || timezone > 12)
    {
        return 1;
    }
    
    time[3] += timezone;
    
    if(time[3] > 24)
    {
        time[3] -= 24;
        if(time[2] == uiDate[uiIsLeap(time[0])][time[1]])
        {            
            time[2] = 1;
            if(time[1] == 12)
            {
                time[0]++;
                time[1] = 1;
            }
            else
            {
                time[1]++;
            }
        }
        else
        {
            time[2]++;
        }
    }
    else if(time[3] < 0)
    {
        time[3] += 24;
   
        if(time[2]  == 1)
        {
            if(time[1] == 1)
            {
                time[1] = 12;
                time[0]--;
            }
            else
            {
                time[1]--;
            }
            time[2] = uiDate[uiIsLeap(time[0])][time[1]];
        }
        else
        {
            time[2]--;
        }
    }
    else
    {
        time[3] += timezone;
    }
    
    return 0;
}

/*
**  weekday
**  time pointer to time array, int32_t time[6] = {0:yy, 1:mm, 2:dd, 3:hh, 4:mm, 5:ss}
**
*/
static EWEEKDAY uiGpsWeekdayCalc(int32_t * time)
{
    EWEEKDAY weekday = SUN;   //2017.01.01
    int32_t days = uiGpsDayCalc(time);
    if(days >= 0)
    {
        weekday = SUN + days % 7;
    }
    else
    {
        weekday = SUN + (7 - days % 7) % 7;
    }
    return weekday;
}

/*
**  daycalc 
**  time pointer to time array, int32_t time[6] = {0:yy, 1:mm, 2:dd, 3:hh, 4:mm, 5:ss}
**  sum of days since the beginning of 2017.01.01
*/
static int32_t uiGpsDayCalc(int32_t * time)
{
    int32_t days = 0;
    int i;
    if(time[0] > 2016)
    {
        for(i = 2017; i < time[0]; i++)
        {
            days += uiDate[uiIsLeap(i)][0];
        }
        for(i = 1; i < time[1]; i++)
        {
            days += uiDate[uiIsLeap(time[0])][i];
        }
        days += time[2] - 1;
    }
    else
    {
        for(i = 2016; i > time[0]; i--)
        {
            days -= uiDate[uiIsLeap(i)][0];
        }
        for(i = 12; i > time[1]; i--)
        {
            days -= uiDate[uiIsLeap(time[0])][i];
        }
        days -= uiDate[uiIsLeap(time[0])][time[1]] - time[2] + 1;
    }
    
    return days;
}

/*
**  NMEA RELEVANT
**
*/

/*
**  GGA ¨C Global Positioning System Fixed Data
**  $GPGGA, 161229.487,3723.2475,N,12158.3416,W,1,07,1.0,9.0,M,,,,0000*18\r\n
**
*/
uint32_t uiGpsNmeaGga(char * pGpsMessageStr, SGPSNMEAGGA * pGpsGgaMsg)
{
    
    return 0;
}

/*
**  Calculating Checksums for NMEA Input/Output
**  The checksum is calculated from all characters in the message, 
**  including commas but excluding the ¡°$¡± and ¡°*¡± delimiters. The 
**  hexadecimal result is converted to two ASCII characters (0¨C9, A¨CF), 
**  of which the most significant appears first.
*/
uint32_t uiGpsNmeaCrcChk(uint8_t * pGpsMsgStr)
{
    uint8_t t = 0;
    uint32_t len = strlen(pGpsMsgStr);
    uint32_t checksum = 0;
    uint32_t i;
    for(i = 1; i < len - 3; i++)
    {
        checksum ^= pGpsMsgStr[i];
    }
    if((checksum & 0x7FFF) == ucCrcCh2Dig(pGpsMsgStr + i))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static uint8_t ucCrcCh2Dig(uint8_t *str)
{
    return (16 * (*str - '0') + (*(str + 1)) - '0');
}

/*
**  NMEA message validator
**
*/
EGPSNMEAOUT uiGpsNmeaMsgValid(char * pGpsMsgStr)
{
    EGPSNMEAOUT ret = NON;
    if(*pGpsMsgStr)
    {
        
        return 1;
    }
    else
    {
        return 0;
    }
}

uint32_t uiGpsNmeaTimConv(char * pGpsMsgUtcTime, int * time)
{
    
    return 0;
}

/*
**  BINARY RELEVANT
**
*/

