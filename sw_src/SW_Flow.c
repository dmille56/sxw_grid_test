	/********************************************************/
/********************************************************/
/*	Source file: SW_Flow.c
	Type: module
	Application: SOILWAT - soilwater dynamics simulator
	Purpose: This is the interesting part of the model--
	         the flow of water through the soil.
	
	ORIGINAL NOTES/COMMENTS
********************************************************************
	PURPOSE: Water-flow submodel.  This submodel is a rewrite of a
	        model originally written by William Parton.  It simulates
	         the flow of water through the plant canopy and soil.
	         See "Abiotic Section of ELM", as a reference.
	The subroutines called are listed in the file "subwatr.f"
	
	HISTORY: 
	4/30/92  (not tested - first pass)
	7/2/92   (SLC) Reset swc to 0 if less than 0.  (Due to roundoff)
	           See function "chkzero" in the subwatr.f file.  Each
	           swc is checked for negative values.
	1/17/94  (SLC) Set daily array to zero when no transpiration or
	          evaporation.
	1/27/94  (TLB) Added daily deep drainage variable
	(4/10/2000) -- INITIAL CODING - cwb
	9/21/01	I have to make some transformation from the record-oriented
			structure of the new design to the state array parameter
			structure of the old design.  I thought it would be worth
			trying to rewrite the routines, but with the current version
			of the record layouts, it required too much coupling with the
			other modules, and I refrained for two reasons.  First was
			the time involved, and second was the possiblity of leaving
			the code in SoWat_flow_subs.c as is to facilitate putting
			it into a library.
	10/09/2009	(drs) added snow accumulation, snow sublimation 
				and snow melt to SW_Water_Flow()
	01/12/2010	(drs) turned not-fuctional snow sublimation to snow_sublimation (void)
	02/02/2010	(drs) added to SW_Water_Flow(): saving values for standcrop_int, litter_int and soil_inf
	02/08/2010	(drs) if there is a snowpack then
					- rain infiltrates directly to soil (no vegetation or litter interception of today)
					- no transpiration or evaporation (except evaporation of yesterdays interception)
				only
					- infiltrate water high
					- infiltrate water low
	10/04/2010	(drs) moved call to SW_SWC_adjust_snow() back to SW_WTH_new_day()
	10/19/2010	(drs) added call to hydraulic_redistribution() in SW_Water_Flow() after all the evap/transp/infiltration is computed
						added temporary array lyrHydRed to arrays2records() and records2arrays()
	11/16/2010	(drs) added call to forest_intercepted_water() depending on SW_VegProd.Type..., added forest_h2o to trace forest intercepted water
						renamed standcrop_h2o_qum -> veg_h2o_qum
						renamed totstcr_h2o -> totveg_h2o
	01/03/2011	(drs) changed type of lyrTrRegions[MAX_LAYERS] from int to IntU to avoid warning message that ' pointer targets differ in signedness' in function transp_weighted_avg()
	01/04/2011	(drs) added snowmelt to h2o_for_soil after interception, otherwise last day of snowmelt (when snowpack is gone) wasn't made available and aet became too small
	01/06/2011	(drs) layer drainage was incorrectly calculated if hydraulic redistribution pumped water into a layer below pwp such that its swc was finally higher than pwp -> fixed it by first calculating hydraulic redistribution and then as final step infiltrate_water_low()
	01/06/2011	(drs) call to infiltrate_water_low() has to be the last swc affecting calculation
	02/19/2011	(drs) calculate runoff as adjustment to snowmelt events before infiltration
	02/22/2011	(drs) init aet for the day in SW_Water_Flow(), instead implicitely in evap_litter_veg()
	02/22/2011	(drs) added snowloss to AET
	07/21/2011	(drs) added module variables 'lyrImpermeability' and 'lyrSWCsaturated' and initialize them in records2arrays()
	07/22/2011	(drs) adjusted soil_infiltration for pushed back water to surface (difference in standingWater)
	07/22/2011	(drs) included evaporation from standingWater into evap_litter_veg_surfaceWater() and reduce_rates_by_surfaceEvaporation(): it includes it to AET
	08/22/2011	(drs) in SW_Water_Flow(void): added snowdepth_scale = 1 - snow depth/vegetation height
						- vegetation interception = only when snowdepth_scale > 0, scaled by snowdepth_scale
						- litter interception = only when no snow cover
						- transpiration = only when snowdepth_scale > 0, scaled by snowdepth_scale
						- bare-soil evaporation = only when no snow cover
	09/08/2011	(drs) interception, evaporation from intercepted, E-T partitioning, transpiration, and hydraulic redistribution for each vegetation type (tree, shrub, grass) of SW_VegProd separately, scaled by their fractions
						replaced PET with unevaped in pot_soil_evap() and pot_transp(): to simulate reduction of atmospheric demand underneath canopies
	09/09/2011	(drs) moved transp_weighted_avg() from before infiltration and percolation to directly before call to pot_transp()
	09/21/2011	(drs)	scaled all (potential) evaporation and transpiration flux rates with PET: SW_Flow_lib.h: reduce_rates_by_unmetEvapDemand() is obsolete
	09/26/2011	(drs)	replaced all uses of monthly SW_VegProd and SW_Sky records with the daily replacements
	02/03/2012	(drs)	added variable 'snow_evap_rate': snow loss is part of aet and needs accordingly also to be scaled so that sum of all et rates is not more than pet 
	01/28/2012	(drs)	transpiration can only remove water from soil down to lyrWiltpts (instead lyrSWCmin)
	02/03/2012	(drs)	added 'lyrHalfWiltpts' = 0.5 * SWC at -1.5 MPa
						soil evaporation extracts water down to 'lyrHalfWiltpts' according to the FAO-56 model, e.g., Burt CM, Mutziger AJ, Allen RG, Howell TA (2005) Evaporation Research: Review and Interpretation. Journal of Irrigation and Drainage Engineering, 131, 37-58.
	02/04/2012	(drs)	added 'lyrSWCatSWPcrit_xx' for each vegetation type
						transpiration can only remove water from soil down to 'lyrSWCatSWPcrit_xx' (instead lyrSWCmin)
	02/04/2012	(drs)	snow loss is fixed and can also include snow redistribution etc., so don't scale to PET
	05/25/2012  (DLM) added module level variables lyroldsTemp [MAX_LAYERS] & lyrsTemp [MAX_LAYERS] to keep track of soil temperatures, added lyrbDensity to keep track of the bulk density for each layer
	05/25/2012  (DLM) edited records2arrays(void); & arrays2records(void); functions to move values to / from lyroldsTemp & lyrTemp & lyrbDensity
	05/25/2012  (DLM) added call to soil_temperature function in SW_Water_Flow(void)
*/
/********************************************************/
/********************************************************/

