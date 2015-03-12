CC = g++
CFLAGS  = -g -Wall

AR = ar
ARFLAGS = rvs

TARGET_V1 = id3v1
TARGET_V2 = id3v2
TEST = test

# the first target is executed by default
default: $(TARGET_V1).a $(TARGET_V2).a

# ID3v1
$(TARGET_V1).a: $(TARGET_V1).o
	$(AR) $(ARFLAGS) $(TARGET_V1).a $(TARGET_V1).o
	@echo "###" \"$(TARGET_V1)\" generated

$(TARGET_V1).o: $(TARGET_V1).cpp $(TARGET_V1).h
	$(CC) $(CFLAGS) -c $(TARGET_V1).cpp

# ID3v2
$(TARGET_V2).a: $(TARGET_V2).o
	$(AR) $(ARFLAGS) $(TARGET_V2).a $(TARGET_V2).o
	@echo "###" \"$(TARGET_V2)\" generated

$(TARGET_V2).o: $(TARGET_V2).cpp $(TARGET_V2).h
	$(CC) $(CFLAGS) -c $(TARGET_V2).cpp

# Test
test: $(TEST).cpp $(TARGET_V1).a
	$(CC) $(CFLAGS) -o $(TEST) $(TEST).cpp $(TARGET_V1).a
	@echo "###" \"$(TEST)\" generated

clean: 
	$(RM) *.o *~ $(TARGET_V1).a $(TEST)
