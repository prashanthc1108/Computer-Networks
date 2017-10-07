CC = gcc
CFLAGS = -g -Wall -I.

TARGET = pc74_proj1


sc: pc74_proj1.c genFunctions.c registryList.c
	clear
	$(CC) $(CFLAGS) -o $(TARGET) pc74_proj1.c genFunctions.c registryList.c

clean:
	$(RM) $(TARGET)
