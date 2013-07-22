/********************************************************/
/********************************************************/
/*	Source file: Veg_Estab.c
	Type: module
	Application: SOILWAT - soilwater dynamics simulator
	Purpose: Reads/writes vegetation establishment info.
	History:
	(8/28/01) -- INITIAL CODING - cwb
	
	8-Sep-03 -- Establishment code works as follows.
		More than one species can be tested per year.
		No more than one establishment per species per year may occur.
		If germination occurs, check environmental conditions
		for establishment.  If a dry period (>max_drydays_postgerm)
		occurs, or temp out of range, kill the plant and
		start over from pregermination state.  Thus, if the
		early estab fails, start over and try again if
		enough time is available.  This is simple but not
		realistic.  Better would be to count and report the
		number of days that would allow establishment which
		would give an index to the number of seedlings
		established in a year.
	20090826 (drs) added return; after LBL_Normal_Exit:
*/
/********************************************************/
/********************************************************/

/* =================================================== */
/* =================================================== */
/*                INCLUDES / DEFINES                   */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "generic.h"
#include "filefuncs.h"
#include "myMemory.h"
#include "SW_Defines.h"
#include "SW_Files.h"
#include "SW_Site.h"
#include "SW_Times.h"
#include "SW_Model.h"
#include "SW_SoilWater.h"
#include "SW_Weather.h"
#include "SW_VegEstab.h"

/* =================================================== */
/*                  Global Variables                   */
/* --------------------------------------------------- */
extern SW_MODEL SW_Model;
extern SW_SITE SW_Site;
extern SW_WEATHER SW_Weather;
extern SW_SOILWAT SW_Soilwat;
extern Bool EchoInits;

SW_VEGESTAB SW_VegEstab; /* declared here, externed elsewhere */

/* =================================================== */
/*                Module-Level Variables               */
/* --------------------------------------------------- */
static char *MyFileName;


/* =================================================== */
/* =================================================== */
/*             Private Function Declarations           */
/* --------------------------------------------------- */
static void _sanity_check(unsigned int sppnum);
static void _spp_init(unsigned int sppnum);
static void _read_spp(const char *infile);
static void _checkit (TimeInt doy, unsigned int sppnum);
static void _zero_state(unsigned int sppnum);
static unsigned int _new_species(void);
static void _echo_inits(void);


/* =================================================== */
/* =================================================== */
/*             Public Function Definitions             */
/* --------------------------------------------------- */

void SW_VES_construct(void) {
/* =================================================== */
/* note that an initializer that is called during
* execution (better called clean() or something)
* will need to free all allocated memory first
* before clearing structure.
*/

/*  for(i=0; i<SW_VegEstab.count; i++)
Mem_Free(SW_VegEstab.parms[i]);
*/
memset(&SW_VegEstab, 0, sizeof(SW_VegEstab));

}

void SW_VES_new_year(void) {
/* =================================================== */
/* we can use the debug memset because we allocated days,
* that is, it wasn't allocated by the compiler. */

if (0==SW_VegEstab.count) return;
Mem_Set(SW_VegEstab.yrsum.days, 0, SW_VegEstab.count);

}

void SW_VES_read(void) {
/* =================================================== */

FILE *f;
IntU i;

MyFileName = SW_F_name(eVegEstab);
f = OpenFile(MyFileName, "r");
SW_VegEstab.use = TRUE;

/* if data file empty or useflag=0, assume no
* establishment checks and just continue the model run. */
if (!GetALine(f, inbuf) || *inbuf == '0' ) {
SW_VegEstab.use = FALSE;
if (EchoInits)
LogError(logfp, LOGNOTE, "Establishment not used.\n");
return;
}
while(GetALine(f, inbuf) ) {
_read_spp(inbuf);
}
CloseFile(&f);

for (i=0; i < SW_VegEstab.count; i++)
_spp_init(i);

if ( SW_VegEstab.count > 0)
SW_VegEstab.yrsum.days = (TimeInt *)
Mem_Calloc(SW_VegEstab.count,
sizeof(TimeInt),
"SW_VES_read()");
if (EchoInits) _echo_inits();
}

