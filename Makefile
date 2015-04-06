all:
	make -C video
	make -C input
	make -C memory
	make -C vm
	make -C storage
	make -C dasm
	make -C init
	make -C docs

cc65:
	make -C cc65

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