/* =================================================== */
/*                INCLUDES / DEFINES                   */
/* --------------------------------------------------- */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "generic.h"
#include "filefuncs.h"
#include "SW_Defines.h"
#include "SW_Model.h"
#include "SW_Site.h"
#include "SW_SoilWater.h"
#include "SW_Flow_lib.h"
/*#include "SW_VegEstab.h" */
#include "SW_VegProd.h"
#include "SW_Weather.h"
#include "SW_Sky.h"



/* =================================================== */
/*                  Global Variables                   */
/* --------------------------------------------------- */
extern SW_MODEL SW_Model;
extern SW_SITE SW_Site;
extern SW_SOILWAT SW_Soilwat;
extern SW_WEATHER SW_Weather;
extern SW_VEGPROD SW_VegProd;
extern SW_SKY SW_Sky; // 


/* *************************************************** */
/*                Module-Level Variables               */
/* --------------------------------------------------- */

/* temporary arrays for SoWat_flow_subs.c subroutines.
* array indexing in those routines will be from
* zero rather than 1.  see records2arrays().
*/
IntU	lyrTrRegions_Tree   [MAX_LAYERS],
		lyrTrRegions_Shrub   [MAX_LAYERS],
		lyrTrRegions_Grass   [MAX_LAYERS];
RealD	lyrSWC       [MAX_LAYERS],
		lyrDrain     [MAX_LAYERS],
		lyrTransp_Tree    [MAX_LAYERS],
		lyrTransp_Shrub    [MAX_LAYERS],
		lyrTransp_Grass    [MAX_LAYERS],
		lyrTranspCo_Tree  [MAX_LAYERS],
		lyrTranspCo_Shrub  [MAX_LAYERS],
		lyrTranspCo_Grass  [MAX_LAYERS],
		lyrEvap_Tree      [MAX_LAYERS],
		lyrEvap_Shrub      [MAX_LAYERS],
		lyrEvap_Grass      [MAX_LAYERS],
		lyrEvapCo    [MAX_LAYERS],
		lyrFieldCaps [MAX_LAYERS],
		lyrWidths    [MAX_LAYERS],
		lyrWiltpts   [MAX_LAYERS],
		lyrHalfWiltpts   [MAX_LAYERS],
		lyrSWCMins   [MAX_LAYERS],
		lyrSWCatSWPcrit_Tree   [MAX_LAYERS],
		lyrSWCatSWPcrit_Shrub   [MAX_LAYERS],
		lyrSWCatSWPcrit_Grass   [MAX_LAYERS],
		lyrPsis      [MAX_LAYERS],
		lyrThetas    [MAX_LAYERS],
		lyrBetas     [MAX_LAYERS],
		lyrBetaInv   [MAX_LAYERS],
		lyrSumTrCo   [MAX_TRANSP_REGIONS +1],
		lyrHydRed_Tree	 [MAX_LAYERS],
		lyrHydRed_Shrub	 [MAX_LAYERS],
		lyrHydRed_Grass	 [MAX_LAYERS],
		lyrImpermeability	 [MAX_LAYERS],
		lyrSWCsaturated	 [MAX_LAYERS],
		lyroldsTemp [MAX_LAYERS],
		lyrsTemp [MAX_LAYERS],
		lyrbDensity[MAX_LAYERS];

RealD drainout;   /* h2o drained out of deepest layer */

static	RealD	tree_h2o_qum[TWO_DAYS], shrub_h2o_qum[TWO_DAYS], grass_h2o_qum[TWO_DAYS],
				litter_h2o_qum[TWO_DAYS],
				standingWater[TWO_DAYS];	/* water on soil surface if layer below is saturated */;


/* *************************************************** */
/* *************************************************** */
/*            Private functions                        */
/* --------------------------------------------------- */
static void records2arrays(void);
static void arrays2records(void);



/* *************************************************** */
/* *************************************************** */
/*             Public functions                        */
/* --------------------------------------------------- */
/* There is only one external function here and it is
* only called from SW_Soilwat, so it is declared there.
* but the compiler may complain if not predeclared here
* This is a specific option for the compiler and may
* not always occur.
*/
void SW_Water_Flow(void);

