/*********************************************************************************
 * FILENAME         time_measurement.h
 * 
 * DESCRIPTION      These functions are part of the submission to exercises of 
 *                  the "Advanced Parallel Computing" 
 *                  lecture of the University of Heidelberg.
 *                               
 * 
 * AUTHORS          Sven Dorkenwald
 *                  Günther Schindler
 *
 * LAST CHANGE      11. MAY 2015
 * 
 ********************************************************************************/
#ifndef TIME_MEASUREMENT_H
#define TIME_MEASUREMENT_H

/*
 * DESCRIPTION - Starts a time-measurement, based on the gettimeofday() functions
 *               It has a resolution up to one microsecond.
 * PARAMETER   - void
 * RETURN      - double: elapsed seconds this day (is the parameter for dstopMesGTOD())
 */
double dstartMesGTOD(void);

/*
 * DESCRIPTION - Stops the time-measurement, based on the gettimeofday() functions.
 *               It has a resolution up to one microsecond.
 * PARAMETER   - double: return-value of dstartMesGTOD()
 * RETURN      - double: elapsed seconds since dstartMesGTOD()
 */
double dstopMesGTOD(double);

/*
 * DESCRIPTION - Start a time-measurement, based on the rdtsc() function
 * PARAMETER   - void
 * RETURN      - ull: actual process ticks
 */
unsigned long long ullstartMesRDTSC(void);

/*
 * DESCRIPTION - Stops the time-measurement, based on the rdtsc() function
 * PARAMETER   - ull: return value of dstartMesRDTSC()
 * RETURN      - double: elapsed seconds sind start of measurement
 */
double dstopMesRDTSC(unsigned long long);

/*
 * DESCRIPTION - 
 * PARAMETER   - 
 * RETURN      - 
 */
FILE* OpenTimeFile(void);

/*
 * DESCRIPTION - 
 * PARAMETER   - 
 * RETURN      - 
 */
void writeTimeToFile(FILE *, int, double);

#endif