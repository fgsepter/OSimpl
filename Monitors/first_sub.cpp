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
std::queue<int> my_disks[1000];
bool if_requested[1000];
mutex queue;
cv cv_queue;
int num_solved = 0;
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
            num_solved++;
            // cout << num_solved << endl;
            // cout << "num_finished = " << num_finished << endl;
            current_track = disk_queue[pos].first;
            if_requested[disk_queue[pos].second] = false;
            if (!my_disks[disk_queue[pos].second].empty())
                active_requester++;
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

    while (!my_disks[num].empty()){
        queue.lock();
        cv_queue.wait(queue);
        if (!if_requested[num] & (current_disk_queue < max_disk_queue)){
            int track = my_disks[num].front();
            my_disks[num].pop();
            disk_queue.push_back(std::make_pair(track,num));
            current_disk_queue++;
            if_requested[num] = true;
            active_requester--;
            print_request(num, track);
        }
        if (!my_disks[num].empty())
            queue.unlock();
    }

    num_finished++;
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
    // cout << "max_disk_queue = " << max_disk_queue << endl;
    // cout << "num_disks = " << num_disks << endl;


    for(int i=0; i<num_disks; i++) {
        if_requested[i] = false;
        std::ifstream ifs;
        ifs.open(argv[i+2], std::ios::in);
        std::string buf;
        while (getline(ifs, buf))
            my_disks[i].push(stoi(buf));
        ifs.close();
	}
    cpu::boot((thread_startfunc_t) main_thread, (void *) 0, 1);    
}