/* *************************************************** */
/* *************************************************** */
/*            The Water Flow                           */
/* --------------------------------------------------- */
void SW_Water_Flow(void) {

RealD	swpot_avg_tree, swpot_avg_shrub, swpot_avg_grass,
		soil_evap_tree, soil_evap_shrub, soil_evap_grass,
		soil_evap_rate_tree = 1., soil_evap_rate_shrub = 1., soil_evap_rate_grass = 1.,
		transp_tree, transp_shrub, transp_grass,
		transp_rate_tree = 1., transp_rate_shrub = 1., transp_rate_grass = 1.,
		snow_evap_rate, surface_evap_tree_rate, surface_evap_shrub_rate, surface_evap_grass_rate, surface_evap_litter_rate, surface_evap_standingWater_rate,
		grass_h2o, shrub_h2o, tree_h2o, litter_h2o, litter_h2o_help,
		surface_h2o,
		h2o_for_soil = 0.,
		ppt_toUse,
		snowmelt,
		snowdepth_scale_grass = 1., snowdepth_scale_shrub = 1., snowdepth_scale_tree = 1.,
		rate_help;

int  doy, month;

	doy = SW_Model.doy;     /* base1 */
	month = SW_Model.month; /* base0 */

	records2arrays();

	/* snowdepth scaling */
	SW_Soilwat.snowdepth = SW_SnowDepth(SW_Soilwat.snowpack[Today], SW_Sky.snow_density_daily[doy]);
	/* if snow depth is deeper than vegetation height then
			- rain and snowmelt infiltrates directly to soil (no vegetation or litter interception of today)
		only
			- evaporation of yesterdays interception
			- infiltrate water high
			- infiltrate water low */
	
	if( GT(SW_VegProd.grass.veg_height_daily[doy], 0.) ){
		snowdepth_scale_grass = 1. - SW_Soilwat.snowdepth / SW_VegProd.grass.veg_height_daily[doy];
	} else {
		snowdepth_scale_grass = 1.;
	}
	if( GT(SW_VegProd.grass.veg_height_daily[doy], 0.) ){
		snowdepth_scale_shrub = 1. - SW_Soilwat.snowdepth / SW_VegProd.shrub.veg_height_daily[doy];
	} else {
		snowdepth_scale_shrub = 1.;
	}
	if( GT(SW_VegProd.grass.veg_height_daily[doy], 0.) ){
		snowdepth_scale_tree = 1. - SW_Soilwat.snowdepth / SW_VegProd.tree.veg_height_daily[doy];
	} else {
		snowdepth_scale_tree = 1.;
	}

	/* Interception */	
	ppt_toUse = SW_Weather.now.rain[Today];	/* ppt is partioned into ppt = snow + rain */
	if ( GT(SW_VegProd.fractionTree, 0.) && GT(snowdepth_scale_tree, 0.) ) { /* trees present AND trees not fully covered in snow */
		tree_intercepted_water(	&h2o_for_soil, &tree_h2o, ppt_toUse,
								SW_VegProd.tree.lai_live_daily[doy], snowdepth_scale_tree * SW_VegProd.fractionTree,
								SW_VegProd.tree.veg_intPPT_a, SW_VegProd.tree.veg_intPPT_b, SW_VegProd.tree.veg_intPPT_c, SW_VegProd.tree.veg_intPPT_d);
		ppt_toUse = h2o_for_soil; /* amount of rain that is not intercepted by the forest canopy */
	} else { /* snow depth is more than vegetation height  */
		h2o_for_soil = ppt_toUse;
		tree_h2o = 0.;
	} /* end forest interception */

	if ( GT(SW_VegProd.fractionShrub, 0.) && GT(snowdepth_scale_shrub, 0.) ) {
		shrub_intercepted_water(	&h2o_for_soil, &shrub_h2o, ppt_toUse,
									SW_VegProd.shrub.vegcov_daily[doy], snowdepth_scale_shrub * SW_VegProd.fractionShrub,
									SW_VegProd.shrub.veg_intPPT_a, SW_VegProd.shrub.veg_intPPT_b, SW_VegProd.shrub.veg_intPPT_c, SW_VegProd.shrub.veg_intPPT_d);
		ppt_toUse = h2o_for_soil; /* amount of rain that is not intercepted by the shrub canopy */
	} else {
		shrub_h2o = 0.;
	} /* end shrub interception */

	if ( GT(SW_VegProd.fractionGrass, 0.) && GT(snowdepth_scale_grass, 0.) ) {
		grass_intercepted_water(	&h2o_for_soil, &grass_h2o, ppt_toUse,
									SW_VegProd.grass.vegcov_daily[doy], snowdepth_scale_grass * SW_VegProd.fractionGrass,
									SW_VegProd.grass.veg_intPPT_a, SW_VegProd.grass.veg_intPPT_b, SW_VegProd.grass.veg_intPPT_c, SW_VegProd.grass.veg_intPPT_d);
	} else {
		grass_h2o = 0.;
	} /* end grass interception */


	if ( EQ(SW_Soilwat.snowpack[Today], 0.) ) { /* litter interception only when no snow */
		litter_h2o_help = 0.;
		
		if(GT(SW_VegProd.fractionTree, 0.) ) {
			litter_intercepted_water(	&h2o_for_soil, &litter_h2o,
										SW_VegProd.tree.litter_daily[doy], SW_VegProd.fractionTree,
										SW_VegProd.tree.litt_intPPT_a, SW_VegProd.tree.litt_intPPT_b, SW_VegProd.tree.litt_intPPT_c, SW_VegProd.tree.litt_intPPT_d);
			litter_h2o_help += litter_h2o;	
		}
		
		if(GT(SW_VegProd.fractionShrub, 0.) ) {
			litter_intercepted_water(	&h2o_for_soil, &litter_h2o,
										SW_VegProd.shrub.litter_daily[doy], SW_VegProd.fractionShrub,
										SW_VegProd.shrub.litt_intPPT_a, SW_VegProd.shrub.litt_intPPT_b, SW_VegProd.shrub.litt_intPPT_c, SW_VegProd.shrub.litt_intPPT_d);
			litter_h2o_help += litter_h2o;	
		}
		
		if(GT(SW_VegProd.fractionGrass, 0.) ) {
			litter_intercepted_water(	&h2o_for_soil, &litter_h2o,
										SW_VegProd.grass.litter_daily[doy], SW_VegProd.fractionGrass,
										SW_VegProd.grass.litt_intPPT_a, SW_VegProd.grass.litt_intPPT_b, SW_VegProd.grass.litt_intPPT_c, SW_VegProd.grass.litt_intPPT_d);
			litter_h2o_help += litter_h2o;	
		}
		
		litter_h2o = litter_h2o_help;
	} else {
		litter_h2o = 0.;
	}

	/* Sum cumulative intercepted components */
	SW_Soilwat.tree_int = tree_h2o;
	SW_Soilwat.shrub_int = shrub_h2o;
	SW_Soilwat.grass_int = grass_h2o;
	SW_Soilwat.litter_int = litter_h2o;
	
	tree_h2o_qum[Today]	= tree_h2o_qum[Yesterday] + tree_h2o;
	shrub_h2o_qum[Today]	= shrub_h2o_qum[Yesterday] + shrub_h2o;
	grass_h2o_qum[Today]	= grass_h2o_qum[Yesterday] + grass_h2o;
	litter_h2o_qum[Today] = litter_h2o_qum[Yesterday] + litter_h2o;
	/* End Interception */
	
	/* Surface water */
	standingWater[Today] = standingWater[Yesterday];
	
	/* Soil infiltration = rain+snowmelt - interception, but should be = rain+snowmelt - interception + (throughfall+stemflow) */	
	surface_h2o = standingWater[Today];
	snowmelt = SW_Weather.now.snowmelt[Today];
	snowmelt = fmax( 0., snowmelt * (1. - SW_Weather.pct_runoff/100.) );	/* amount of snowmelt is changed by runon/off as percentage */
	SW_Weather.runoff = SW_Weather.now.snowmelt[Today] - snowmelt;
	h2o_for_soil += snowmelt;  /* if there is snowmelt, it goes un-intercepted to the soil */
	h2o_for_soil += surface_h2o;
	SW_Weather.soil_inf = h2o_for_soil;

	/* Percolation for saturated soil conditions */
	infiltrate_water_high(	lyrSWC,
							lyrDrain,
							&drainout,
							h2o_for_soil,
							SW_Site.n_layers,
							lyrFieldCaps,
							lyrSWCsaturated,
							lyrImpermeability,
							&standingWater[Today]);
	
	SW_Weather.soil_inf -= standingWater[Today]; /* adjust soil_infiltration for pushed back or infiltrated surface water */
	surface_h2o = standingWater[Today];

	/* PET */
	SW_Soilwat.pet = SW_Site.pet_scale
					* petfunc(	doy,
								SW_Weather.now.temp_avg[Today],
								SW_Site.latitude,
								SW_VegProd.grass.albedo*SW_VegProd.fractionGrass + SW_VegProd.shrub.albedo*SW_VegProd.fractionShrub + SW_VegProd.tree.albedo*SW_VegProd.fractionTree,
								SW_Sky.r_humidity_daily[doy],
								SW_Sky.windspeed_daily[doy],
								SW_Sky.cloudcov_daily[doy],
								SW_Sky.transmission_daily[doy]);
	

	/* Tree transpiration & bare-soil evaporation rates */	
	if ( GT(SW_VegProd.fractionTree, 0.) && GT(snowdepth_scale_tree, 0.) ) { /* trees present AND trees not fully covered in snow */
		tree_EsT_partitioning(	&soil_evap_tree,
								&transp_tree,
								SW_VegProd.tree.lai_live_daily[doy],
								SW_VegProd.tree.EsTpartitioning_param);

		if ( EQ(SW_Soilwat.snowpack[Today], 0.) ) { /* bare-soil evaporation only when no snow */
			pot_soil_evap(	&soil_evap_rate_tree,
							SW_Site.n_evap_lyrs,
							lyrEvapCo,
							SW_VegProd.tree.total_agb_daily[doy],
							soil_evap_tree,
							SW_Soilwat.pet,
							SW_Site.evap.xinflec,  SW_Site.evap.slope, SW_Site.evap.yinflec, SW_Site.evap.range,
							lyrWidths,
							lyrSWC,
							SW_VegProd.tree.Es_param_limit);
			soil_evap_rate_tree *= SW_VegProd.fractionTree;
		} else {
			soil_evap_rate_tree = 0.;
		}
		
		transp_weighted_avg(	&swpot_avg_tree,
								SW_Site.n_transp_rgn,
								SW_Site.n_transp_lyrs_tree,
								lyrTrRegions_Tree,
								lyrTranspCo_Tree,
								lyrSWC);							

		pot_transp(	&transp_rate_tree,
					swpot_avg_tree,
					SW_VegProd.tree.biolive_daily[doy],
					SW_VegProd.tree.biodead_daily[doy],
					transp_tree,
					SW_Soilwat.pet,
					SW_Site.transp.xinflec, SW_Site.transp.slope, SW_Site.transp.yinflec, SW_Site.transp.range,
					SW_VegProd.tree.shade_scale, SW_VegProd.tree.shade_deadmax,
					SW_VegProd.tree.tr_shade_effects.xinflec, SW_VegProd.tree.tr_shade_effects.slope, SW_VegProd.tree.tr_shade_effects.yinflec, SW_VegProd.tree.tr_shade_effects.range);
		transp_rate_tree *= snowdepth_scale_tree * SW_VegProd.fractionTree;
	} else {
		soil_evap_rate_tree = 0.;
		transp_rate_tree = 0.;
	}

	/* Shrub transpiration & bare-soil evaporation rates */	
	if ( GT(SW_VegProd.fractionShrub, 0.) && GT(snowdepth_scale_shrub, 0.) ) { /* shrubs present AND shrubs not fully covered in snow */
		shrub_EsT_partitioning(	&soil_evap_shrub,
								&transp_shrub,
								SW_VegProd.shrub.lai_live_daily[doy],
								SW_VegProd.shrub.EsTpartitioning_param);

		if ( EQ(SW_Soilwat.snowpack[Today], 0.) ) { /* bare-soil evaporation only when no snow */
			pot_soil_evap(	&soil_evap_rate_shrub,
							SW_Site.n_evap_lyrs,
							lyrEvapCo,
							SW_VegProd.shrub.total_agb_daily[doy],
							soil_evap_shrub,
							SW_Soilwat.pet,
							SW_Site.evap.xinflec,  SW_Site.evap.slope, SW_Site.evap.yinflec, SW_Site.evap.range,
							lyrWidths,
							lyrSWC,
							SW_VegProd.shrub.Es_param_limit);
			soil_evap_rate_shrub *= SW_VegProd.fractionShrub;
		} else {
			soil_evap_rate_shrub = 0.;
		}
		
		transp_weighted_avg(	&swpot_avg_shrub,
								SW_Site.n_transp_rgn,
								SW_Site.n_transp_lyrs_shrub,
								lyrTrRegions_Shrub,
								lyrTranspCo_Shrub,
								lyrSWC);							

		pot_transp(	&transp_rate_shrub,
					swpot_avg_shrub,
					SW_VegProd.shrub.biolive_daily[doy],
					SW_VegProd.shrub.biodead_daily[doy],
					transp_shrub,
					SW_Soilwat.pet,
					SW_Site.transp.xinflec, SW_Site.transp.slope, SW_Site.transp.yinflec, SW_Site.transp.range,
					SW_VegProd.shrub.shade_scale, SW_VegProd.shrub.shade_deadmax,
					SW_VegProd.shrub.tr_shade_effects.xinflec, SW_VegProd.shrub.tr_shade_effects.slope, SW_VegProd.shrub.tr_shade_effects.yinflec, SW_VegProd.shrub.tr_shade_effects.range);
		transp_rate_shrub *= snowdepth_scale_shrub * SW_VegProd.fractionShrub;
		
	} else {
		soil_evap_rate_shrub = 0.;
		transp_rate_shrub = 0.;
	}

	/* Grass transpiration & bare-soil evaporation rates */	
	if ( GT(SW_VegProd.fractionGrass, 0.) && GT(snowdepth_scale_grass, 0.)) { /* grasses present AND grasses not fully covered in snow */
		grass_EsT_partitioning(	&soil_evap_grass,
								&transp_grass,
								SW_VegProd.grass.lai_live_daily[doy],
								SW_VegProd.grass.EsTpartitioning_param);

		if ( EQ(SW_Soilwat.snowpack[Today], 0.) ) { /* bare-soil evaporation only when no snow */
			pot_soil_evap(	&soil_evap_rate_grass,
							SW_Site.n_evap_lyrs,
							lyrEvapCo,
							SW_VegProd.grass.total_agb_daily[doy],
							soil_evap_grass,
							SW_Soilwat.pet,
							SW_Site.evap.xinflec,  SW_Site.evap.slope, SW_Site.evap.yinflec, SW_Site.evap.range,
							lyrWidths,
							lyrSWC,
							SW_VegProd.grass.Es_param_limit);
			soil_evap_rate_grass *= SW_VegProd.fractionGrass;
		} else {
			soil_evap_rate_grass = 0.;
		}
		
		transp_weighted_avg(	&swpot_avg_grass,
								SW_Site.n_transp_rgn,
								SW_Site.n_transp_lyrs_grass,
								lyrTrRegions_Grass,
								lyrTranspCo_Grass,
								lyrSWC);							

		pot_transp(	&transp_rate_grass,
					swpot_avg_grass,
					SW_VegProd.grass.biolive_daily[doy],
					SW_VegProd.grass.biodead_daily[doy],
					transp_grass,
					SW_Soilwat.pet,
					SW_Site.transp.xinflec, SW_Site.transp.slope, SW_Site.transp.yinflec, SW_Site.transp.range,
					SW_VegProd.grass.shade_scale, SW_VegProd.grass.shade_deadmax,
					SW_VegProd.grass.tr_shade_effects.xinflec, SW_VegProd.grass.tr_shade_effects.slope, SW_VegProd.grass.tr_shade_effects.yinflec, SW_VegProd.grass.tr_shade_effects.range);
		transp_rate_grass *= snowdepth_scale_grass * SW_VegProd.fractionGrass;		
	} else {
		soil_evap_rate_grass = 0.;
		transp_rate_grass = 0.;
	}


	/* Potential evaporation rates of intercepted and surface water */
	surface_evap_tree_rate = tree_h2o_qum[Today];
	surface_evap_shrub_rate = shrub_h2o_qum[Today];
	surface_evap_grass_rate = grass_h2o_qum[Today];
	surface_evap_litter_rate = litter_h2o_qum[Today];
	surface_evap_standingWater_rate = standingWater[Today];
	snow_evap_rate = SW_Weather.now.snowloss[Today];	/* but this is fixed and can also include snow redistribution etc., so don't scale to PET */
	
	/* Scale all (potential) evaporation and transpiration flux rates to PET */
	rate_help = surface_evap_tree_rate + surface_evap_shrub_rate + surface_evap_grass_rate + surface_evap_litter_rate + surface_evap_standingWater_rate + 
				soil_evap_rate_tree + transp_rate_tree + soil_evap_rate_shrub + transp_rate_shrub + soil_evap_rate_grass + transp_rate_grass;
	
	if ( GT(rate_help,  SW_Soilwat.pet) ) {
		rate_help = SW_Soilwat.pet/rate_help;
	
		surface_evap_tree_rate *= rate_help;
		surface_evap_shrub_rate *= rate_help;
		surface_evap_grass_rate *= rate_help;
		surface_evap_litter_rate *= rate_help;
		surface_evap_standingWater_rate *= rate_help;
		soil_evap_rate_tree *= rate_help;
		transp_rate_tree *= rate_help;
		soil_evap_rate_shrub *= rate_help;
		transp_rate_shrub *= rate_help;
		soil_evap_rate_grass *= rate_help;
		transp_rate_grass *= rate_help;
	}
	
	/* Start adding components to AET */
	SW_Soilwat.aet = 0.;	/* init aet for the day */
	SW_Soilwat.aet += snow_evap_rate;

	/* Evaporation of intercepted and surface water */
	evap_fromSurface(	&tree_h2o_qum[Today], &surface_evap_tree_rate, &SW_Soilwat.aet);
	evap_fromSurface(	&shrub_h2o_qum[Today], &surface_evap_shrub_rate, &SW_Soilwat.aet);
	evap_fromSurface(	&grass_h2o_qum[Today], &surface_evap_grass_rate, &SW_Soilwat.aet);
	evap_fromSurface(	&litter_h2o_qum[Today], &surface_evap_litter_rate, &SW_Soilwat.aet);
	evap_fromSurface(	&standingWater[Today], &surface_evap_standingWater_rate, &SW_Soilwat.aet);
							
	SW_Soilwat.tree_evap = surface_evap_tree_rate;
	SW_Soilwat.shrub_evap = surface_evap_shrub_rate;
	SW_Soilwat.grass_evap = surface_evap_grass_rate;
	SW_Soilwat.litter_evap    = surface_evap_litter_rate;
	SW_Soilwat.surfaceWater_evap = surface_evap_standingWater_rate;

	/* Tree transpiration and bare-soil evaporation */
	if ( GT(SW_VegProd.fractionTree, 0.) && GT(snowdepth_scale_tree, 0.) ) {
		/* remove bare-soil evap from swc */
		remove_from_soil(	lyrSWC,
							lyrEvap_Tree,
							&SW_Soilwat.aet,
							SW_Site.n_evap_lyrs,
							lyrEvapCo,
							soil_evap_rate_tree,
							lyrHalfWiltpts);

		/* remove transp from swc */
		remove_from_soil(	lyrSWC,
							lyrTransp_Tree,
							&SW_Soilwat.aet,
							SW_Site.n_transp_lyrs_tree,
							lyrTranspCo_Tree,
							transp_rate_tree,
							lyrSWCatSWPcrit_Tree);		
	} else {
		/* Set daily array to zero, no evaporation or transpiration */
		LyrIndex i;
		for(i=0; i< SW_Site.n_evap_lyrs;   ) lyrEvap_Tree[i++]   = 0.;
		for(i=0; i< SW_Site.n_transp_lyrs_tree; ) lyrTransp_Tree[i++] = 0.;
	}

	/* Shrub transpiration and bare-soil evaporation */
	if ( GT(SW_VegProd.fractionShrub, 0.) && GT(snowdepth_scale_shrub, 0.) ) {
		/* remove bare-soil evap from swc */
		remove_from_soil(	lyrSWC,
							lyrEvap_Shrub,
							&SW_Soilwat.aet,
							SW_Site.n_evap_lyrs,
							lyrEvapCo,
							soil_evap_rate_shrub,
							lyrHalfWiltpts);

		/* remove transp from swc */
		remove_from_soil(	lyrSWC,
							lyrTransp_Shrub,
							&SW_Soilwat.aet,
							SW_Site.n_transp_lyrs_shrub,
							lyrTranspCo_Shrub,
							transp_rate_shrub,
							lyrSWCatSWPcrit_Shrub);
	} else {
		/* Set daily array to zero, no evaporation or transpiration */
		LyrIndex i;
		for(i=0; i< SW_Site.n_evap_lyrs;   ) lyrEvap_Shrub[i++]   = 0.;
		for(i=0; i< SW_Site.n_transp_lyrs_shrub; ) lyrTransp_Shrub[i++] = 0.;
	}

	/* Grass transpiration & bare-soil evaporation */	
	if ( GT(SW_VegProd.fractionGrass, 0.) && GT(snowdepth_scale_grass, 0.) ) {
		/* remove bare-soil evap from swc */
		remove_from_soil(	lyrSWC,
							lyrEvap_Grass,
							&SW_Soilwat.aet,
							SW_Site.n_evap_lyrs,
							lyrEvapCo,
							soil_evap_rate_grass,
							lyrHalfWiltpts);

		/* remove transp from swc */
		remove_from_soil(	lyrSWC,
							lyrTransp_Grass,
							&SW_Soilwat.aet,
							SW_Site.n_transp_lyrs_grass,
							lyrTranspCo_Grass,
							transp_rate_grass,
							lyrSWCatSWPcrit_Grass);
	} else {
		/* Set daily array to zero, no evaporation or transpiration */
		LyrIndex i;
		for(i=0; i< SW_Site.n_evap_lyrs;   ) lyrEvap_Grass[i++]   = 0.;
		for(i=0; i< SW_Site.n_transp_lyrs_grass; ) lyrTransp_Grass[i++] = 0.;
	}

	/* Hydraulic redistribution */
	if (SW_VegProd.grass.flagHydraulicRedistribution && GT(SW_VegProd.fractionGrass, 0.) && GT(SW_VegProd.grass.biolive_daily[doy], 0.) ) {
		hydraulic_redistribution(	lyrSWC, lyrWiltpts, lyrTranspCo_Grass, lyrHydRed_Grass,
									SW_Site.n_layers,
									SW_VegProd.grass.maxCondroot, SW_VegProd.grass.swp50, SW_VegProd.grass.shapeCond,
									SW_VegProd.fractionGrass);
	}
	if (SW_VegProd.shrub.flagHydraulicRedistribution && GT(SW_VegProd.fractionShrub, 0.) && GT(SW_VegProd.shrub.biolive_daily[doy], 0.) ) {
		hydraulic_redistribution(	lyrSWC, lyrWiltpts, lyrTranspCo_Shrub, lyrHydRed_Shrub,
									SW_Site.n_layers,
									SW_VegProd.shrub.maxCondroot, SW_VegProd.shrub.swp50, SW_VegProd.shrub.shapeCond,
									SW_VegProd.fractionShrub);
	}
	if (SW_VegProd.tree.flagHydraulicRedistribution && GT(SW_VegProd.fractionTree, 0.) && GT(SW_VegProd.tree.biolive_daily[doy], 0.) ) {
		hydraulic_redistribution(	lyrSWC, lyrWiltpts, lyrTranspCo_Tree, lyrHydRed_Tree,
									SW_Site.n_layers,
									SW_VegProd.tree.maxCondroot, SW_VegProd.tree.swp50, SW_VegProd.tree.shapeCond,
									SW_VegProd.fractionTree);
	}

	/* Calculate percolation for unsaturated soil water conditions. */
	/* 01/06/2011	(drs) call to infiltrate_water_low() has to be the last swc affecting calculation */

	infiltrate_water_low(	lyrSWC,
							lyrDrain,
							&drainout,
							SW_Site.n_layers,
							SW_Site.slow_drain_coeff,
							SLOW_DRAIN_DEPTH,
							lyrFieldCaps,
							lyrWidths,
							lyrSWCMins,
							lyrSWCsaturated,
							lyrImpermeability,
							&standingWater[Today]);
							
	SW_Soilwat.surfaceWater = standingWater[Today];
	
	/* Soil Temperature starts here */
	
	double biomass; // computing the standing crop biomass real quickly to condense the call to soil_temperature
	biomass = SW_VegProd.grass.biomass_daily[doy] * SW_VegProd.fractionGrass + 
				SW_VegProd.shrub.biomass_daily[doy] * SW_VegProd.fractionShrub +
				 SW_VegProd.tree.biolive_daily[doy] * SW_VegProd.fractionTree; // changed to exclude tree biomass, b/c it was breaking the soil_temperature function
	
	// soil_temperature function computes the soil temp for each layer and stores it in lyrsTemp
	// doesn't affect SWC at all, but needs it for the calculation, so therefore the temperature is the last calculation done
	if(SW_Site.use_soil_temp)
		soil_temperature( SW_Weather.now.temp_avg[Today], SW_Soilwat.pet, SW_Soilwat.aet, biomass,
					  lyrSWC, lyrbDensity, lyrWidths, 
					  lyroldsTemp, lyrsTemp, SW_Site.n_layers,
					  lyrFieldCaps, lyrWiltpts, SW_Site.bmLimiter,
					  SW_Site.t1Param1, SW_Site.t1Param2, SW_Site.t1Param3,
					  SW_Site.csParam1, SW_Site.csParam2, SW_Site.shParam, 
					  SW_Soilwat.snowpack[Today], SW_Site.meanAirTemp /*SW_Weather.hist.temp_year_avg*/,
					  SW_Site.stDeltaX, SW_Site.stMaxDepth, SW_Site.stNRGR);
		
	/* Soil Temperature ends here */

	/* Move local values into main arrays */
	arrays2records();

	standingWater[Yesterday] = standingWater[Today];
	litter_h2o_qum[Yesterday] = litter_h2o_qum[Today];
	tree_h2o_qum[Yesterday] = tree_h2o_qum[Today];
	shrub_h2o_qum[Yesterday] = shrub_h2o_qum[Today];
	grass_h2o_qum[Yesterday] = grass_h2o_qum[Today];

}  /* END OF WATERFLOW */



