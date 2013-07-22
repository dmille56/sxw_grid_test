/********************************************************/
/********************************************************/
/*  Source file: SW_Site.h
	Type: header
	Application: SOILWAT - soilwater dynamics simulator
	Purpose: Define a structure to hold parameters that
	         are read in Site::read() and passed to
	         Layers::init(*).  There are a couple of
	         parameters that belong at the site level
	         but it also makes sense to keep the layer
	         parms in the same site input file.
	History:
	   (8/28/01) -- INITIAL CODING - cwb
	   1-Oct-03  (cwb) Removed variables sum_evap_coeff and
	             *sum_transp_coeff.  sum_evap_coeff doesn't
	             do anything since it's always 1.0 (due
	             to normalization) and sum_transp_coeff[]
	             can easily be calculated in the only
	             function where it applies: transp_weighted_avg().
	(10/12/2009) - (drs) added altitude
	11/02/2010	(drs) added snow parameters to SW_SITE to be read in from siteparam.in
	10/19/2010	(drs) added Bool HydraulicRedistribution flag and RealD maxCondroot, swp50, and shapeCond parameters to the structure SW_SITE
	07/20/2011	(drs) added RealD impermeability to struct SW_LAYER_INFO: fraction of how impermeable a layer is (0=permeable, 1=impermeable)
 					added RealD swc_saturated to struct SW_LAYER_INFO: saturated soil water content (cm) * width
	09/08/2011	(drs) moved all hydraulic redistribution parameters to SW_VegProd.h struct VegType
	09/09/2011	(drs) added RealD transp_coeff_xx for each vegetation type (tree, shrub, grass) to struct SW_LAYER_INFO
	09/09/2011	(drs) added RealD my_transp_rgn_xx for each vegetation type (tree, shrub, grass) to struct SW_LAYER_INFO
						added RealD n_transp_lyrs_xx for each vegetation type (tree, shrub, grass) to struct SW_SITE
	09/15/2011	(drs)	deleted RealD albedo in struct SW_SITE and moved it to SW_VegProd.h to make input vegetation type dependent
	02/04/2012	(drs)	added Reald swc_atSWPcrit_xx for each vegetation type (tree, shrub, grass) to struct SW_LAYER_INFO: swc at the critical soil water potential
	05/24/2012  (DLM) added variables for Soil Temperature constants to SW_SITE struct
	05/25/2012  (DLM) added variable sTemp to SW_LAYER_INFO struct to keep track of the soil temperature for each soil layer
	05/30/2012  (DLM) added stDeltaX variable for soil_temperature function to SW_SITE struct
	05/31/2012  (DLM) added use_soil_temp, stMaxDepth, stNRGR variables to SW_SITE struct
*/
/********************************************************/
/********************************************************/
#ifndef SW_SITE_H
#define SW_SITE_H

#include "SW_Defines.h"

typedef unsigned int
  LyrIndex;

typedef struct {

    RealD width     , /* width of the soil layer (cm) */
          bulk_density , /* g/m**3 */
          evap_coeff ,   /* prop. of total soil evap from this layer */
          transp_coeff_tree, transp_coeff_shrub, transp_coeff_grass , /* prop. of total transp from this layer    */
          pct_sand   ,   /* percent sand in this layer (frac)        */
          pct_clay   ,   /* percent clay in this layer (frac)        */
          swc_fieldcap , /* field_cap * width                        */
          swc_wiltpt ,   /* wilting_pt * width                       */
          swc_wet    ,   /* swc considered "wet" (cm) *width         */
          swc_init   ,   /* start the model at this swc (cm) *width  */
          swc_min    ,   /* swc cannot go below this (cm) *width     */
          swc_saturated,	/* saturated soil water content (cm) * width */
          impermeability,   /* fraction of how impermeable a layer is (0=permeable, 1=impermeable)    */
          swc_atSWPcrit_tree, swc_atSWPcrit_shrub, swc_atSWPcrit_grass,	/* swc at the critical soil water potential */

          thetas   ,  /* This group is parameters for */
          psis     ,  /* Cosby et al. (1982) SWC <-> SWP */
          b        ,  /* conversion functions. */
          binverse ,
          
          sTemp; /* initial soil temperature for each soil layer */

    LyrIndex   my_transp_rgn_tree, my_transp_rgn_shrub, my_transp_rgn_grass;  /* which transp zones from Site am I in? */

} SW_LAYER_INFO;

typedef struct {

    Bool reset_yr,   /* 1: reset values at start of each year */
         deepdrain,  /* 1: allow drainage into deepest layer  */
         use_soil_temp; /* whether or not to do soil_temperature calculations */
    LyrIndex  n_layers,   /* total number of soil layers */
              n_transp_rgn, /* soil layers are grouped into n transp. regions */
              n_evap_lyrs,  /* number of layers in which evap is possible */
              n_transp_lyrs_tree, n_transp_lyrs_shrub, n_transp_lyrs_grass,  /* layer index of deepest transp. region       */
              deep_lyr;    /* index of deep drainage layer if deepdrain, 0 otherwise */
    RealD slow_drain_coeff, /* low soil water drainage coefficient   */
		pet_scale,  /* changes relative effect of PET calculation */
		latitude,   /* latitude of the site (radians)        */
		altitude,	/* altitude a.s.l (m) of the site */
					/* SWAT2K model parameters : Neitsch S, Arnold J, Kiniry J, Williams J. 2005. Soil and water assessment tool (SWAT) theoretical documentation. version 2005. Blackland Research Center, Texas Agricultural Experiment Station: Temple, TX. */
		TminAccu2,	/* Avg. air temp below which ppt is snow ( C) */
		TmaxCrit,	/* Snow temperature at which snow melt starts ( C) */
		lambdasnow,	/* Relative contribution of avg. air temperature to todays snow temperture vs. yesterday's snow temperature (0-1) */
		RmeltMin,	/* Minimum snow melt rate on winter solstice (cm/day/C) */
		RmeltMax,	/* Maximum snow melt rate on summer solstice (cm/day/C) */
		t1Param1,	/* Soil temperature constants */
		t1Param2,	/* t1Params are the parameters for the avg daily temperature at the top of the soil (T1) equation */
		t1Param3,
		csParam1,	/* csParams are the parameters for the soil thermal conductivity (cs) equation */
		csParam2,
		shParam,	/* shParam is the parameter for the specific heat capacity equation */
		bmLimiter,	/* bmLimiter is the biomass limiter constant, for use in the T1 equation */
		meanAirTemp, /* meanAirTemp is the mean air temperature for last year, it's a constant read in from the siteparams.in file used in soil_temperature function */
		stDeltaX,	 /* for the soil_temperature function, deltaX is the distance between profile points (default: 15) */
		stMaxDepth;  /* for the soil_temperature function, the maxDepth of the regression function */
		
	unsigned int stNRGR; /* number of regressions, for the soil_temperature function */ 
		
   /* params for tanfunc rate calculations for evap and transp. */
   /* tanfunc() creates a logistic-type graph if shift is positive,
    * the graph has a negative slope, if shift is 0, slope is positive.
    */
   tanfunc_t evap, transp;

   SW_LAYER_INFO **lyr; /* one struct per soil layer pointed to by   */
                        /* a dynamically allocated block of pointers */

 } SW_SITE;

void SW_SIT_read(void);
void SW_SIT_construct(void);

/* these used to be in Layers */
void SW_SIT_clear_layers(void);

#ifdef DEBUG_MEM
void SW_SIT_SetMemoryRefs(void);
#endif


#endif
