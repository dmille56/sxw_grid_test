#=======================================================================
#@V@:Note: File automatically generated by VIDE: (17:03:21 08 Jan 2003) (gcc).
#  This file rcommonegenerated each time you run VIDE, so save under a
#    new name if you hand edit, or it will be overwritten.
#=======================================================================

# DLM : 08-01-2012, after a TON of editing I finally got this old relic to work with the sw_v23 source.  It shouldn't be too hard to add new files as needed, or to change the compiler/compiler options.  DO NOT LOSE THIS FILE, or you will be screwed.
# I have no idea what the hell that VIDE thing is, but I'll leave it in here I guess for old times sake, or if anyone else has a clue.

# to change the soilwat source folder replace all 'sw_src' with 'yourfolder', without the quotes ofcourse.  If your changing the folder make sure that there is an empty folder of the same name in the 'obj' folder as well to hold the sw object files.

# to compile on JANUS, you would want to change the value of CC from gcc to mpicc, and also possibly change the C_FLAGS

# Standard defines:
CC  	=	gcc

WRES	=	windres #no idea what this was actually used for since it doesn't do anything...

HOMEV	=	
VPATH	=	$(HOMEV)/include #no idea what this was used for either...
oDir	=	./obj
Bin	=	.
Src	=	.
libDirs	=	-Lsw_src

incDirs	=	-Isw_src

LIBS	=	
C_FLAGS	=	-g -02 -m32 -Wstrict-prototypes -Wmissing-prototypes -Wimplicit -Wunused -Wformat -Wredundant-decls -Wcast-align\
	-DSTEPWAT

SRCS	=\
	$(Src)/sw_src/filefuncs.c\
	$(Src)/sw_src/generic.c\
	$(Src)/sw_src/mymemory.c\
	$(Src)/sw_src/Times.c\
	$(Src)/sw_src/rands.c\
	$(Src)/sxw_environs.c\
	$(Src)/sxw.c\
	$(Src)/sxw_resource.c\
	$(Src)/sxw_soilwat.c\
	$(Src)/sw_src/SW_Markov.c\
	$(Src)/sw_src/SW_Weather.c\
	$(Src)/sw_src/SW_Files.c\
	$(Src)/sw_src/SW_Model.c\
	$(Src)/sw_src/SW_Output.c\
	$(Src)/sw_src/SW_Site.c\
	$(Src)/sw_src/SW_Sky.c\
	$(Src)/sw_src/SW_VegProd.c\
	$(Src)/sw_src/SW_Flow_lib.c\
	$(Src)/sw_src/SW_Flow.c\
	$(Src)/ST_environs.c\
	$(Src)/sw_src/SW_VegEstab.c\
	$(Src)/sw_src/SW_Control.c\
	$(Src)/sw_src/SW_SoilWater.c\
	$(Src)/ST_indivs.c\
	$(Src)/ST_main.c\
	$(Src)/ST_mortality.c\
	$(Src)/ST_output.c\
	$(Src)/ST_params.c\
	$(Src)/ST_resgroups.c\
	$(Src)/ST_species.c\
	$(Src)/ST_stats.c\
	$(Src)/ST_grid.c\
	#$(Src)/sxw_tester.c