static void records2arrays(void) {
/* some values are unchanged by the water subs but
* are still required in an array format.
* Also, some arrays start out empty and are
* filled during the water flow.
* See arrays2records() for the modified arrays.
*
* 3/24/2003 - cwb - when running with steppe, the
*       static variable firsttime would only be set once
*       so the firsttime tasks were done only the first
*       year, but what we really want with stepwat is
*       to firsttime tasks on the first day of each year.
* 1-Oct-03 (cwb) - Removed references to sum_transp_coeff.
*       see also Site.c.
*/
	LyrIndex i;
	
	ForEachSoilLayer(i) {
		lyrSWC[i]         = SW_Soilwat.swc[Today][i];
		lyroldsTemp[i]	  = SW_Soilwat.sTemp[i];
	}
	
	if (SW_Model.doy == SW_Model.firstdoy) {
	ForEachSoilLayer(i) {
		lyrTrRegions_Tree[i] = SW_Site.lyr[i]->my_transp_rgn_tree;
		lyrTrRegions_Shrub[i] = SW_Site.lyr[i]->my_transp_rgn_shrub;
		lyrTrRegions_Grass[i] = SW_Site.lyr[i]->my_transp_rgn_grass;
		lyrFieldCaps[i] = SW_Site.lyr[i]->swc_fieldcap;
		lyrWidths[i]    = SW_Site.lyr[i]->width;
		lyrWiltpts[i]   = SW_Site.lyr[i]->swc_wiltpt;
		lyrHalfWiltpts[i]   = SW_Site.lyr[i]->swc_wiltpt/2.;
		lyrSWCatSWPcrit_Tree[i]	= SW_Site.lyr[i]->swc_atSWPcrit_tree;
		lyrSWCatSWPcrit_Shrub[i]	= SW_Site.lyr[i]->swc_atSWPcrit_shrub;
		lyrSWCatSWPcrit_Grass[i]	= SW_Site.lyr[i]->swc_atSWPcrit_grass;
		lyrSWCMins[i]   = SW_Site.lyr[i]->swc_min;
		lyrPsis[i]      = SW_Site.lyr[i]->psis;
		lyrThetas[i]    = SW_Site.lyr[i]->thetas;
		lyrBetas[i]     = SW_Site.lyr[i]->b;
		lyrBetaInv[i]   = SW_Site.lyr[i]->binverse;
		lyrImpermeability[i]	= SW_Site.lyr[i]->impermeability;
		lyrSWCsaturated[i]	= SW_Site.lyr[i]->swc_saturated;
		lyrbDensity[i] = SW_Site.lyr[i]->bulk_density;
	}
	
	ForEachTreeTranspLayer(i) {
		lyrTranspCo_Tree[i]  = SW_Site.lyr[i]->transp_coeff_tree;
	}

	ForEachShrubTranspLayer(i) {	
		lyrTranspCo_Shrub[i]  = SW_Site.lyr[i]->transp_coeff_shrub;
	}
	
	ForEachGrassTranspLayer(i) {	
		lyrTranspCo_Grass[i]  = SW_Site.lyr[i]->transp_coeff_grass;
	}
	
	ForEachEvapLayer(i)
		lyrEvapCo[i]    = SW_Site.lyr[i]->evap_coeff;
	
	}  /* end firsttime stuff */

}


