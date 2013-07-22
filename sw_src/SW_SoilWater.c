/********************************************************/
/********************************************************/
/*	Source file: SoilWater.c
	Type: module
	Application: SOILWAT - soilwater dynamics simulator
	Purpose: Read / write and otherwise manage the
		soil water values.  Includes reading input
		parameters and ordinary daily water flow.
		In addition, generally useful soilwater-
		related functions should go here.
	History:
	(8/28/01) -- INITIAL CODING - cwb
	10/04/2010	(drs) added snowMAUS snow accumulation, sublimation and melt algorithm: Trnka, M., Kocmánková, E., Balek, J., Eitzinger, J., Ruget, F., Formayer, H., Hlavinka, P., Schaumberger, A., Horáková, V., Mozny, M. & Zalud, Z. (2010) Simple snow cover model for agrometeorological applications. Agricultural and Forest Meteorology, 150, 1115-1127.
					replaced SW_SWC_snow_accumulation, SW_SWC_snow_sublimation, and SW_SWC_snow_melt with SW_SWC_adjust_snow (temp_min, temp_max, *ppt, *rain, *snow, *snowmelt)
	10/19/2010	(drs) replaced snowMAUS simulation with SWAT2K routines: Neitsch S, Arnold J, Kiniry J, Williams J. 2005. Soil and water assessment tool (SWAT) theoretical documentation. version 2005. Blackland Research Center, Texas Agricultural Experiment Station: Temple, TX.
	10/25/2010	(drs)	in SW_SWC_water_flow(): replaced test that "swc can't be adjusted on day 1 of year 1" to "swc can't be adjusted on start day of first year of simulation"
	01/04/2011	(drs) added parameter '*snowloss' to function SW_SWC_adjust_snow()
	08/22/2011	(drs) added function RealD SW_SnowDepth( RealD SWE, RealD snowdensity)
	01/20/2012	(drs)	in function 'SW_SnowDepth': catching division by 0 if snowdensity is 0
	02/03/2012	(drs)	added function 'RealD SW_SWC_SWCres(RealD sand, RealD clay, RealD porosity)': which calculates 'Brooks-Corey' residual volumetric soil water based on Rawls & Brakensiek (1985)
	05/25/2012  (DLM) edited SW_SWC_read(void) function to get the initial values for soil temperature from SW_Site
*/
/********************************************************/
/********************************************************/

/* =================================================== */
/* =================================================== */
/*                INCLUDES / DEFINES                   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "generic.h"
#include "filefuncs.h"
#include "myMemory.h"
#include "SW_Defines.h"
#include "SW_Files.h"
#include "SW_Model.h"
#include "SW_Site.h"
#include "SW_SoilWater.h"
#include "SW_Output.h"



void SW_Water_Flow( void); /* see Water_Flow.c */


/* =================================================== */
/*                  Global Variables                   */
/* --------------------------------------------------- */

extern SW_MODEL SW_Model;
extern SW_SITE SW_Site;
extern SW_OUTPUT SW_Output[];
SW_SOILWAT SW_Soilwat;  /* declared here, externed elsewhere */


/* =================================================== */
/*                Module-Level Variables               */
/* --------------------------------------------------- */
static char *MyFileName;


/* =================================================== */
/* =================================================== */
/*             Private Function Definitions            */
/* --------------------------------------------------- */

static void _read_hist( TimeInt year);

static void _clear_hist( void) {
/* --------------------------------------------------- */
TimeInt d;
LyrIndex z;
for(d=0; d<MAX_DAYS; d++) {
ForEachSoilLayer(z) {
SW_Soilwat.hist.swc[d][z]     = SW_MISSING;
SW_Soilwat.hist.std_err[d][z] = SW_MISSING;
}
}
}

/* =================================================== */
/* =================================================== */
/*             Public Function Definitions             */
/* --------------------------------------------------- */

void SW_SWC_construct(void) {
/* =================================================== */

memset(&SW_Soilwat, 0, sizeof(SW_SOILWAT));

}