void SW_VES_checkestab (void) {
/* =================================================== */
IntUS i;

for(i=0; i<SW_VegEstab.count; i++)
_checkit(SW_Model.doy, i);

}



/* =================================================== */
/* =================================================== */
/*            Private Function Definitions             */
/* --------------------------------------------------- */

static void _checkit (TimeInt doy, unsigned int sppnum) {

SW_VEGESTAB_INFO *v = SW_VegEstab.parms[sppnum];
SW_WEATHER_2DAYS *wn = &SW_Weather.now;
SW_SOILWAT  *sw = &SW_Soilwat;

IntU i;
RealF avgtemp = wn->temp_avg[Today], /* avg of today's min/max temp */
avgswc;  /* avg_swc today */


if (doy == SW_Model.firstdoy) {
_zero_state(sppnum);
}

if (v->no_estab || v->estab_doy > 0)
goto LBL_Normal_Exit;

/* keep up with germinating wetness regardless of current state */
if (GT(sw->swc[Today][0], v->min_swc_germ) )
v->wetdays_for_germ++;
else
v->wetdays_for_germ = 0;

if (doy < v->min_pregerm_days) goto LBL_Normal_Exit;

/* ---- check for germination, establishment */
if (!v->germd && v->wetdays_for_germ >= v->min_wetdays_for_germ) {

if (doy  < v->min_pregerm_days)
goto LBL_Normal_Exit;
if (doy > v->max_pregerm_days) {
v->no_estab = TRUE;
goto LBL_Normal_Exit;
}
/* temp doesn't affect wetdays */
if ( LT(avgtemp, v->min_temp_germ) || GT(avgtemp, v->max_temp_germ) )
goto LBL_Normal_Exit;

v->germd = TRUE;
goto LBL_Normal_Exit;

} else {  /* continue monitoring sprout's progress */

/* any dry period (> max_drydays) or temp out of range
* after germination means restart */
for(i = avgswc =0; i< v->estab_lyrs;) avgswc += sw->swc[Today][i++];
avgswc /= (RealF) v->estab_lyrs;
if ( LT(avgswc, v->min_swc_estab)  ) {
v->drydays_postgerm++;
v->wetdays_for_estab = 0;
} else {
v->drydays_postgerm=0;
v->wetdays_for_estab++;
}

if ( v->drydays_postgerm > v->max_drydays_postgerm   ||
LT(avgtemp, v->min_temp_estab)                  ||
GT(avgtemp, v->max_temp_estab) ) {
/* too bad: discontinuity in environment, plant dies, start over */
goto LBL_EstabFailed_Exit;
}

v->germ_days++;

if ( v->wetdays_for_estab < v->min_wetdays_for_estab ||
v->germ_days         < v->min_days_germ2estab   ) {
goto LBL_Normal_Exit;  /* no need to zero anything */
}

if ( v->germ_days > v->max_days_germ2estab )
goto LBL_EstabFailed_Exit;


v->estab_doy = SW_Model.doy;
goto LBL_Normal_Exit;
}

LBL_EstabFailed_Exit:
/* allows us to try again if not too late */
v->wetdays_for_estab = 0;
v->germ_days = 0;
v->germd = FALSE;


LBL_Normal_Exit: return;
}

static void _zero_state(unsigned int sppnum) {
/* =================================================== */
/* zero any values that need it for the new growing season */

SW_VEGESTAB_INFO *v = SW_VegEstab.parms[sppnum];

v->no_estab = v->germd = FALSE;
v->estab_doy = v->germ_days = v->drydays_postgerm = 0;
v->wetdays_for_germ = v->wetdays_for_estab = 0;
}

