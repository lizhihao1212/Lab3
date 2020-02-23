mmu: main.cpp GlobalVariable.cpp GlobalVariable.h page.cpp page.hpp 
	g++ -std=c++0x main.cpp GlobalVariable.cpp GlobalVariable.h page.cpp page.hpp -o mmu

clean:
	rm -f page*~ GlobalVariable*~ main*~ makefile*~
