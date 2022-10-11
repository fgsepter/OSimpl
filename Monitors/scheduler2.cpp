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
int num_finished = 0;
int active_requester = 0;

std::vector<std::pair<int,int>> disk_queue;
std::vector<std::string> file_names;
std::vector<bool> if_requested;
std::vector<bool> if_finished;
mutex queue;
cv cv_queue;
cv ask_for_request;
mutex finish_lock;

int my_abs(int a) {
    return (a>0)? a:(-a);
}

void servicer(void *a)
{
    queue.lock();
    while ((num_finished < num_disks) | (current_disk_queue > 0)){
        queue.unlock();
        queue.lock();
        if ((current_disk_queue < max_disk_queue) & (num_finished < num_disks) & (active_requester > 0)){
            //ask_for_request.wait(queue);
            cv_queue.signal();
        }
        else {
            int pos = 0;
            int min = 10000;
            for (int i = 0; i < current_disk_queue; i++){
                if (my_abs(disk_queue[i].first - current_track) < min){
                    pos = i;
                    min = my_abs(disk_queue[i].first - current_track);
                }
            }
            print_service(disk_queue[pos].second, disk_queue[pos].first);
            current_track = disk_queue[pos].first;

            if (!if_finished[disk_queue[pos].second]){
                if_requested[disk_queue[pos].second] = false;
                active_requester++;
            }
            disk_queue.erase(disk_queue.begin()+pos);
            current_disk_queue--;
        }
        queue.unlock();
        queue.lock();
    }
    queue.unlock();
    return;
}

void requester(void *a)
{
    uintptr_t num = (uintptr_t) a;

    std::ifstream ifs;
    ifs.open(file_names[num], std::ios::in);
    std::string buf;
    while (getline(ifs, buf)){
        queue.lock();
        cv_queue.wait(queue);
        if (!if_requested[num] & (current_disk_queue < max_disk_queue)){
            int track = stoi(buf);
            disk_queue.push_back(std::make_pair(track,num));
            current_disk_queue++;
            if_requested[num] = true;
            active_requester--;
            print_request(num, track);
            //ask_for_request.signal();
        }
        queue.unlock();
    }
    queue.lock();
    num_finished++;
    if_finished[num] = true;
    ifs.close();
    queue.unlock();
    return;
}


void main_thread(void *a)
{
    thread service ((thread_startfunc_t) servicer, (void *) "This is servicer");

    for (int i = 0; i < num_disks; i ++){
        uintptr_t num = i;
        thread request  ((thread_startfunc_t) requester, (void *) num);
    }

}

int main(int argc, char *argv[])
{

    max_disk_queue = atoi(argv[1]);
    num_disks = argc - 2;
    active_requester = num_disks;

    for(int i=0; i<num_disks; i++) {
        file_names.push_back(argv[i+2]);
        if_requested.push_back(false);
        if_finished.push_back(false);
	}

    cpu::boot((thread_startfunc_t) main_thread, (void *) 0, 0);    
}