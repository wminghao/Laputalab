#ifndef __PIPE_TABLE__
#define __PIPE_TABLE__

#include <unordered_map>
#include "Output.h"
#include "ProcessPipe.h"

//process pipe table.
const int MAX_PROCESS_PIPES = 4; //max 32 instances at the same time.
typedef pair <int, ProcessPipe*> Int_Pipe_Pair;
const unsigned int maskArray[ ] = {0x7fffffff,
                                   0xbfffffff,
                                   0xdfffffff,
                                   0xefffffff,

                                   0xf7ffffff,
                                   0xfbffffff,
                                   0xfdffffff,
                                   0xfeffffff,

                                   0xff7fffff,
                                   0xffbfffff,
                                   0xffdfffff,
                                   0xffefffff,

                                   0xfff7ffff,
                                   0xfffbffff,
                                   0xfffdffff,
                                   0xfffeffff,

                                   0xffff7fff,
                                   0xffffbfff,
                                   0xffffdfff,
                                   0xffffefff,

                                   0xfffff7ff,
                                   0xfffffbff,
                                   0xfffffdff,
                                   0xfffffeff,

                                   0xffffff7f,
                                   0xffffffbf,
                                   0xffffffdf,
                                   0xffffffef,

                                   0xfffffff7,
                                   0xfffffffb,
                                   0xfffffffd,
                                   0xfffffffe };
const unsigned int ALL_PIPE_OCCUPIED = 0xffffffff;

class PipeTable {
 public:
    PipeTable(const char* procPath) {
        for(int i = 0; i< MAX_PROCESS_PIPES; i++) {
            pipeMap_.insert(Int_Pipe_Pair(i, new ProcessPipe(procPath)));
        }
        pipeMask_ = 0;        
    }
    //return index
    int acquireUnusedPipe() {
        int index = -1;
        if( pipeMask_ != ALL_PIPE_OCCUPIED ) {
            for( int i=0; i < MAX_PROCESS_PIPES ; i++) {
                if( (pipeMask_ & (~maskArray[i])) == 0 ) {
                    pipeMask_ |= (~maskArray[i]);
                    index = i;
                    OUTPUT("acquireUnusedPipe, index=%d\r\n", index);
                    break;
                }
            }
        }
        return index;
    }
    
    void releasePipe(int index) {
        if( index >= 0 && index < MAX_PROCESS_PIPES ) {
            pipeMask_ &= maskArray[index];
            OUTPUT("releasePipe, index=%d\r\n", index);
        }
    }

    ProcessPipe* get(int index) {
        return pipeMap_[index];
    }
 private:
    unordered_map <int, ProcessPipe*> pipeMap_;
    unsigned int pipeMask_;
};

#endif