EXOBJS	=\
	$(oDir)/sw_src/filefuncs.o\
	$(oDir)/sw_src/generic.o\
	$(oDir)/sw_src/mymemory.o\
	$(oDir)/sw_src/Times.o\
	$(oDir)/sw_src/rands.o\
	$(oDir)/sxw.o\
	$(oDir)/sxw_resource.o\
	$(oDir)/sxw_soilwat.o\
	$(oDir)/sw_src/SW_Markov.o\
	$(oDir)/sw_src/SW_Weather.o\
	$(oDir)/sw_src/SW_Files.o\
	$(oDir)/sw_src/SW_Model.o\
	$(oDir)/sw_src/SW_Output.o\
	$(oDir)/sw_src/SW_Site.o\
	$(oDir)/sw_src/SW_Sky.o\
	$(oDir)/sw_src/SW_VegProd.o\
	$(oDir)/sw_src/SW_Flow_lib.o\
	$(oDir)/sw_src/SW_Flow.o\
	$(oDir)/ST_environs.o\
	$(oDir)/sw_src/SW_VegEstab.o\
	$(oDir)/sw_src/SW_Control.o\
	$(oDir)/sw_src/SW_SoilWater.o\
	$(oDir)/ST_indivs.o\
	$(oDir)/ST_main.o\
	$(oDir)/ST_mortality.o\
	$(oDir)/ST_output.o\
	$(oDir)/ST_params.o\
	$(oDir)/ST_resgroups.o\
	$(oDir)/ST_species.o\
	$(oDir)/ST_stats.o\
	$(oDir)/sxw_environs.o\
	$(oDir)/ST_grid.o\
	#$(oDir)/sxw_tester.o

ALLOBJS	=	$(EXOBJS)
ALLBIN	=	$(Bin)/stepwat
ALLTGT	=	$(Bin)/stepwat

# User defines:

#@# Targets follow ---------------------------------

all:	$(ALLTGT)
		cp stepwat testing/stepwat
		dsymutil testing/stepwat

objs:	$(ALLOBJS)

cleanobjs:
	rm -f $(ALLOBJS)

cleanbin:
	rm -f $(ALLBIN)

clean:	cleanobjs cleanbin

cleanall:	cleanobjs cleanbin

#@# User Targets follow ---------------------------------


#@# Dependency rules follow -----------------------------

$(Bin)/stepwat: $(EXOBJS)
	$(CC) -g -02 -m32 -o $(Bin)/stepwat $(EXOBJS) $(incDirs) $(libDirs) $(LIBS)
	
