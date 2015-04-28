#ifndef __PENDING_TASK__
#define __PENDING_TASK__

#include "utility/Output.h"
#include "utility/SmartPtr.h"
#include "utility/SmartPtrInterface.h"
#include "Client.h"
#include <unordered_map>
#include <deque>

const int MAX_PENDING_TASKS = 500; //no more than 100 pending tasks can be initiated at the same time.

class PendingTask: public SmartPtrInterface<PendingTask>
{
 public:
    PendingTask(char* urlStr, int urlLen):urlLen_(0){
        urlStr_ = (char*) malloc(urlLen);
        if( urlStr_ ) {
            memcpy(urlStr_, urlStr, urlLen);
            urlLen_ = urlLen;
        }
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
    bool addTask(Client* client, char* urlStr, int urlLen){
        bool bRet = false;
        if( clientQueue_.size() < MAX_PENDING_TASKS ) {
            pendingTaskTable_.insert(PendingMap::value_type(client, new PendingTask(urlStr, urlLen)));
            clientQueue_.push_back(client);
            bRet = true;
        }
        return bRet;
    }
    
    void removeNext() {
        Client* client = clientQueue_.front();
        pendingTaskTable_.erase(client);
        clientQueue_.pop_front();
    }
    
    SmartPtr<PendingTask> getNext(Client*& client) {
        if( clientQueue_.size() > 0 ) {
            client = clientQueue_.front();
            return pendingTaskTable_[client];
        }
        return NULL;
    }

    void removeTask(Client* client) {
        if( pendingTaskTable_.erase(client) > 0 ) {
            deque<Client*>::iterator found = clientQueue_.end();
            for (deque<Client*>::iterator itr=clientQueue_.begin(); itr!=clientQueue_.end(); ++itr) {
                if( *itr == client ) {
                    found = itr;
                    break;
                }
            }
            //found and remove
            if( found != clientQueue_.end() ) {
                clientQueue_.erase(found);
            }
        }
    }

 private:
    PendingMap pendingTaskTable_;
    deque<Client*> clientQueue_; //keep track of fifo order
};

#endif//__PENDING_TASK__
