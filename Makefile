
#the number of make jobs to run simultaneously
JOBS = 1

CC      = g++


C_DBG_FLAGS = -g3
C_OPT_FLAGS = -O3


SHARED_STR_LIB_DIR = /home/darkstone/darkston/lib
SHARED_STR_INCLUDE = /home/darkstone/darkston/include

LUA_LIB_DIR = /usr/local/lib
LUA_INCLUDE = /usr/local/include
LUA_LIB = lua

# Use Makefile.local for the directories and so forth
# that might change per developer
-include Makefile.local

INCLUDES = -I$(SHARED_STR_INCLUDE) -I$(LUA_INCLUDE) -I/home/darkstone/darkston/include


# Set C_FLAGS to one of DBG or OPT, depending on build type
C_FLAGS = -Wall --pedantic -Wno-long-long -DSHARED_STR_NO_STRICMP  \
		  $(C_DBG_FLAGS) $(INCLUDES)

LIB_DIRS = -L./pargen_include -L$(SHARED_STR_LIB_DIR) -L$(LUA_LIB_DIR) -L/home/darkstone/darkston/lib

L_FLAGS = -lcrypt -lm -rdynamic -ldl -lPargen -lsharedstr -l$(LUA_LIB) $(LIB_DIRS)
		  

#	-L/home/darkstone/BerkeleyDBXML.1.1/lib -L/home/darkstone/BerkeleyDB.4.1/lib \
#	-L/home/darkstone/pathan/lib -L/home/darkstone/xerces/lib \
#	-lpathan -lxerces-c -ldbxml-1.1 -ldb_cxx-4.1

PARGEN_FLAGS = -DYYSTDCPPLIB -I pargen_include -I pargen_cpp

#DATABASE_INCLUDES = -I /home/darkstone/BerkeleyDBXML.1.1/include -I /home/darkstone/xerces/include -I /home/darkstone/BerkeleyDB.4.1/include
DATABASE_INCLUDES = 


# Took out modules.o, ibuild.o

PARGEN_O_FILES = \
	pargen_cpp/yyacback.o pargen_cpp/yyaccpya.o pargen_cpp/yyacdci.o \
	pargen_cpp/yyacdcic.o pargen_cpp/yyacdela.o pargen_cpp/yyacdisc.o \
	pargen_cpp/yyacdop.o pargen_cpp/yyacdpop.o pargen_cpp/yyacecho.o \
	pargen_cpp/yyacgetc.o pargen_cpp/yyacgtok.o pargen_cpp/yyacinp.o \
	pargen_cpp/yyaclcln.o pargen_cpp/yyaclcon.o pargen_cpp/yyaclcre.o \
	pargen_cpp/yyacldbg.o pargen_cpp/yyacldes.o pargen_cpp/yyaclerr.o \
	pargen_cpp/yyacless.o pargen_cpp/yyaclex.o pargen_cpp/yyaclexc.o \
	pargen_cpp/yyaclvts.o pargen_cpp/yyaclvtv.o pargen_cpp/yyacnewa.o \
	pargen_cpp/yyacoutp.o pargen_cpp/yyacpar.o pargen_cpp/yyacpcln.o \
	pargen_cpp/yyacpcon.o pargen_cpp/yyacpcre.o pargen_cpp/yyacpdbg.o \
	pargen_cpp/yyacpdes.o pargen_cpp/yyacperr.o pargen_cpp/yyacpop.o \
	pargen_cpp/yyacpp.o pargen_cpp/yyacpush.o pargen_cpp/yyacres.o \
	pargen_cpp/yyacserr.o pargen_cpp/yyacsin.o pargen_cpp/yyacsofw.o \
	pargen_cpp/yyacsskp.o pargen_cpp/yyacsssz.o pargen_cpp/yyacstsz.o \
	pargen_cpp/yyacstv.o pargen_cpp/yyacsup.o pargen_cpp/yyacsusz.o \
	pargen_cpp/yyactofw.o pargen_cpp/yyacucin.o pargen_cpp/yyacunp.o \
	pargen_cpp/yyacuofw.o pargen_cpp/yyacvtlv.o pargen_cpp/yyacvts.o \
	pargen_cpp/yyacwip.o pargen_cpp/yyacwrap.o pargen_cpp/yyacwrk.o pargen_cpp/yyacwrkc.o