void SW_SWC_water_flow( void) {
/* =================================================== */
/* Adjust SWC according to historical (measured) data
* if available, compute water flow, and check if swc
* is above threshold for "wet" condition.
*/

LyrIndex i;


/* if there's no swc observation for today,
* it shows up as SW_MISSING.  The input must
* define historical swc for at least the top
* layer to be recognized.
* IMPORTANT: swc can't be adjusted on day 1 of first year of simulation.
10/25/2010	(drs)	in SW_SWC_water_flow(): replaced test that "swc can't be adjusted on day 1 of year 1" to "swc can't be adjusted on start day of first year of simulation"
*/


if ( SW_Soilwat.hist_use && !missing( SW_Soilwat.hist.swc[SW_Model.doy-1][1]) ) {

	if (! (SW_Model.doy == SW_Model.startstart && SW_Model.year == SW_Model.startyr) ) {

	SW_SWC_adjust_swc(SW_Model.doy);

	} else {
		LogError(logfp, LOGWARN, "Attempt to set SWC on start day of first year of simulation disallowed.");
	}

} else {
	SW_Water_Flow();
}


ForEachSoilLayer(i) 
SW_Soilwat.is_wet[i] = (GE( SW_Soilwat.swc[Today][i],
SW_Site.lyr[i]->swc_wet));
}

void SW_SWC_end_day( void) {
/* =================================================== */
SW_SOILWAT *v = &SW_Soilwat;
LyrIndex i;

ForEachSoilLayer(i)
v->swc[Yesterday][i] = v->swc[Today][i];


v->snowpack[Yesterday] = v->snowpack[Today];

}


void SW_SWC_new_year(void) {
/* =================================================== */
/* init first doy swc, either by the computed init value
* or by the last day of last year, which is also,
* coincidentally, Yesterday.
*/

LyrIndex lyr;
TimeInt year = SW_Model.year;
Bool reset = (SW_Site.reset_yr ||
SW_Model.year == SW_Model.startyr);

memset(&SW_Soilwat.yrsum, 0, sizeof(SW_SOILWAT_OUTPUTS));

/* reset the swc */
ForEachSoilLayer(lyr) {
if (reset) {
SW_Soilwat.swc[Today][lyr]
= SW_Soilwat.swc[Yesterday][lyr]
= SW_Site.lyr[lyr]->swc_init;
SW_Soilwat.drain[lyr] = 0.;
} else {
SW_Soilwat.swc[Today][lyr] = SW_Soilwat.swc[Yesterday][lyr];
}
}

/* reset the snowpack */
if (reset) {
	SW_Soilwat.snowpack[Today] = SW_Soilwat.snowpack[Yesterday] = 0.;
} else {
	SW_Soilwat.snowpack[Today] = SW_Soilwat.snowpack[Yesterday];
}

/* reset the historical (measured) values, if needed */
if ( SW_Soilwat.hist_use && year >= SW_Soilwat.hist.yr.first)
_read_hist( year);

/* always reset deep drainage */
if (SW_Site.deepdrain)
SW_Soilwat.swc[Today][SW_Site.deep_lyr] = 0.;
}


void SW_SWC_read(void) {
/* =================================================== */
/* Like all of the other "objects", read() reads in the
* setup parameters.  See _read_hist() for reading
* historical files.
*
*  1/25/02 - cwb - removed unused records of logfile and
*          start and end days.  also removed the SWTIMES dy
*          structure element.
*/

SW_SOILWAT *v = &SW_Soilwat;
FILE *f;
int lineno=0, nitems=4;

// gets the soil temperatures from where they are read in the SW_Site struct for use later
// SW_Site.c must call it's read function before this, or it won't work
LyrIndex i;
ForEachSoilLayer(i)
	v->sTemp[i] = SW_Site.lyr[i]->sTemp;

MyFileName = SW_F_name(eSoilwat);
f = OpenFile(MyFileName, "r");

while( GetALine(f, inbuf) ) {
switch(lineno) {
case 0:  v->hist_use = (atoi(inbuf)) ? TRUE : FALSE; break;
case 1:  v->hist.file_prefix = (char *)Str_Dup(inbuf);                break;
case 2:  v->hist.yr.first = yearto4digit(atoi(inbuf));  break;
case 3:  v->hist.method = atoi(inbuf);               break;
}
if (!v->hist_use)  return;
lineno++;
}
if (lineno < nitems) {
LogError(logfp, LOGFATAL,
"%s : Insufficient parameters specified.",MyFileName);
}
if (v->hist.method < 1 || v->hist.method > 2) {
LogError(logfp, LOGFATAL,
"%s : Invalid swc adjustment method.", MyFileName);
}
v->hist.yr.last = SW_Model.endyr;
v->hist.yr.total = v->hist.yr.last - v->hist.yr.first +1;
}


