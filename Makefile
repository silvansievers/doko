TARGET = doko
HEADERS = actual_game_state.h \
	belief_game_state.h \
	card_assignment.h \
	cards.h \
	game_state.h \
	game_type.h \
	human_player.h \
	move.h \
	options.h \
	player.h \
	random_player.h \
	rng.h \
	session.h \
	trick.h \
	uct.h \
	uct_player.h
SOURCES = main.cpp $(HEADERS:%.h=%.cpp)
OBJECTS = $(SOURCES:%.cpp=.obj/%.o)
#PROFILE_OBJECTS = $(SOURCES:%.cpp=.obj/%.profile.o)

#SUFFIX_TEST = .test
#TARGET_TEST = $(TARGET)$(SUFFIX_TEST)
#SOURCES_TEST = test.cpp $(HEADERS:%.h=%.cpp)
#OBJECTS_TEST = $(SOURCES_TEST:%.cpp=.obj/%$(SUFFIX_TEST).o)

CC = g++
DEPEND = g++ -MM
CCOPT = -g -m32 -Wall -O0 -I./
LINKOPT = -g -m32
POST_LINKOPT = -Llib -lboost_program_options

release: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LINKOPT) $(OBJECTS) -o $(TARGET) $(POST_LINKOPT)

$(OBJECTS): .obj/%.o: %.cpp
	mkdir -p $$(dirname $@)
	$(CC) $(CCOPT) -c $< -o $@

#test: $(TARGET_TEST)

#$(TARGET_TEST) : $(OBJECTS_TEST)
#	$(CC) $(LINKOPT) $(OBJECTS_TEST) -o $(TARGET_TEST)

#$(OBJECTS_TEST): .obj/%$(SUFFIX_TEST).o: %.cpp
#	mkdir -p $$(dirname $@)
#	$(CC) $(CCOPT) -c $< -o $@

clean:
	rm -rf .obj
	rm -f *~
	rm -f Makefile.depend gmon.out PROFILE core

distclean: clean
	rm -f $(TARGET)

Makefile.depend: $(SOURCES) $(HEADERS)
	rm -f Makefile.temp
	for source in $(SOURCES) ; do \
	    $(DEPEND) $$source > Makefile.temp0; \
	    objfile=$${source%%.cpp}.o; \
	    sed -i -e "s@^[^:]*:@$$objfile:@" Makefile.temp0; \
	    cat Makefile.temp0 >> Makefile.temp; \
	done
	rm -f Makefile.temp0
	sed -e "s@\(.*\)\.o:\(.*\)@.obj/\1.o:\2@" Makefile.temp > Makefile.depend
	sed -e "s@\(.*\)\.o:\(.*\)@.obj/\1.profile.o:\2@" Makefile.temp >> Makefile.depend
	rm -f Makefile.temp

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
-include Makefile.depend
endif
endif

.PHONY: release clean distclean
