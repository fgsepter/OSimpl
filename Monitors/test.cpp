#include <iostream>
#include <queue>
#include <vector>
using namespace std;

int main() 
{
    vector <pair<int,int>> my_queue;
    my_queue.push_back(make_pair(37,38));
    my_queue.push_back(make_pair(7,38));
    my_queue.push_back(make_pair(47,8));
    my_queue.push_back(make_pair(77,88));
    my_queue.erase(my_queue.begin()+1);
    cout << my_queue[2].first <<endl;
}