static void _read_hist( TimeInt year) {
/* =================================================== */
/* read a file containing historical swc measurements.
* Enter with year a four digit year number.  This is
* appended to the swc prefix to make the input file
* name.
*
*
* 1/25/02 - cwb - removed year field from input records.
*         This code uses GetALine() which discards comments
*         and only one year's data per file is allowed, so
*         and the date is part of the file name, but if you
*         must, you can add the date as well as other data
*         inside comments, preferably at the top of the file.
*
* Format of the input file is
* "doy layer swc stderr"
*
* for example,
*  90 1 1.11658 .1
*  90 2 1.11500 .1
*    ...
* 185 1 2.0330  .23
* 185 2 3.1432  .25
*    ...
* note that missing days or layers will not cause
* an error in the input, but missing layers could
* cause problems in the flow model.
*/
SW_SOILWAT *v = &SW_Soilwat;
FILE *f;
TimeInt doy;
int x, lyr, recno=0;
RealF swc, st_err;
char fname[MAX_FILENAMESIZE];



sprintf(fname, "%s.%4d", v->hist.file_prefix, year);

if (!FileExists(fname)) {
LogError(logfp, LOGWARN, "Historical SWC file %s not found.", fname);
return;
}

f = OpenFile(fname, "r");

_clear_hist();

while( GetALine(f, inbuf) ) {
recno++;
x=sscanf( inbuf, "%d %d %f %f",
&doy,
&lyr,
&swc,
&st_err);
if (x<4) {
LogError(logfp, LOGFATAL, "%s : Incomplete layer data at record %d\n"
"  Should be DOY LYR SWC STDERR.",
fname, recno);
}
if (x>4) {
LogError(logfp, LOGFATAL, "%s : Too many input fields at record %d\n"
"  Should be DOY LYR SWC STDERR.",
fname, recno);
}
if (doy < 1 || doy > MAX_DAYS) {
LogError(logfp, LOGFATAL, "%s : Day of year out of range at record %d",
fname, recno);
}
if (lyr < 1 || lyr > MAX_LAYERS) {
LogError(logfp, LOGFATAL, "%s : Layer number out of range (%d > %d), record %d\n",
fname, lyr, MAX_LAYERS, recno);
}

v->hist.swc[doy-1][lyr-1]  = swc;
v->hist.std_err[doy-1][lyr-1] = st_err;

}

}

void SW_SWC_adjust_swc( TimeInt doy) {
/* =================================================== */
/* 01/07/02 (cwb) added final loop to guarantee swc > swc_min
*/

SW_SOILWAT *v = &SW_Soilwat;
RealD lower, upper;
LyrIndex lyr;
TimeInt dy = doy -1;

switch (SW_Soilwat.hist.method) {
case SW_Adjust_Avg:
ForEachSoilLayer(lyr) {
v->swc[Today][lyr] += v->hist.swc[dy][lyr];
v->swc[Today][lyr] /= 2.;
}
break;

case SW_Adjust_StdErr:
ForEachSoilLayer(lyr) {
upper = v->hist.swc[dy][lyr] + v->hist.std_err[dy][lyr];
lower = v->hist.swc[dy][lyr] - v->hist.std_err[dy][lyr];
if ( GT(v->swc[Today][lyr], upper) )
v->swc[Today][lyr] = upper;
else if ( LT(v->swc[Today][lyr], lower) )
v->swc[Today][lyr] = lower;
}
break;

default:
LogError(logfp, LOGFATAL, "%s : Invalid SWC adjustment method.",
SW_F_name(eSoilwat));
}


/* this will guarantee that any method will not lower swc */
/* below the minimum defined for the soil layers          */
ForEachSoilLayer(lyr)
	v->swc[Today][lyr] = fmax(v->swc[Today][lyr], SW_Site.lyr[lyr]->swc_min);

}