$(oDir)/sw_src/filefuncs.o: sw_src/filefuncs.c sw_src/filefuncs.h \
 sw_src/generic.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/sw_src/generic.o: sw_src/generic.c sw_src/generic.h \
 sw_src/filefuncs.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/sw_src/mymemory.o: sw_src/mymemory.c sw_src/generic.h sw_src/myMemory.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/sw_src/Times.o: sw_src/Times.c sw_src/generic.h sw_src/myMemory.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/sw_src/rands.o: sw_src/rands.c sw_src/generic.h sw_src/rands.h \
 sw_src/myMemory.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/sxw_environs.o: sxw_environs.c sw_src/generic.h ST_steppe.h \
 ST_defines.h ST_structs.h ST_functions.h sw_src/SW_Defines.h sxw.h \
 sw_src/SW_Times.h sxw_module.h sw_src/SW_Model.h sw_src/SW_Site.h sw_src/SW_SoilWater.h \
 sw_src/SW_Weather.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/sxw.o: sxw.c sw_src/generic.h sw_src/filefuncs.h \
 sw_src/myMemory.h ST_steppe.h ST_defines.h ST_structs.h \
 ST_functions.h ST_globals.h sw_src/SW_Defines.h sxw.h sw_src/SW_Times.h sxw_funcs.h \
 sxw_module.h sw_src/SW_Control.h sw_src/SW_Model.h sw_src/SW_Site.h sw_src/SW_SoilWater.h \
 sw_src/SW_Files.h sw_src/SW_VegProd.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/sxw_resource.o: sxw_resource.c sw_src/generic.h \
 sw_src/filefuncs.h sw_src/myMemory.h ST_steppe.h \
 ST_defines.h ST_structs.h ST_functions.h ST_globals.h sw_src/SW_Defines.h \
 sxw.h sw_src/SW_Times.h sxw_module.h sxw_vars.h sw_src/SW_Control.h sw_src/SW_Model.h \
 sw_src/SW_Site.h sw_src/SW_SoilWater.h sw_src/SW_VegProd.h sw_src/SW_Files.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/sxw_soilwat.o: sxw_soilwat.c sw_src/generic.h \
 sw_src/filefuncs.h sw_src/myMemory.h ST_steppe.h \
 ST_defines.h ST_structs.h ST_functions.h ST_globals.h sw_src/SW_Defines.h \
 sxw.h sw_src/SW_Times.h sxw_module.h sw_src/SW_Control.h sw_src/SW_Model.h sw_src/SW_Site.h \
 sw_src/SW_SoilWater.h sw_src/SW_VegProd.h sw_src/SW_Files.h sxw_vars.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/sw_src/SW_Markov.o: sw_src/SW_Markov.c sw_src/generic.h \
 sw_src/filefuncs.h sw_src/rands.h sw_src/myMemory.h \
 sw_src/SW_Defines.h sw_src/SW_Files.h sw_src/SW_Weather.h sw_src/SW_Times.h sw_src/SW_Model.h \
 sw_src/SW_Markov.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/sw_src/SW_Weather.o: sw_src/SW_Weather.c sw_src/generic.h \
 sw_src/filefuncs.h sw_src/myMemory.h sw_src/SW_Defines.h \
 sw_src/SW_Files.h sw_src/SW_Model.h sw_src/SW_Times.h sw_src/SW_Output.h sw_src/SW_SoilWater.h \
 sw_src/SW_Weather.h sw_src/SW_Markov.h sw_src/SW_Sky.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/sw_src/SW_Files.o: sw_src/SW_Files.c sw_src/generic.h sw_src/filefuncs.h \
 sw_src/myMemory.h sw_src/SW_Defines.h sw_src/SW_Files.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/sw_src/SW_Model.o: sw_src/SW_Model.c sw_src/generic.h sw_src/filefuncs.h \
 sw_src/rands.h sw_src/SW_Defines.h sw_src/SW_Files.h sw_src/SW_Weather.h sw_src/SW_Times.h \
 sw_src/SW_Site.h sw_src/SW_SoilWater.h sw_src/SW_Model.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/sw_src/SW_Output.o: sw_src/SW_Output.c sw_src/generic.h \
 sw_src/filefuncs.h sw_src/myMemory.h sw_src/SW_Defines.h \
 sw_src/SW_Files.h sw_src/SW_Model.h sw_src/SW_Times.h sw_src/SW_Site.h sw_src/SW_SoilWater.h sw_src/SW_Output.h \
 sw_src/SW_Weather.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/sw_src/SW_Site.o: sw_src/SW_Site.c sw_src/generic.h sw_src/filefuncs.h \
 sw_src/myMemory.h sw_src/SW_Defines.h sw_src/SW_Files.h sw_src/SW_Site.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/sw_src/SW_Sky.o: sw_src/SW_Sky.c sw_src/generic.h sw_src/filefuncs.h \
 sw_src/SW_Defines.h sw_src/SW_Files.h sw_src/SW_Sky.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/sw_src/SW_VegProd.o: sw_src/SW_VegProd.c sw_src/generic.h \
 sw_src/filefuncs.h sw_src/SW_Defines.h sw_src/SW_Files.h sw_src/SW_Times.h \
 sw_src/SW_VegProd.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/sw_src/SW_Flow.o: sw_src/SW_Flow.c sw_src/generic.h sw_src/filefuncs.h \
 sw_src/SW_Defines.h sw_src/SW_Model.h sw_src/SW_Times.h sw_src/SW_Site.h sw_src/SW_SoilWater.h \
 sw_src/SW_Flow_lib.h sw_src/SW_VegProd.h sw_src/SW_Weather.h sw_src/SW_Sky.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/ST_environs.o: ST_environs.c ST_steppe.h ST_defines.h \
 sw_src/generic.h ST_structs.h ST_functions.h ST_globals.h \
 sw_src/rands.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/sw_src/SW_Control.o: sw_src/SW_Control.c sw_src/generic.h \
 sw_src/filefuncs.h sw_src/rands.h sw_src/SW_Defines.h sw_src/SW_Files.h \
 sw_src/SW_Control.h sw_src/SW_Model.h sw_src/SW_Times.h sw_src/SW_Output.h sw_src/SW_Site.h \
 sw_src/SW_SoilWater.h sw_src/SW_VegProd.h sw_src/SW_Weather.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/sw_src/SW_SoilWater.o: sw_src/SW_SoilWater.c sw_src/generic.h \
 sw_src/filefuncs.h sw_src/myMemory.h sw_src/SW_Defines.h \
 sw_src/SW_Files.h sw_src/SW_Model.h sw_src/SW_Times.h sw_src/SW_Site.h sw_src/SW_SoilWater.h sw_src/SW_Output.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/ST_indivs.o: ST_indivs.c ST_steppe.h ST_defines.h \
 sw_src/generic.h ST_structs.h ST_functions.h ST_globals.h \
 sw_src/filefuncs.h sw_src/myMemory.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/ST_main.o: ST_main.c ST_steppe.h ST_defines.h sw_src/generic.h \
 ST_structs.h ST_functions.h sw_src/filefuncs.h \
 sw_src/myMemory.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/ST_mortality.o: ST_mortality.c ST_steppe.h ST_defines.h \
 sw_src/generic.h ST_structs.h ST_functions.h ST_globals.h \
 sw_src/rands.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/ST_output.o: ST_output.c ST_steppe.h ST_defines.h \
 sw_src/generic.h ST_structs.h ST_functions.h ST_globals.h \
 sw_src/filefuncs.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/ST_params.o: ST_params.c ST_steppe.h ST_defines.h \
 sw_src/generic.h ST_structs.h ST_functions.h \
 sw_src/filefuncs.h sw_src/myMemory.h sw_src/rands.h \
 ST_globals.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/ST_resgroups.o: ST_resgroups.c ST_steppe.h ST_defines.h \
 sw_src/generic.h ST_structs.h ST_functions.h ST_globals.h \
 sw_src/myMemory.h sw_src/rands.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/ST_species.o: ST_species.c ST_steppe.h ST_defines.h \
 sw_src/generic.h ST_structs.h ST_functions.h ST_globals.h \
 sw_src/myMemory.h sw_src/rands.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/ST_stats.o: ST_stats.c ST_steppe.h ST_defines.h sw_src/generic.h \
 ST_structs.h ST_functions.h sw_src/filefuncs.h \
 sw_src/myMemory.h ST_globals.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