static void _read_spp(const char *infile) {
/* =================================================== */
SW_VEGESTAB_INFO *v;
const int nitems=15;
FILE *f;
int lineno=0 ;
char name[80];  /* only allow 4 char sppnames */

f = OpenFile(infile, "r");

v = SW_VegEstab.parms[ _new_species() ];

while( GetALine(f, inbuf) ) {
switch(lineno) {
case 0:  strcpy(name, inbuf);      break;
case 1:  v->estab_lyrs            = atoi(inbuf); break;
case 2:  v->bars[SW_GERM_BARS]    = fabs(atof(inbuf)); break;
case 3:  v->bars[SW_ESTAB_BARS]   = fabs(atof(inbuf)); break;
case 4:  v->min_pregerm_days      = atoi(inbuf); break;
case 5:  v->max_pregerm_days      = atoi(inbuf); break;
case 6:  v->min_wetdays_for_germ  = atoi(inbuf); break;
case 7:  v->max_drydays_postgerm  = atoi(inbuf); break;
case 8:  v->min_wetdays_for_estab = atoi(inbuf); break;
case 9:  v->min_days_germ2estab   = atoi(inbuf); break;
case 10: v->max_days_germ2estab   = atoi(inbuf); break;
case 11: v->min_temp_germ         = atof(inbuf); break;
case 12: v->max_temp_germ         = atof(inbuf); break;
case 13: v->min_temp_estab        = atof(inbuf); break;
case 14: v->max_temp_estab        = atof(inbuf); break;
}
/* check for valid name first */
if (0==lineno ) {
if (strlen(name) > MAX_SPECIESNAMELEN) {
LogError( logfp, LOGFATAL, "%s: Species name <%s> too long (> %d chars).\n"
"Try again.\n",
infile, name, MAX_SPECIESNAMELEN);
} else {
strcpy(v->sppname, name);
}
}

lineno++;  /*only increments when there's a value */
}

if (lineno < nitems) {
LogError(logfp, LOGFATAL, "%s : Too few input parameters.\n",infile);
}

CloseFile(&f);
}

static void _spp_init(unsigned int sppnum) {
/* =================================================== */
/* initializations performed after acquiring parameters
* after read() or some other function call.
*/
SW_VEGESTAB_INFO *v = SW_VegEstab.parms[sppnum];
SW_LAYER_INFO   **lyr = SW_Site.lyr;
IntU i;

/* The thetas and psis etc should be initialized by now */
/* because init_layers() must be called prior to this routine */
/* (see watereqn() ) */
v->min_swc_germ  = SW_SWC_bars2vol(v->bars[SW_GERM_BARS], 0) * lyr[0]->width;

/* due to possible differences in layer textures and widths, we need
* to average the estab swc across the given layers to peoperly
* compare the actual swc average in the checkit() routine */
v->min_swc_estab = 0.;
for ( i=0; i < v->estab_lyrs; i++)
v->min_swc_estab += SW_SWC_bars2vol(v->bars[SW_ESTAB_BARS], i ) * lyr[i]->width;
v->min_swc_estab /= v->estab_lyrs;


_sanity_check(sppnum);

}



static void _sanity_check(unsigned int sppnum) {
/* =================================================== */
SW_LAYER_INFO **lyr = SW_Site.lyr;
SW_VEGESTAB_INFO *v = SW_VegEstab.parms[sppnum];
double	min_transp_lyrs;

min_transp_lyrs = fmin(SW_Site.n_transp_lyrs_tree, fmin(SW_Site.n_transp_lyrs_shrub, SW_Site.n_transp_lyrs_grass));

if (v->estab_lyrs > min_transp_lyrs ) {
LogError(logfp, LOGFATAL, "%s : Layers requested (estab_lyrs) > (# transpiration layers=%d).",
MyFileName, min_transp_lyrs);
}

if ( v->min_pregerm_days > v->max_pregerm_days ) {
LogError(logfp, LOGFATAL,"%s : First day of germination > last day of germination.",
MyFileName);
}

if ( v->min_wetdays_for_estab > v->max_days_germ2estab ) {
LogError(logfp, LOGFATAL,
"%s : Minimum wetdays after germination (%d) > maximum days allowed for establishment (%d).",
MyFileName, v->min_wetdays_for_estab, v->max_days_germ2estab);
}

if ( v->min_swc_germ < lyr[0]->swc_wiltpt ) {
LogError(logfp, LOGFATAL, "%s : Minimum swc for germination (%.4f) < wiltpoint (%.4f)",
MyFileName, v->min_swc_germ, lyr[0]->swc_wiltpt);
}

if ( v->min_swc_estab < lyr[0]->swc_wiltpt ) {
LogError(logfp, LOGFATAL,"%s : Minimum swc for establishment (%.4f) < wiltpoint (%.4f)",
MyFileName, v->min_swc_estab, lyr[0]->swc_wiltpt);
}


}


