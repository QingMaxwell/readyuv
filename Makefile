


SRC=$(wildcard *.c)
EXE=read_yuv
$(EXE): $(SRC)
	gcc -o $@ $^ -lcurses
	
run: $(EXE)
	./read_yuv -s 1920x1080 -f 420sp frame_1920x1080_yuv420p.yuv

install: $(EXE)
	sudo cp $^ /usr/local/bin

.PHONY: run install
