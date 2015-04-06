CC = g++
CFLAGS = -g -Wall
#CXXFLAGS = -std=c++0x

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

DEPS = common.h

# Targets
TAG = tag
TAG_V1 = id3v1
TAG_V2 = id3v2
UTF8 = utf8
GENRE = genre
FRAME = frame

TEST = test

### Target: default (the first to be executed)
default: $(TAG).a

$(TAG).a: $(TAG_V1).o $(TAG_V2).o $(UTF8).o $(GENRE).o $(FRAME).o
	$(AR) $(ARFLAGS) $(TAG).a $(TAG_V1).o $(TAG_V2).o $(UTF8).o $(GENRE).o $(FRAME).o
	@echo "###" \"$(TAG)\" generated

# ID3v1
$(TAG_V1).o: $(TAG_V1).cpp $(TAG_V1).h $(DEPS)
	$(CC) $(CFLAGS) -c $(TAG_V1).cpp

# ID3v2 (-liconv)
$(TAG_V2).o: $(TAG_V2).cpp $(TAG_V2).h $(DEPS)
	$(CC) $(CFLAGS) -c $(TAG_V2).cpp

$(UTF8).o: $(UTF8).cpp $(UTF8).h $(DEPS)
	$(CC) $(CFLAGS) -c $(UTF8).cpp

$(FRAME).o: $(FRAME).cpp $(FRAME).h $(DEPS)
	$(CC) $(CFLAGS) -c $(FRAME).cpp

# Genre
$(GENRE).o: $(GENRE).cpp $(GENRE).h $(DEPS)
	$(CC) $(CFLAGS) -c $(GENRE).cpp

### Target: test
$(TEST): $(TEST).cpp $(TAG).a
	$(CC) $(CFLAGS) $(LIBS) -o $(TEST) $(TEST).cpp $(TAG).a
	@echo "###" \"$(TEST)\" generated

### Target: clean
clean: 
	$(RM) *.o *~ $(TAG).a $(TEST)
