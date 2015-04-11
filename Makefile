all:
	make -C video
	make -C input
	make -C memory
	make -C vm
	make -C storage
	make -C dasm
	make -C init

doc:
	make -C docs

# requires cc65 in the path
cc65-support:
	make -C cc65

bin:
	make all
	mkdir dan64_v1-bin
	cp LICENSE init/main.hex init/main.bin init/main.srec dan64_v1-bin
	zip -r dan64_v1-bin.zip dan64_v1-bin
	rm -rf dan64_v1-bin
	mv dan64_v1-bin.zip docs

upload: all
	make -C init upload

clean:
	make -s -C video clean
	make -s -C input clean
	make -s -C memory clean
	make -s -C vm clean
	make -s -C storage clean
	make -s -C storage/tools clean
	make -s -C dasm clean
	make -s -C dasm/tools clean
	make -s -C init clean
	make -s -C docs clean
	make -s -C cc65 clean

