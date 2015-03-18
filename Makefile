CC = g++
CFLAGS  = -g -Wall
CXXFLAGS  = -std=c++0x

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
	$(CC) $(CFLAGS) $(CXXFLAGS) -c $(TARGET_V2).cpp

$(UTF8).o: $(UTF8).cpp $(UTF8).h $(DEPS)
	$(CC) $(CFLAGS) -c $(UTF8).cpp

# Genre
$(GENRE).o: $(GENRE).cpp $(GENRE).h $(DEPS)
	$(CC) $(CFLAGS) -c $(GENRE).cpp

### Target: test
$(TEST): $(TEST).cpp $(TARGET_V1).a $(TARGET_V2).a
	$(CC) $(CFLAGS) $(CXXFLAGS) -o $(TEST) $(TEST).cpp $(TARGET_V1).a $(TARGET_V2).a
	@echo "###" \"$(TEST)\" generated

### Target: clean
clean: 
	$(RM) *.o *~ $(TARGET_V1).a $(TARGET_V2).a $(TEST)