void SW_SWC_adjust_snow( RealD temp_min, RealD temp_max, RealD ppt, RealD *rain, RealD *snow, RealD *snowmelt, RealD *snowloss ) {
/*---------------------
	10/04/2010	(drs) added snowMAUS snow accumulation, sublimation and melt algorithm: Trnka, M., Kocmánková, E., Balek, J., Eitzinger, J., Ruget, F., Formayer, H., Hlavinka, P., Schaumberger, A., Horáková, V., Mozny, M. & Zalud, Z. (2010) Simple snow cover model for agrometeorological applications. Agricultural and Forest Meteorology, 150, 1115-1127.
					replaced SW_SWC_snow_accumulation, SW_SWC_snow_sublimation, and SW_SWC_snow_melt with SW_SWC_adjust_snow
	10/19/2010	(drs) replaced snowMAUS simulation with SWAT2K routines: Neitsch S, Arnold J, Kiniry J, Williams J. 2005. Soil and water assessment tool (SWAT) theoretical documentation. version 2005. Blackland Research Center, Texas Agricultural Experiment Station: Temple, TX.
	Inputs:	temp_min: daily minimum temperature (C)
			temp_max: daily maximum temperature (C)
			ppt: daily precipitation (cm)
			snowpack[Yesterday]: yesterday's snowpack (water-equivalent cm)
	Outputs:	snowpack[Today], partitioning of ppt into rain and snow, snowmelt and snowloss
---------------------*/


RealD	*snowpack = &SW_Soilwat.snowpack[Today],
		doy = SW_Model.doy,
		temp_ave, Rmelt, snow_cov = 1., cov_soil = 0.5,
		SnowAccu = 0., SnowMelt = 0., SnowLoss = 0.;
		
static RealD temp_snow = 0.;

temp_ave = (temp_min+temp_max)/2.;
/* snow accumulation */
if ( LE(temp_ave, SW_Site.TminAccu2) ) {SnowAccu = ppt;} else {SnowAccu = 0.;}
*rain = fmax(0., ppt - SnowAccu);
*snow = fmax(0., SnowAccu);
*snowpack += SnowAccu;

/* snow melt */
Rmelt = (SW_Site.RmeltMax+SW_Site.RmeltMin)/2. + sin((doy-81.)/58.09) * (SW_Site.RmeltMax-SW_Site.RmeltMin)/2.;
temp_snow = temp_snow*(1-SW_Site.lambdasnow) + temp_ave * SW_Site.lambdasnow;
if ( GT(temp_snow, SW_Site.TmaxCrit) ) {SnowMelt = fmin( *snowpack, Rmelt * snow_cov * ((temp_snow + temp_max)/2. - SW_Site.TmaxCrit) );} else {SnowMelt = 0.;}
if ( GT(*snowpack, 0.) ) {
	*snowmelt = fmax(0., SnowMelt);
	*snowpack = fmax(0., *snowpack - *snowmelt );
} else {
	*snowmelt = 0.;
}

/* snow loss through sublimation and other processes */
SnowLoss = fmin( *snowpack, cov_soil * SW_Soilwat.pet );
if ( GT(*snowpack, 0.) ) {
	*snowloss = fmax(0., SnowLoss);
	*snowpack = fmax(0., *snowpack - *snowloss );
} else {
	*snowloss = 0.;
}

}


RealD SW_SnowDepth( RealD SWE, RealD snowdensity) {
/*---------------------
	08/22/2011	(drs)	calculates depth of snowpack
	Input:	SWE: snow water equivalents (cm = 10kg/m2)
			snowdensity (kg/m3)
	Output: snow depth (cm)
---------------------*/
	if( GT(snowdensity, 0.) ){
		return SWE / snowdensity * 10. * 100.;
	} else {
		return 0.;
	}
}


