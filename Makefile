


SRC=$(wildcard *.c)
HDR=$(wildcard *.h)
EXE=read_yuv
$(EXE): $(SRC) $(HDR)
	gcc -o $@ $(SRC) -lcurses
	
run: $(EXE)
	./read_yuv -s 1920x1080 -f 420sp frame_1920x1080_yuv420p.yuv

install: $(EXE)
	sudo cp $^ /usr/local/bin

clean:
	rm -f read_yuv

.PHONY: run install clean
