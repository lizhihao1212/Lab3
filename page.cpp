//
//  page.cpp
//  Lab3
//
//

#include "page.hpp"

void frame_list::add_free_frame_to_list(frame_t *new_frame){
    if(new_frame->present != 0){
        printf("Not a free frame!\n");
        exit(0);
    }
    if(head == NULL){
        head = new frame_list_node;
        head->this_frame = new_frame;
        head->next = NULL;
        return;
    }
    if(head->this_frame->present != 0){
        frame_list_node* new_node = new frame_list_node;
        new_node->this_frame = new_frame;
        new_node->next = head;
        head = new_node;
        return;
    }
    frame_list_node* temp = head;
    while(temp->next != NULL){
        if(temp->next != NULL){
            if(temp->next->this_frame->present != 0)
                break;
        }
        temp = temp->next;
    }
    frame_list_node* new_node = new frame_list_node;
    new_node->this_frame = new_frame;
    new_node->next = temp->next;
    temp->next = new_node;
    return;
}

void frame_list::add_frame_to_list(frame_t *new_frame){
    if(head == NULL){
        head = new frame_list_node;
        head->this_frame = new_frame;
        head->next = NULL;
        return;
    }
    frame_list_node* temp = head;
    while(temp->next != NULL){
        temp = temp->next;
    }
    frame_list_node* new_node = new frame_list_node;
    new_node->this_frame = new_frame;
    new_node->next = NULL;
    temp->next = new_node;
}

frame_t* frame_list::get_head_frame(){
    frame_t* head_frame = head->this_frame;
    head = head->next;
    return head_frame;
}

frame_t* frame_list::remove_frame(int frame_index){
    if(head == NULL){
        printf("Error in list, head is null\n");
        exit(0);
    }
    frame_list_node* temp = head;
    while(temp->next->this_frame->frame_index != frame_index){
        temp = temp->next;
    }
    frame_list_node* delete_node = temp->next;
    temp->next = temp->next->next;
    frame_t* result = delete_node->this_frame;
    return result;
}

void frame_list::print_list(){
    frame_list_node* temp = head;
    while(temp != NULL){
        printf(" %d:%d:%d:%d", temp->this_frame->process, temp->this_frame->vmp, temp->this_frame->present, temp->this_frame->frame_index);
        temp = temp->next;
    }
    printf("\n");
}

frame_t* FIFO_pager::select_victim_frame(){
    if(!free_frame.is_empty()){
        frame_t* victim_frame = free_frame.get_head_frame();
        return victim_frame;
    }
    frame_index++;
    if(frame_index == frame_num)
        frame_index = 0;
    return frame_table[frame_index];
}

void FIFO_pager::add_frame(frame_t *new_frame){
    free_frame.add_frame_to_list(new_frame);
    return;
}

void Random_pager::add_frame(frame_t *new_frame){
    free_frame.add_frame_to_list(new_frame);
    return;
}

frame_t* Random_pager::select_victim_frame(){
    if(!free_frame.is_empty()){
        frame_t* victim_frame = free_frame.get_head_frame();
        return victim_frame;
    }
    ofs++;
    int frame_index = myrandom(ofs);
    return frame_table[frame_index];
}

void Clock_pager::add_frame(frame_t *new_frame){
    free_frame.add_frame_to_list(new_frame);
    return;
}

frame_t* Clock_pager::select_victim_frame(){
    if(!free_frame.is_empty()){
        frame_t* victim_frame = free_frame.get_head_frame();
        return victim_frame;
    }
    clock_pointer++;
    if(clock_pointer == frame_num)
        clock_pointer = 0;
    while(process_table[frame_table[clock_pointer]->process].pageTable[frame_table[clock_pointer]->vmp].reference== 1){
        process_table[frame_table[clock_pointer]->process].pageTable[frame_table[clock_pointer]->vmp].reference = 0;
        clock_pointer++;
        if(clock_pointer == frame_num)
            clock_pointer = 0;
    }
    return frame_table[clock_pointer];
}

void Working_set_pager::add_frame(frame_t *new_frame){
    free_frame.add_frame_to_list(new_frame);
    return;
}

frame_t* Working_set_pager::select_victim_frame(){
    if(!free_frame.is_empty()){
        frame_t* victim_frame = free_frame.get_head_frame();
        return victim_frame;
    }
    clock_pointer++;
    if(clock_pointer == frame_num)
        clock_pointer = 0;
    int current_pointer = clock_pointer;
    int loop_counter = 0;
    for(;loop_counter < frame_num; loop_counter++){
        if(process_table[frame_table[current_pointer]->process].pageTable[frame_table[current_pointer]->vmp].reference== 0){
            if(operation_count - frame_table[current_pointer]->current_virtual_time >= tau){
                clock_pointer = current_pointer;
                return frame_table[current_pointer];
            }
        }
        if(process_table[frame_table[current_pointer]->process].pageTable[frame_table[current_pointer]->vmp].reference== 1){
            process_table[frame_table[current_pointer]->process].pageTable[frame_table[current_pointer]->vmp].reference= 0;
            frame_table[current_pointer]->current_virtual_time = operation_count;
        }
        current_pointer++;
        if(current_pointer == frame_num)
            current_pointer = 0;
    }
    current_pointer = clock_pointer;
    loop_counter = 0;
    unsigned long long current_virtual_time = frame_table[clock_pointer]->current_virtual_time;
    for(;loop_counter < frame_num;loop_counter++){
        if(frame_table[current_pointer]->current_virtual_time < current_virtual_time ){
            clock_pointer = current_pointer;
            current_virtual_time = frame_table[current_pointer]->current_virtual_time;
        }
        current_pointer++;
        if(current_pointer == frame_num)
            current_pointer = 0;
    }
    frame_table[clock_pointer]->current_virtual_time = operation_count;
    return frame_table[clock_pointer];
}

