
linux:
	g++ -g -O3 -D_NANO_LINUX_ -o HMM HMM/HMM.cpp
mingw:
	g++ -g -O3 -D_NANO_MINGW_ -o HMM HMM/HMM.cpp
run:
	./HMM.exe
	
clean:
	rm ./HMM.exe