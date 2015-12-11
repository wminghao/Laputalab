//
//  main.cpp
//  Solution
//
//  Created by Howard Wang on 15-11-18.
//
/*
We have to find the integer solution of the below equation .

n<=sum of(ki*xi)<n+1

such that ki are all constants, xi are integers.

For example : 4<=1.4x1+3.4*x2+1.1*x3<5

solutions (x1,x2,x3) =(1,1,0),(3,0,0),(0,1,1),(0,0,4) ...
 */

#include <iostream>
#include <vector>

using namespace std;


int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    vector<double> results;
    int n = 5;
    
    int x1 = 0;
    int x2 = 0;
    int x3 = 0;
    
    double k[] = {1.4, 3.4, 1.1};
    
    while(true) {
        
        double val1 = k[0]*(x1+1)+k[1]*x2+k[2]*x3; //100
        double val2 = k[0]*(x1+1)+k[1]*(x2+1)+k[2]*x3; //110
        double val3 = k[0]*(x1+1)+k[1]*(x2+1)+k[2]*(x3+1); //111
        double val4 = k[0]*(x1+1)+k[1]*x2+k[2]*(x3+1);//101
        double val5 = k[0]*x1+k[1]*(x2+1)+k[2]*x3; //010
        double val6 = k[0]*x1+k[1]*(x2+1)+k[2]*(x3+1); //011
        double val7 = k[0]*x1+k[1]*x2+k[2]*(x3+1);//001
        
        x1++;
        x2++;
        x3++;
        
        int boundsRight = n+1;
        if(val1 > boundsRight &&
           val2 > boundsRight &&
           val3 > boundsRight &&
           val4 > boundsRight &&
           val5 > boundsRight &&
           val6 > boundsRight &&
           val7 > boundsRight
           ) {
            break;
        } else {
            
            results.push_back(val1);
            results.push_back(val2);
            results.push_back(val3);
            results.push_back(val4);
            results.push_back(val5);
            results.push_back(val6);
            results.push_back(val7);
            //sort it first
            sort(results.begin(), results.end());
            
            //then erase elements out of bounds
            vector<double>::iterator ptr = results.begin();
            while(ptr!=results.end()) {
                double val = *ptr;
                cout<<"cur val = "<<val<<endl;
                if( val < n || val>(n+1)) {
                    ptr = results.erase(ptr);
                } else {
                    ptr++;
                }
            }
        }
    }
    
    cout<<"total size="<<results.size()<<endl;
    vector<double>::iterator ptr = results.begin();
    while(ptr!=results.end()) {
        double val = *ptr;
        cout<<"val = "<<val<<endl;
        ptr++;
    }
    return 0;
}
