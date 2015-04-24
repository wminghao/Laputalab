#ifndef __PENDING_TASK__
#define __PENDING_TASK__

#include "utility/SmartPtr.h"
#include "utility/SmartPtrInterface.h"
#include "Client.h"
#include <unordered_map>

class PendingTask: public SmartPtrInterface<PendingTask>
{
 public:
    PendingTask(char* urlStr, int urlLen){
        urlStr_ = (char*) malloc(urlLen);
        memcpy(urlStr_, urlStr, urlLen);
        urlLen_ = urlLen;
    }
    ~PendingTask(){
        free(urlStr_);
        urlStr_ = NULL;
        urlLen_ = 0;
    }
    char* getUrlStr(){ return urlStr_; }
    int getUrlLen() { return urlLen_; }
 private:
    char* urlStr_;
    int urlLen_;
};

using namespace std;
typedef unordered_map<Client*, SmartPtr<PendingTask> > PendingMap; 
class PendingTaskTable
{
 public:
    PendingTaskTable(){};
    void addTask(Client* client, char* urlStr, int urlLen){
        pendingTaskTable_[client] = new PendingTask(urlStr, urlLen);
    }
    
    void removeTask(Client* client) {
        pendingTaskTable_.erase(client);
    }
    
    SmartPtr<PendingTask> getNext(Client*& client) {
        PendingMap::iterator it = pendingTaskTable_.begin();
        if( it != pendingTaskTable_.end() ) {
            client = it->first;
            return it->second;
        } else {
            return NULL;
        }
    }
 private:
    PendingMap pendingTaskTable_;
};

#endif//__PENDING_TASK__