void NRU_pager::add_frame(frame_t *new_frame){
    free_frame.add_frame_to_list(new_frame);
    return;
}

frame_t* NRU_pager::select_victim_frame(){
    if(!free_frame.is_empty()){
        frame_t* victim_frame = free_frame.get_head_frame();
        return victim_frame;
    }
    clock_pointer++;
    if(clock_pointer == frame_num)
        clock_pointer = 0;
    int* frame_class = new int[3];
    frame_class[0] = -1;
    frame_class[1] = -1;
    frame_class[2] = -1;
    frame_class[3] = -1;
    if(operation_count - last_reset_time + 1 >= tau){
        last_reset_time = operation_count + 1;
        int current_pointer = clock_pointer;
        for(int i = 0; i < frame_num; i++){
            if(process_table[frame_table[current_pointer]->process].pageTable[frame_table[current_pointer]->vmp].reference == 0){
                if(process_table[frame_table[current_pointer]->process].pageTable[frame_table[current_pointer]->vmp].modified == 0){
                    if(frame_class[0] == -1)
                        frame_class[0] = current_pointer;
                }
                else{
                    if(frame_class[1] == -1)
                        frame_class[1] = current_pointer;
                }
            }
            else{
                process_table[frame_table[current_pointer]->process].pageTable[frame_table[current_pointer]->vmp].reference = 0;
                if(process_table[frame_table[current_pointer]->process].pageTable[frame_table[current_pointer]->vmp].modified == 0){
                    if(frame_class[2] == -1)
                        frame_class[2] = current_pointer;
                }
                else{
                    if(frame_class[3] == -1)
                        frame_class[3] = current_pointer;
                }
            }
            current_pointer++;
            if(current_pointer == frame_num){
                current_pointer = 0;
            }
        }
    }
    else{
        int current_pointer = clock_pointer;
        for(int i = 0; i < frame_num; i++){
            if(process_table[frame_table[current_pointer]->process].pageTable[frame_table[current_pointer]->vmp].reference == 0){
                if(process_table[frame_table[current_pointer]->process].pageTable[frame_table[current_pointer]->vmp].modified == 0){
                    clock_pointer = current_pointer;
                    return frame_table[current_pointer];
                }
                else{
                    if(frame_class[1] == -1)
                        frame_class[1] = current_pointer;
                }
            }
            else{
                if(process_table[frame_table[current_pointer]->process].pageTable[frame_table[current_pointer]->vmp].modified == 0){
                    if(frame_class[2] == -1)
                        frame_class[2] = current_pointer;
                }
                else{
                    if(frame_class[3] == -1)
                        frame_class[3] = current_pointer;
                }
            }
            current_pointer++;
            if(current_pointer == frame_num){
                current_pointer = 0;
            }
        }
    }
    if(frame_class[0] != -1){
        clock_pointer = frame_class[0];
    }
    else if(frame_class[1] != -1){
        clock_pointer = frame_class[1];
    }
    else if(frame_class[2] != -1){
        clock_pointer = frame_class[2];
    }
    else{
        clock_pointer = frame_class[3];
    }
    delete[] frame_class;
    return frame_table[clock_pointer];
}

void Aging_pager::add_frame(frame_t *new_frame){
    free_frame.add_frame_to_list(new_frame);
    return;
}

frame_t* Aging_pager::select_victim_frame(){
    if(!free_frame.is_empty()){
        frame_t* victim_frame = free_frame.get_head_frame();
        victim_frame->aging_vector = 0x00000000;
        return victim_frame;
    }
    clock_pointer++;
    if(clock_pointer == frame_num)
        clock_pointer = 0;
    int current_pointer = clock_pointer;
    for(int i = 0; i < frame_num; i++){
        frame_table[current_pointer]->aging_vector = frame_table[current_pointer]->aging_vector>>1;
        frame_table[current_pointer]->aging_vector = process_table[frame_table[current_pointer]->process].pageTable[frame_table[current_pointer]->vmp].reference<<31 ^ frame_table[current_pointer]->aging_vector;
        process_table[frame_table[current_pointer]->process].pageTable[frame_table[current_pointer]->vmp].reference = 0;
        current_pointer++;
        if(current_pointer == frame_num){
            current_pointer = 0;
        }
    }
    current_pointer = clock_pointer;
    for(int i = 0; i < frame_num; i++){
        if(frame_table[current_pointer]->aging_vector == 0){
            clock_pointer = current_pointer;
            frame_table[clock_pointer]->aging_vector = 0x00000000;
            return frame_table[clock_pointer];
        }
        if(frame_table[current_pointer]->aging_vector < frame_table[clock_pointer]->aging_vector){
            clock_pointer = current_pointer;
        }
        current_pointer++;
        if(current_pointer == frame_num){
            current_pointer = 0;
        }
    }
    frame_table[clock_pointer]->aging_vector = 0x00000000;
    return frame_table[clock_pointer];
}
