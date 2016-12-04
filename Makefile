CC = g++
CFLAGS = -std=c++11 -Wall -Wextra -Werror
CFLAGS += -g3

#ifeq ($(OS),Windows_NT)
#	CCFLAGS += -D WIN32
#	ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
#		CCFLAGS += -D AMD64
#	endif
#	ifeq ($(PROCESSOR_ARCHITECTURE),x86)
#		CCFLAGS += -D IA32
#	endif
#else
UNAME_S := $(shell uname -s)
#ifeq ($(UNAME_S), Linux)
#	CCFLAGS += -D LINUX
#endif
ifeq ($(UNAME_S), Darwin)
#	CCFLAGS += -D OSX
	LIBS = -liconv
endif

#UNAME_P := $(shell uname -p)
#ifeq ($(UNAME_P),x86_64)
#	CCFLAGS += -D AMD64
#endif
#ifneq ($(filter %86,$(UNAME_P)),)
#	CCFLAGS += -D IA32
#endif
#ifneq ($(filter arm%,$(UNAME_P)),)
#	CCFLAGS += -D ARM
#endif
#endif

AR = ar
ARFLAGS = rvs

# Targets
TAG = tag
TAG_V1 = id3v1
TAG_V2 = id3v2
TAG_APE = ape
TAG_LYRICS = lyrics
UTF8 = utf8
GENRE = genre
FRAME = frame

TEST = test

# Common dependencies
DEPS = common.h $(TAG).h

### Target: default (the first to be executed)
default: $(TAG).a

$(TAG).a: $(TAG_V1).o $(TAG_V2).o $(FRAME).o $(TAG_APE).o $(TAG_LYRICS).o $(UTF8).o $(GENRE).o
	@echo "#" generate \"$(TAG)\" library
	$(AR) $(ARFLAGS) $(TAG).a $(TAG_V1).o $(TAG_V2).o $(FRAME).o $(TAG_APE).o $(TAG_LYRICS).o $(UTF8).o $(GENRE).o

# ID3v1
$(TAG_V1).o: $(TAG_V1).cpp $(TAG_V1).h $(DEPS) $(GENRE).h
	@echo "#" generate \"$(TAG_V1)\"
	$(CC) $(CFLAGS) -c $(TAG_V1).cpp

# ID3v2 (-liconv)
$(TAG_V2).o: $(TAG_V2).cpp $(TAG_V2).h $(DEPS) $(FRAME).h $(GENRE).h $(UTF8).h
	@echo "#" generate \"$(TAG_V2)\"
	$(CC) $(CFLAGS) -c $(TAG_V2).cpp

$(FRAME).o: $(FRAME).cpp $(FRAME).h $(DEPS)
	$(CC) $(CFLAGS) -c $(FRAME).cpp

# APE
$(TAG_APE).o: $(TAG_APE).cpp $(DEPS)
	@echo "#" generate \"$(TAG_APE)\"
	$(CC) $(CFLAGS) -c $(TAG_APE).cpp

# Lyrics
$(TAG_LYRICS).o: $(TAG_LYRICS).cpp $(DEPS)
	@echo "#" generate \"$(TAG_LYRICS)\"
	$(CC) $(CFLAGS) -c $(TAG_LYRICS).cpp

# Aux
$(GENRE).o: $(GENRE).cpp $(GENRE).h
	$(CC) $(CFLAGS) -c $(GENRE).cpp

$(UTF8).o: $(UTF8).cpp $(UTF8).h $(DEPS)
	$(CC) $(CFLAGS) -c $(UTF8).cpp

### Target: test
$(TEST): $(TEST).cpp $(TAG).h $(TAG).a
	$(CC) $(CFLAGS) $(LIBS) -o $(TEST) $(TEST).cpp $(TAG).a
	@echo "###" \"$(TEST)\" generated

### Target: clean
clean: 
	$(RM) *.o *~ $(TAG).a $(TEST)
	$(RM) -r $(TEST).dSYM
