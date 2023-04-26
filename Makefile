CC = cl
CFLAGS = /EHsc /W4 /MT
TARGET = hen_splitter

all: $(TARGET).exe

$(TARGET).exe: main.obj
    $(CC) $(CFLAGS) main.obj /Fe:$(TARGET).exe

main.obj: main.cpp
    $(CC) $(CFLAGS) /c main.cpp /Fe:main.obj

clean:
    del *.obj *.exe

.PHONY: all clean
