#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include "thread.h"
#include "disk.h"

using std::cout;
using std::endl;

int current_track = 0;
int max_disk_queue = 0;
int current_disk_queue = 0;
int num_disks = 0;
int active_requester = 0;

std::vector<std::pair<int,int>> disk_queue;
std::vector<bool> if_requested;
std::vector<bool> if_finished;
std::ifstream ifs[1000];
mutex queue;
cv cv_queue;
cv cv_request;

int my_abs(int a) {
    return (a>0)? a:(-a);
}

void servicer(void *a){
    queue.lock();
    while ((active_requester > 0) || (current_disk_queue > 0)){
        while ((current_disk_queue < max_disk_queue) & (active_requester > 0)){
            cv_request.broadcast();
            cv_queue.wait(queue);
        }
        int pos = 0, min = 10000;
        for (int i = 0; i < current_disk_queue; i++){
            if (my_abs(disk_queue[i].first - current_track) < min){
                pos = i;
                min = my_abs(disk_queue[i].first - current_track);
            }
        }
        print_service(disk_queue[pos].second, disk_queue[pos].first);
        current_track = disk_queue[pos].first;
        if (!if_finished[disk_queue[pos].second]){
            active_requester++;
            if_requested[disk_queue[pos].second] = false;
        }
        disk_queue.erase(disk_queue.begin()+pos);
        current_disk_queue--;
    }
    queue.unlock();
    return;
}

void requester(void *a){
    uintptr_t num = (uintptr_t) a;
    std::string buf;
    getline(ifs[num], buf);
    while (true){
        queue.lock();
        while (if_requested[num]||(current_disk_queue == max_disk_queue))
            cv_request.wait(queue);
        int track = stoi(buf);
        disk_queue.push_back(std::make_pair(track,num));
        current_disk_queue++;
        if_requested[num] = true;
        active_requester--;
        print_request(num, track);
        if (getline(ifs[num], buf)) {
            cv_queue.signal();
            queue.unlock();
        }
        else {
            if_finished[num] = true;
            ifs[num].close();
            cv_queue.signal();
            queue.unlock();
            return;
        }       
    }
}


void main_thread(void *a){
    thread service ((thread_startfunc_t) servicer, (void *) "This is servicer");
    for (int i = 0; i < num_disks; i ++){
        uintptr_t num = i;
        thread request  ((thread_startfunc_t) requester, (void *) num);
    }
}

int main(int argc, char *argv[]){
    max_disk_queue = atoi(argv[1]);
    num_disks = argc - 2;
    active_requester = num_disks;
    for(int i=0; i<num_disks; i++) {
        if_requested.push_back(false);
        if_finished.push_back(false);
        ifs[i].open(argv[i+2], std::ios::in);
	}
    cpu::boot((thread_startfunc_t) main_thread, (void *) 0, 0);    
}
