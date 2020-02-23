//
//  main.cpp
//  Lab3
////

#include "GlobalVariable.h"
#include "page.hpp"
#include <unistd.h>
#include <getopt.h>
#include <string>
bool verbose = false;
const int page_num = 64;
int process_num;
int frame_num = 32;
unsigned long long sum_cycle = 0;
unsigned long long ctx_switch = 0;
unsigned long long process_exit = 0;
unsigned long long operation_count;
bool O = false;
bool P = false;
bool F = false;
bool S = false;

process* process_table;
frame_t** frame_table;

bool get_next_operation(ifstream &in, char &operation, int &vpage){
    if(!in.eof()){
        char* c = new char[1000];
        in.getline(c, 1000);
        while(c[0] == '#' && !in.eof()){
            in.getline(c, 1000);
        }
        if(in.eof())
            return false;
        sscanf(c, "%c %d", &operation, &vpage);
        return true;
    }
    return false;
}

void initial_page_table(pte_t &page){
    page.index_to_page_frames = 0;
    page.modified = 0;
    page.pageout = 0;
    page.present = 0;
    page.reference = 0;
    page.write_protect = 0;
    page.file_mapped = 0;
};

void initial_process_table(ifstream &in){
    if(!in.eof()){
        char* c = new char[1000];
        in.getline(c, 1000);
        while(c[0] == '#' && !in.eof()){
            in.getline(c, 1000);
        }
        if(in.eof())
            return;
        sscanf(c, "%d", &process_num);
        process_table = new process[process_num];
        for(int i = 0; i < process_num; i++){
            process_table[i].pstats = new stats;
            process_table[i].pstats->fins = 0;
            process_table[i].pstats->fouts = 0;
            process_table[i].pstats->ins = 0;
            process_table[i].pstats->maps = 0;
            process_table[i].pstats->outs = 0;
            process_table[i].pstats->segprot = 0;
            process_table[i].pstats->segv = 0;
            process_table[i].pstats->unmaps = 0;
            process_table[i].pstats->zeros = 0;
            process_table[i].pageTable = new pte_t[page_num];
            for(int page = 0; page <page_num; page++){
                initial_page_table(process_table[i].pageTable[page]);
            }
            in.getline(c, 1000);
            while(c[0] == '#' && !in.eof()){
                in.getline(c, 1000);
            }
            int vma_num;
            sscanf(c, "%d", &vma_num);
            process_table[i].VMA = new vma[vma_num];
            process_table[i].vma_num = vma_num;
            for(int j = 0; j < vma_num; j++){
                in.getline(c, 1000);
                sscanf(c, "%d %d %d %d", &process_table[i].VMA[j].start_page, &process_table[i].VMA[j].end_page, &process_table[i].VMA[j].write_protect, &process_table[i].VMA[j].file_mapped);
            }
        }
        return;
    }
    return;
}

void initial_frame_table(int frame_num){
    frame_table = new frame_t*[frame_num];
    for(int i = 0;i < frame_num; i++){
        frame_table[i] = new frame_t;
        frame_table[i]->present = 0;
        frame_table[i]->frame_index = i;
        frame_table[i]->process = 0;
        frame_table[i]->aging_vector = 0x00000000;
    }
}

bool is_in_vma(process* current_process, int vpage){
    for(int i = 0; i < current_process->vma_num; i++){
        if(current_process->VMA[i].start_page <= vpage && vpage <= current_process->VMA[i].end_page){
            current_process->pageTable[vpage].file_mapped = current_process->VMA[i].file_mapped;
            current_process->pageTable[vpage].write_protect = current_process->VMA[i].write_protect;
            return true;
        }
    }
    return false;
}

void print_frame_table(){
    printf("FT:");
    for(int i = 0; i < frame_num; i++){
        if(frame_table[i]->present == 1)
            printf(" %d:%d ", frame_table[i]->process, frame_table[i]->vmp);
        else
            printf(" * ");
    }
    cout<<endl;
}