static void arrays2records(void) {
/* move output quantities from arrays to
* the appropriate records.
*/
	LyrIndex i;
	
	ForEachSoilLayer(i) {
		SW_Soilwat.swc[Today][i]    = lyrSWC[i];
		SW_Soilwat.drain[i]         = lyrDrain[i];
		SW_Soilwat.hydred_tree[i]		= lyrHydRed_Tree[i];
		SW_Soilwat.hydred_shrub[i]		= lyrHydRed_Shrub[i];
		SW_Soilwat.hydred_grass[i]		= lyrHydRed_Grass[i];
		SW_Soilwat.sTemp[i]	= lyrsTemp[i];
	}
	
	if (SW_Site.deepdrain)
		SW_Soilwat.swc[Today][SW_Site.deep_lyr] = drainout;
	
	
	ForEachTreeTranspLayer(i) {
		SW_Soilwat.transpiration_tree[i] = lyrTransp_Tree[i];
	}

	ForEachShrubTranspLayer(i) {
		SW_Soilwat.transpiration_shrub[i] = lyrTransp_Shrub[i];
	}
	
	ForEachGrassTranspLayer(i) {
		SW_Soilwat.transpiration_grass[i] = lyrTransp_Grass[i];
	}
	
	ForEachEvapLayer(i) {
		SW_Soilwat.evaporation[i]   = lyrEvap_Tree[i] + lyrEvap_Shrub[i] + lyrEvap_Grass[i];
	}

}