static unsigned int _new_species(void){
/* --------------------------------------------------- */
/* first time called with no species defined so
SW_VegEstab.count==0 and SW_VegEstab.parms is
not initialized yet, malloc() required.  For each
species thereafter realloc() is called.
*/
char *me = "SW_VegEstab_newspecies()";
SW_VEGESTAB *v = &SW_VegEstab;

v->parms = (!v->count)
? (SW_VEGESTAB_INFO **)
Mem_Calloc( v->count+1,
sizeof(SW_VEGESTAB_INFO *),
me)
: (SW_VEGESTAB_INFO **)
Mem_ReAlloc(v->parms,
sizeof(SW_VEGESTAB_INFO *)
* v->count+1);
v->parms[v->count] = (SW_VEGESTAB_INFO *)
Mem_Calloc(1, sizeof(SW_VEGESTAB_INFO),
me);
return (++v->count) -1;
}


static void _echo_inits(void) {
/* --------------------------------------------------- */
SW_VEGESTAB_INFO **v = SW_VegEstab.parms ;
SW_LAYER_INFO **lyr  = SW_Site.lyr;
IntU i;
char outstr[2048];

sprintf(errstr,"\n=========================================================\n\n"
"Parameters for the SoilWat Vegetation Establishment Check.\n"
"----------------------------------------------------------\n"
"Number of species to be tested: %d\n", SW_VegEstab.count);


strcpy(outstr, errstr);
for(i=0; i < SW_VegEstab.count; i++) {
sprintf(errstr, "Species: %s\n----------------\n"
"Germination parameters:\n"
"\tMinimum SWP (bars)  : -%.4f\n"
"\tMinimum SWC (cm/cm) : %.4f\n"
"\tMinimum SWC (cm/lyr): %.4f\n"
"\tMinimum temperature : %.1f\n"
"\tMaximum temperature : %.1f\n"
"\tFirst possible day  : %d\n"
"\tLast  possible day  : %d\n"
"\tMinimum consecutive wet days (after first possible day): %d\n"

"Establishment parameters:\n"
"\tNumber of layers affecting successful establishment: %d\n"
"\tMinimum SWP (bars) : -%.4f\n"
"\tMinimum SWC (cm/layer) averaged across top %d layers: %.4f\n"
"\tMinimum temperature : %.1f\n"
"\tMaximum temperature : %.1f\n"
"\tMinimum number of days after germination      : %d\n"
"\tMaximum number of days after germination      : %d\n"
"\tMinimum consecutive wet days after germination: %d\n"
"\tMaximum consecutive dry days after germination: %d\n"
"---------------------------------------------------------------\n\n",

v[i]->sppname, v[i]->bars[SW_GERM_BARS], v[i]->min_swc_germ / lyr[0]->width,
v[i]->min_swc_germ, v[i]->min_temp_germ, v[i]->max_temp_germ,
v[i]->min_pregerm_days, v[i]->max_pregerm_days,
v[i]->min_wetdays_for_germ,

v[i]->estab_lyrs, v[i]->bars[SW_ESTAB_BARS], v[i]->estab_lyrs,
v[i]->min_swc_estab, v[i]->min_temp_estab, v[i]->max_temp_estab,
v[i]->min_days_germ2estab, v[i]->max_days_germ2estab,
v[i]->min_wetdays_for_estab, v[i]->max_drydays_postgerm

);
strcat(outstr, errstr);
}
strcat(outstr, "\n-----------------  End of Establishment Parameters ------------\n");

LogError(logfp, LOGNOTE, outstr);
}
