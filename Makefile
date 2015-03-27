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
TARGET_V1 = id3v1
TARGET_V2 = id3v2
UTF8 = utf8
GENRE = genre

TEST = test

### Target: default (the first to be executed)
default: $(TARGET_V1).a $(TARGET_V2).a

# ID3v1
$(TARGET_V1).a: $(TARGET_V1).o $(GENRE).o
	$(AR) $(ARFLAGS) $(TARGET_V1).a $(TARGET_V1).o $(GENRE).o
	@echo "###" \"$(TARGET_V1)\" generated

$(TARGET_V1).o: $(TARGET_V1).cpp $(TARGET_V1).h $(DEPS)
	$(CC) $(CFLAGS) -c $(TARGET_V1).cpp

# ID3v2 (-liconv)
$(TARGET_V2).a: $(TARGET_V2).o $(UTF8).o $(GENRE).o
	$(AR) $(ARFLAGS) $(TARGET_V2).a $(TARGET_V2).o $(UTF8).o $(GENRE).o
	@echo "###" \"$(TARGET_V2)\" generated

$(TARGET_V2).o: $(TARGET_V2).cpp $(TARGET_V2).h $(DEPS)
	$(CC) $(CFLAGS) -c $(TARGET_V2).cpp

$(UTF8).o: $(UTF8).cpp $(UTF8).h $(DEPS)
	$(CC) $(CFLAGS) -c $(UTF8).cpp

# Genre
$(GENRE).o: $(GENRE).cpp $(GENRE).h $(DEPS)
	$(CC) $(CFLAGS) -c $(GENRE).cpp

### Target: test
$(TEST): $(TEST).cpp $(TARGET_V1).a $(TARGET_V2).a
	$(CC) $(CFLAGS) $(LIBS) -o $(TEST) $(TEST).cpp $(TARGET_V1).a $(TARGET_V2).a
	@echo "###" \"$(TEST)\" generated

### Target: clean
clean: 
	$(RM) *.o *~ $(TARGET_V1).a $(TARGET_V2).a $(TEST)
