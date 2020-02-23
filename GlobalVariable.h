//
//  GlobalVariable.h
//  Lab3
//
//

#ifndef GlobalVariable_h
#define GlobalVariable_h

#include <iostream>
#include <fstream>
using namespace std;
typedef struct {
    unsigned present:1;
    unsigned write_protect:1;
    unsigned modified:1;
    unsigned reference:1;
    unsigned pageout:1;
    unsigned file_mapped:1;
    unsigned index_to_page_frames:7;
} pte_t;

typedef struct {
    int frame_index;
    int vmp;
    int process;
    unsigned present:1;
    unsigned long long current_virtual_time;
    unsigned aging_vector:32;
}frame_t;

struct vma {
    int start_page;
    int end_page;
    int write_protect;
    int file_mapped;
};

extern const int page_num;
extern frame_t** frame_table;
extern int frame_num;
extern int* randval;
extern int myrandom(int ofs);
extern void readRandval(ifstream &randomNum);
extern unsigned long long operation_count;

struct stats{
    unsigned long unmaps;
    unsigned long maps;
    unsigned long ins;
    unsigned long outs;
    unsigned long fins;
    unsigned long fouts;
    unsigned long zeros;
    unsigned long segv;
    unsigned long segprot;
};

struct process {
    pte_t* pageTable;
    vma* VMA;
    int vma_num;
    stats* pstats;
};


extern process* process_table;







#endif /* GlobalVariable_h */