SKRYPT_O_FILES = \
	skrypt/skrypt.o skrypt/skrypt_builtins.o skrypt/mylexer.o skrypt/myparser.o \
	skrypt/skrypt_events.o

DB_O_FILES = \

#	dataBase/db_public.o dataBase/db_env.o dataBase/db_util.o dataBase/vault_db.o

# Took out id.o

CORE_O_FILES = \
	accounts.o act_comm.o act_info.o act_move.o act_obj.o act_wiz.o \
	area.o arena.o bank.o boards.o build.o clans.o character.o color.o comm.o \
	comments.o connection.o connection_manager.o const.o copyover.o db.o \
	deity.o fight.o grub.o handler.o hashstr.o interp.o last.o lua_util.o magic.o \
	makeobjs.o mapout.o memory.o misc.o mpxset.o mud_comm.o mud_prog.o \
	mxp.o new_dump.o object.o pager.o player.o quests.o requests.o reset.o room.o save.o \
	Scent.o ScentController.o shared_str.o shops.o skills.o socials.o \
	socket_connection.o socket_general.o socket_listener.o special.o stable.o \
	stored_objs.o tables.o track.o translate.o update.o utility.o World.o 

O_FILES = $(CORE_O_FILES:%.o=.o/%.o) $(SKRYPT_O_FILES) $(DB_O_FILES)

all:
	@mkdir -p .o .d
	@$(MAKE) -j$(JOBS) dark
	@$(MAKE) tags

.PHONY: tags

tags:
	@echo Making tag file...
	@ctags --c++-kinds=+cdefgmnpstuv --fields=+iaS --extra=+q *.cpp *.h *.hpp


grux:	grux.o
	rm -f grux
	$(CC) $(L_FLAGS) -o grux grux.o
	chmod g+w grux
	chmod g+w grux.o

dark: $(O_FILES) pargenlib
	rm -f dark
	$(CC) $(O_FILES) -o dark $(L_FLAGS)
	@chmod g+w dark
	@chmod g+w $(O_FILES)

prof: $(O_FILES)
	rm -f dark.prof
	$(CC) -pg -static $(O_FILES) -o dark.prof $(L_FLAGS)
	chmod g+w dark.prof
	chmod g+w $(O_FILES)

test: $(O_FILES) pargenlib
	rm -f dark.test
	$(CC) $(O_FILES) -o dark.test $(L_FLAGS)
	chmod g+w dark.test
	chmod g+w $(O_FILES)

pargen_cpp/%.o: pargen_cpp/%.cpp
	$(CC) -c $(C_OPT_FLAGS) $(PARGEN_FLAGS) -o $@ $<
	strip --strip-unneeded $@

skrypt/%.o: skrypt/%.cpp
	$(CC) -I skrypt -c $(C_FLAGS) $(PARGEN_FLAGS) -o $@ $<

dataBase/%.o: dataBase/%.cpp
	$(CC) $(DATABASE_INCLUDES) -I dataBase -c $(C_FLAGS) -o $@ $<


DEPDIR = .d
df = $(DEPDIR)/$(*F)

MAKEDEPEND = gcc -M $(C_FLAGS) -o $(df).d $<


.o/%.o: %.cpp
	@echo Compiling $<...
	$(CC) -MD -c $(C_FLAGS) -o $@ $<
	@cp .o/$*.d $(df).P; \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < .o/$*.d >> $(df).P; \
	  rm -f .o/$*.d

#	$(CC) -pg -c $(C_FLAGS) $<

-include $(CORE_O_FILES:%.o=$(DEPDIR)/%.P)


clean:
	rm -f $(O_FILES)

pargenlib: pargen_include/libPargen.a

pargen_include/libPargen.a: $(PARGEN_O_FILES)
	rm -f pargen_include/libPargen.a
	ar -r pargen_include/libPargen.a $(PARGEN_O_FILES)
	ranlib pargen_include/libPargen.a
	strip --strip-unneeded $@
