//
//  GlobalVariable.cpp
//  Lab3
#include "GlobalVariable.h"
int* randval;
int length;
int myrandom(int ofs) { return (randval[ofs % length] % frame_num); }

void readRandval(ifstream &randomNum){
    char* c = new char[1000];
    randomNum.getline(c, 1000);
    length = stoi(string(c));
    randval = new int[length];
    int i = 0;
    while(!randomNum.eof()){
        randomNum.getline(c, 1000);
        if(randomNum.eof())
            break;
        randval[i] = stoi(string(c));
        i++;
    }
}
