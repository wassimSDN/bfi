bfi.exe: main.cpp
	clang++ main.cpp -o bfi.exe
clean: 
	rm bfi.exe