#$(oDir)/sw_src/SW_Flow_subs.o: sw_src/SW_Flow_subs.c sw_src/generic.h sw_src/SW_Defines.h \
 #sw_src/SW_Site.h sw_src/SW_Model.h sw_src/SW_Times.h sw_src/SW_SoilWater.h
#	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/sw_src/SW_Flow_lib.o: sw_src/SW_Flow_lib.c sw_src/generic.h sw_src/SW_Defines.h \
 sw_src/SW_Flow_lib.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<

$(oDir)/sxw_tester.o: sxw_tester.c sw_src/generic.h \
 sw_src/filefuncs.h sw_src/myMemory.h ST_steppe.h \
 ST_defines.h ST_structs.h ST_functions.h ST_Globals.h sw_src/SW_Defines.h \
 sw_src/SW_Site.h sxw_funcs.h sxw.h sw_src/SW_Times.h sxw_module.h sxw_vars.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<
	
$(oDir)/sw_src/SW_VegEstab.o: sw_src/SW_VegEstab.c sw_src/generic.h sw_src/filefuncs.h sw_src/myMemory.h \
	sw_src/SW_Defines.h sw_src/SW_Files.h sw_src/SW_Site.h sw_src/SW_Times.h \
	sw_src/SW_Model.h sw_src/SW_SoilWater.h sw_src/SW_Weather.h sw_src/SW_VegEstab.h
		$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<
		
$(oDir)/ST_grid.o: ST_grid.c ST_steppe.h ST_defines.h sw_src/generic.h \
 ST_globals.h \
 sw_src/myMemory.h ST_globals.h
	$(CC) $(C_FLAGS) $(incDirs) -c -o $@ $<