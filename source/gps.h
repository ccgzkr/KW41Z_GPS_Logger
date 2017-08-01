#ifndef __GPS_H
#define __GPS_H

#include "fsl_common.h"

/*
** NMEA relevant
*/
typedef enum EgpsNmeaOut
{    
    NON = 0;
    GGA,
    GLL,
    GSA,
    GSV,
    RMC,
    VTG,
    MSS,
    NDF,
    ZDA
}EGPSNMEAOUT;

typedef enum Emonth
{    
    JAN = 1,
    FEB,
    MAR,
    APR,
    MAY,
    JUN,
    JUL,
    AUG,
    SEP,
    OCT,
    NOV,
    DEC,
}EMONTH;

typedef enum Eweekday
{    
    SUN = 0,
    MON,
    TUE,
    WED,
    THU,
    FRI,
    SAT,
}EWEEKDAY;

typedef struct SgpsNmeaUtc
{
    uint32_t hh;
    uint32_t mm;
    uint32_t ss;
    uint32_t fsss;
}SGPSNMEAUTC;

typedef struct SgpsNmeaLat
{
    uint32_t dd;
    uint32_t mm;
    uint32_t fmmmm;
}SGPSNMEALAT;

typedef struct SgpsNmeaLon
{
    uint32_t ddd;
    uint32_t mm;
    uint32_t fmmmm;
}SGPSNMEALON;

typedef struct SgpsNmeaSatInfo
{
    uint32_t ucSatId;
    uint32_t ucElev;     //0~90
    uint32_t udwAzim;   //0~359
    uint32_t ucSnr;      //0~99
}SGPSNMEASATINFO;

typedef struct SgpsNmeaGga
{
    uint8_t ucMid[8];
    SGPSNMEAUTC Sutc;
    uint8_t ucNs;    
    uint8_t ucEw;
    uint8_t ucPfi;  //0: Fix not available or invalid; 1: GPS SPS Mode, fix valid; 2: Differential GPS, SPS Mode, fix valid; 4: GPS PPS Mode, fix valid
    uint8_t ucSat;
    SGPSNMEALAT Slat;
    SGPSNMEALON Slon;
    double dblHdop;    //HDOP2+VDOP2=PDOP2    PDOP2+TDOP2=GDOP2 
    uint32_t uiMslAlt;
    uint8_t ucUnit;
    //uint32_t uiChecksum;
}SGPSNMEAGGA;

typedef struct SgpsNmeaGll
{
    uint8_t ucMid[8];
    uint8_t ucNs;
    uint8_t ucEw;
    uint8_t ucDataValid;
    SGPSNMEALAT Slat;    
    SGPSNMEALON Slon;    
    SGPSNMEAUTC Sutc;    
    //uint32_t uiChecksum;
}SGPSNMEAGLL;

typedef struct SgpsNmeaGsa
{
    uint8_t ucMid[8];
    uint8_t ucMode1;    //1: Fix not available; 2: 2D; 3: 3D
    uint8_t ucMode2;    //M: Manual; A: Automatic
    uint8_t ucSatId[12];
    double dblPdop;    //HDOP2+VDOP2=PDOP2 PDOP2+TDOP2=GDOP2
    double dblHdop;
    double dblVdop;
    //uint32_t uiChecksum;
}SGPSNMEAGSA;

typedef struct SgpsNmeaGsv
{
    uint8_t ucMid[8];    
    uint8_t ucSat;
    SGPSNMEASATINFO SsatInfo[12];
    //uint32_t uiChecksum;
}SGPSNMEAGSV;

typedef struct SgpsNmeaRmc
{
    uint8_t ucMid[8];
    SGPSNMEAUTC Sutc;
    uint8_t ucSatStat;    //A: data valid; V: data not valid
    SGPSNMEALAT Slat;
    uint8_t ucNs;
    SGPSNMEALON Slon;
    uint8_t ucEw;
    double dblSpd;
    double dblCourse;
    
}SGPSNMEARMC;

typedef struct SgpsNmeaMss
{
    uint8_t ucMid[8];
    uint8_t ucSS;
    uint8_t ucSnr;
    uint8_t ucBcnFrq;
    uint8_t ucBcnBr;
}SGPSNMEAMSS;

typedef struct SgpsNmeaVtg
{
    uint8_t ucMid[8];
    double dblCourse;
    uint8_t ucRef;
    double dblSpd;
    //uint32_t uiChecksum;
}SGPSNMEAVTG;

typedef union UgpsMessage
{
    SGPSNMEAGGA gga;
    SGPSNMEAGLL gll;
    SGPSNMEAGSA gsa;
    SGPSNMEAVTG vtg;
    SGPSNMEARMC rmc;
    SGPSNMEAGSV gsv;
    
}UGPSMESSAGE;

typedef struct Scmd100
{
    
}SCMD100;


typedef union UsirfCmd
{
    SGPSNMEAGGA gga;
    SGPSNMEAGLL gll;
    SGPSNMEAGSA gsa;
    SGPSNMEAVTG vtg;
    SGPSNMEARMC rmc;
    SGPSNMEAGSV gsv;
    
}USIRFCMD;


uint32_t uiGpsCmd(uint32_t cmd);

uint32_t uiGpsNmeaGga(char * pGpsMessageStr, SGPSNMEAGGA * pGpsGgaMsg);
uint32_t uiGpsNmeaCrcChk(char * pGpsMsgStr);
uint32_t uiGpsNmeaMsgValid(char * pGpsMsgStr);
uint32_t uiGpsNmeaTimConv(char * pGpsMsgUtcTime, int * time);



#endif