void simulation(ifstream &in, int frame_num, pager* page){
    initial_process_table(in);
    initial_frame_table(frame_num);
    char operation;
    int vpage;
    for(int i = 0; i < frame_num; i++){
        page->add_frame(frame_table[i]);
    }
    process* current_process = new process;
    int current_process_index = 0;
    operation_count = -1;
    bool paging = false;
    while(get_next_operation(in, operation, vpage)){
        operation_count++;
        paging = false;
        switch (operation) {
            case 'c':{
                if(O)
                    printf("%llu: ==> %c %d\n", operation_count, operation, vpage);
                current_process = &process_table[vpage];
                current_process_index = vpage;
                sum_cycle = sum_cycle + 121;
                ctx_switch++;
                break;
            };
            case 'r':{
                if(O)
                    printf("%llu: ==> %c %d\n", operation_count, operation, vpage);
                sum_cycle++;
                if(!is_in_vma(current_process, vpage)){ //also set the filed map and write protected
                    if(O)
                        printf(" SEGV\n");
                    current_process->pstats->segv++;
                    sum_cycle += 240;
                    break;
                }
                current_process->pageTable[vpage].reference = 1;
                if(current_process->pageTable[vpage].present == 0){
                    paging = true;
                }
                break;
            };
            case 'w':{
                if(O)
                    printf("%llu: ==> %c %d\n", operation_count,operation, vpage);
                sum_cycle++;
                if(!is_in_vma(current_process, vpage)){
                    if(O)
                        printf(" SEGV\n");
                    current_process->pstats->segv++;
                    sum_cycle += 240;
                    break;
                }
                current_process->pageTable[vpage].reference = 1;
                current_process->pageTable[vpage].modified = 1;
                if(current_process->pageTable[vpage].present == 0){
                    paging = true;
                }
                break;
            };
            case 'e':{
                if(O){
                    printf("%llu: ==> %c %d\n", operation_count,operation, vpage);
                    printf("EXIT current process %d\n", current_process_index);
                }
                process_exit++;
                sum_cycle += 175;
                for(int i = 0; i < page_num; i++){
                    if(current_process->pageTable[i].present == 1){
                        int index = current_process->pageTable[i].index_to_page_frames;
                        frame_t* new_frame = frame_table[index];
                        if(O)
                            printf(" UNMAP %d:%d\n", frame_table[index]->process, frame_table[index]->vmp);
                        current_process->pstats->unmaps++;
                        if(current_process->pageTable[i].file_mapped == 1){
                            if(O){
                                printf(" FOUT\n");
                            }
                            current_process->pstats->fouts++;
                            sum_cycle += 2500;
                        }
                        new_frame->present = 0;
                        new_frame->process = 0;
                        new_frame->vmp = 0;
                        page->add_frame(new_frame);
                        sum_cycle += 400;
                    }
                    current_process->pageTable[i].present = 0;
                    current_process->pageTable[i].pageout = 0;
                }
                break;
            };
            default:
                break;
        }
        if(paging){
            frame_t* new_frame_page = page->select_victim_frame();
            if(new_frame_page->present == 1){
                if(O)
                    printf(" UNMAP %d:%d\n", new_frame_page->process, new_frame_page->vmp);
                process_table[new_frame_page->process].pstats->unmaps++;
                sum_cycle += 400;
                if(process_table[new_frame_page->process].pageTable[new_frame_page->vmp].modified == 1){
                    if(process_table[new_frame_page->process].pageTable[new_frame_page->vmp].file_mapped == 1){
                        if(O)
                            printf(" FOUT\n");
                        process_table[new_frame_page->process].pstats->fouts++;
                        sum_cycle += 2500;
                    }
                    else{
                        if(O)
                            printf(" OUT\n");
                        process_table[new_frame_page->process].pageTable[new_frame_page->vmp].pageout = 1;
                        process_table[new_frame_page->process].pstats->outs++;
                        sum_cycle += 3000;
                    }
                    process_table[new_frame_page->process].pageTable[new_frame_page->vmp].modified = 0;
                }
                process_table[new_frame_page->process].pageTable[new_frame_page->vmp].present = 0;
            }
            current_process->pageTable[vpage].present = 1;
            current_process->pageTable[vpage].index_to_page_frames = new_frame_page->frame_index;
            new_frame_page->present = 1;
            new_frame_page->process = current_process_index;
            new_frame_page->vmp = vpage;
            if(current_process->pageTable[vpage].file_mapped == 1){
                if(O)
                    printf(" FIN\n");
                current_process->pstats->fins++;
                sum_cycle += 2500;
            }
            else if(current_process->pageTable[vpage].pageout == 1){
                if(O)
                    printf(" IN\n");
                current_process->pstats->ins++;
                sum_cycle += 3000;
            }
            else{
                if(O)
                    printf(" ZERO\n");
                current_process->pstats->zeros++;
                sum_cycle += 150;
            }
            if(O)
                printf(" MAP %d\n", new_frame_page->frame_index);
            new_frame_page->current_virtual_time = operation_count;
            current_process->pstats->maps++;
            sum_cycle += 400;
        }
        if(operation == 'w' && current_process->pageTable[vpage].write_protect == 1){
            current_process->pageTable[vpage].modified = 0;
            if(O)
                printf(" SEGPROT\n");
            current_process->pstats->segprot++;
            sum_cycle += 300;
        }
        if(verbose)
            print_frame_table();
    }
}