RealD SW_SWC_vol2bars ( RealD lyrvolcm, LyrIndex n) {
/**********************************************************************
PURPOSE: Calculate the soil water potential or the soilwater
content of the current layer,
as a function of soil texture at the layer.

DATE:  April 2, 1992

HISTORY:
9/1/92  (SLC) if swc comes in as zero, set swpotentl to
upperbnd.  (Previously, we flagged this
as an error, and set swpotentl to zero).

27-Aug-03 (cwb) removed the upperbnd business. Except for
missing values, swc < 0 is impossible, so it's an error,
and the previous limit of swp to 80 seems unreasonable.
return 0.0 if input value is MISSING

INPUTS:
swc - soilwater content of the current layer (cm/layer)
n   - layer number to index the **lyr pointer.

These are the values for each layer obtained via lyr[n]:
width  - width of current soil layer
psis   - "saturation" matric potential
thetas - saturated moisture content.
b       - see equation below.
swc_lim - limit for matric potential

LOCAL VARIABLES:
theta1 - volumetric soil water content

DEFINED CONSTANTS:
barconv - conversion factor from bars to cm water.  (i.e.
1 bar = 1024cm water)

COMMENT:
See the routine "watreqn" for a description of how the variables
psis, b, binverse, thetas are initialized.

OUTPUTS:
swpotentl - soilwater potential of the current layer
(if swflag=TRUE)
or
soilwater content (if swflag=FALSE)

DESCRIPTION: The equation and its coefficients are based on a
paper by Cosby,Hornberger,Clapp,Ginn,  in WATER RESOURCES RESEARCH
June 1984.  Moisture retention data was fit to the power function

**********************************************************************/

SW_LAYER_INFO *lyr= SW_Site.lyr[n];
float  theta1, swp=0.;

if (missing(lyrvolcm) || ZRO(lyrvolcm)) return 0.0;

if ( GT(lyrvolcm, 0.0) ) {
theta1 = (lyrvolcm / lyr->width) * 100.;
swp  = lyr->psis / pow(theta1/lyr->thetas, lyr->b) / BARCONV;
} else {
LogError( logfp, LOGFATAL,
"Invalid SWC value (%.4f) in SW_SWC_swc2potential.\n"
"    Year = %d, DOY=%d, Layer = %d\n",
lyrvolcm, SW_Model.year, SW_Model.doy, n);
}

return swp;
}

RealD SW_SWC_bars2vol(RealD bars, LyrIndex n) {
/* =================================================== */
/* used to be swfunc in the fortran version */
/* 27-Aug-03 (cwb) moved from the Site module. */
/* return the volume as cmH2O/cmSOIL */

SW_LAYER_INFO *lyr = SW_Site.lyr[n];
RealD t, p;

bars *= BARCONV;
p = pow(lyr->psis / bars, lyr->binverse);
t = lyr->thetas * p * 0.01;
return( t );

}


RealD SW_SWC_SWCres(RealD sand, RealD clay, RealD porosity){
/*---------------------
	02/03/2012	(drs)	calculates 'Brooks-Corey' residual volumetric soil water based on Rawls WJ, Brakensiek DL (1985) Prediction of soil water properties for hydrological modeling. In Watershed management in the Eighties (eds Jones EB, Ward TJ), pp. 293-299. American Society of Civil Engineers, New York.
						however, equation is only valid if (0.05 < clay < 0.6) & (0.05 < sand < 0.7)

	Input:	sand: soil texture sand content (fraction)
			clay: soil texture clay content (fraction)
			porosity: soil porosity = saturated VWC (fraction)
	Output: residual volumetric soil water (cm/cm)
---------------------*/
RealD res;

	sand *= 100.;
	clay *= 100.;

	res = -0.0182482 + 0.00087269*sand + 0.00513488*clay + 0.02939286*porosity - 0.00015395*pow(clay, 2) - 0.0010827*sand*porosity - 0.00018233*pow(clay, 2)*pow(porosity, 2) + 0.00030703*pow(clay, 2)*porosity - 0.0023584*pow(porosity, 2)*clay;
	
	return( fmax(res, 0.) );
}



#ifdef DEBUG_MEM
#include "myMemory.h"
/*======================================================*/
void SW_SWC_SetMemoryRefs( void) {
/* when debugging memory problems, use the bookkeeping
code in myMemory.c
This routine sets the known memory refs in this module
so they can be  checked for leaks, etc.  Includes
malloc-ed memory in SOILWAT.  All refs will have been
cleared by a call to ClearMemoryRefs() before this, and
will be checked via CheckMemoryRefs() after this, most
likely in the main() function.
*/

/*  NoteMemoryRef(SW_Soilwat.hist.file_prefix); */


}

#endif