void print_page_table(pte_t* page_table){
    for(int i = 0; i < page_num; i++){
        if(page_table[i].present == 0 && page_table[i].pageout == 1)
            printf(" # ");
        else if(page_table[i].present == 0)
            printf(" * ");
        else{
            string output = to_string((long long int)(i)) + ':';
            if(page_table[i].reference == 1)
                output += 'R';
            else
                output += '-';
            if(page_table[i].modified == 1)
                output += 'M';
            else
                output += '-';
            if(page_table[i].pageout == 1)
                output += 'S';
            else
                output += '-';
            output += ' ';
            cout<<output;
        }
    }
}

void print_process_table(){
    for(int i = 0; i < process_num; i++){
        printf("PT[%d]: ", i);
        process* current_process = &process_table[i];
        print_page_table(current_process->pageTable);
        cout<<endl;
    }
}

void print_stats(){
    for(int i = 0; i < process_num; i++){
        printf("PROC[%d]: U=%lu M=%lu I=%lu O=%lu FI=%lu FO=%lu Z=%lu SV=%lu SP=%lu\n",
               i,
               process_table[i].pstats->unmaps, process_table[i].pstats->maps, process_table[i].pstats->ins, process_table[i].pstats->outs,
               process_table[i].pstats->fins, process_table[i].pstats->fouts, process_table[i].pstats->zeros,
               process_table[i].pstats->segv, process_table[i].pstats->segprot);
    }
}

bool check_parameter(char* optarg, char parameter){
    int i = 0;
    while(optarg[i] != '\0'){
        if(optarg[i] == parameter)
            return true;
        i++;
    }
    return false;
}


int main(int argc,  char* const* argv) {
    // insert code here...
    ifstream in(argv[argc-2]);
    ifstream random(argv[argc-1]);
    int c;
    pager* pager;
    while((c = getopt(argc, argv, "a:o:f:"))!= -1){
        switch (c) {
            case 'f':{
                frame_num = atoi(&optarg[0]);
                break;
            }
            case 'a':{
                switch (optarg[0]) {
                    case 'f':{
                        FIFO_pager* this_pager = new FIFO_pager;
                        pager = this_pager;
                        break;
                    }
                    case 'w':{
                        Working_set_pager* this_pager = new Working_set_pager;
                        pager = this_pager;
                        break;
                    }
                    case 'c':{
                        Clock_pager* this_pager = new Clock_pager;
                        pager = this_pager;
                        break;
                    }
                    case 'r':{
                        Random_pager* this_pager = new Random_pager;
                        pager = this_pager;
                        break;
                    }
                    case 'e':{
                        NRU_pager* this_pager = new NRU_pager;
                        pager = this_pager;
                        break;
                    }
                    case 'a':{
                        Aging_pager* this_pager = new Aging_pager;
                        pager = this_pager;
                        break;
                    }
                    default:
                        break;
                }
            }
            case 'o':{
                char parameter = 'O';
                if(check_parameter(optarg, parameter)){
                    O = true;
                }
                parameter = 'F';
                if(check_parameter(optarg, 'F')){
                    F = true;
                }
                parameter = 'P';
                if(check_parameter(optarg, 'P')){
                    P = true;
                }
                parameter = 'S';
                if(check_parameter(optarg, 'S')){
                    S = true;
                }
            }
            default:
                break;
        }
    }
    frame_table = new frame_t*[frame_num];
    readRandval(random);
    simulation(in, frame_num, pager);
    if(P)
        print_process_table();
    if(F)
        print_frame_table();
    if(S){
        print_stats();
        printf("TOTALCOST %llu %llu %llu %llu", operation_count + 1, ctx_switch, process_exit, sum_cycle);
    }
    return 0;
}
