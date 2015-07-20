#ifndef __FWK_SMARTBUFFER_H__
#define __FWK_SMARTBUFFER_H__

#include "SmartPtr.h"
#include "SmartPtrInterface.h"
#include <string.h>
#include <string>

class SmartBuffer;
typedef SmartPtr<SmartBuffer> SmartBufferPtr;
class SmartBuffer : public SmartPtrInterface<SmartBuffer>
{
 public:
 SmartBuffer( size_t dataLength, const u8 *data )
     : dataLength_(dataLength), more_( false ) {
        data_ = new u8[dataLength];
        memcpy( data_, data, dataLength );
    }
    
 SmartBuffer( size_t dataLength, const char *data )
     : dataLength_(dataLength), more_( false ) {
        data_ = new u8[dataLength];
        memcpy( data_, data, dataLength );
    }
    
 SmartBuffer( const std::string& str)
     : dataLength_(str.length()), more_( false ) {
        data_ = new u8[dataLength_];
        memcpy( data_, str.c_str(), dataLength_ );
    }
    
 SmartBuffer( size_t dataLength ) 
     : dataLength_(dataLength), more_( false ) {
        data_ = new u8[dataLength];
    }
    
    virtual ~SmartBuffer() {
        delete[] data_;
    }
    
    virtual size_t dataLength() const { return dataLength_; }
    virtual u8 *data() const { return data_; }
    
    virtual bool hasSameDataAs( const SmartBufferPtr &b ) const { 
        return ( dataLength_ == b->dataLength_ ) && 
            ( !memcmp( data_, b->data_, dataLength_ ) );
    }

    static SmartPtr<SmartBuffer> genBlankBuffer(SmartPtr<SmartBuffer> a)
    {
        u32 bufSize = a->dataLength();
        SmartPtr<SmartBuffer> c = new SmartBuffer(bufSize);
        if( c ) {
            memset(c->data(), 0, bufSize);
        }
        return c;
    }
    
 private:
    SmartBuffer();
    SmartBuffer( const SmartBufferPtr& b );
    SmartBuffer& operator=( const SmartBufferPtr& b );
    
 private:
    size_t dataLength_;
    u8 *data_;
    
    bool more_;

 public:
    bool more() const { return more_; }
    void more( bool t ) { more_ = t; }
};

inline SmartPtr<SmartBuffer> combine2SmartBuffers(SmartPtr<SmartBuffer> one, SmartPtr<SmartBuffer> two ) {
    int totalLen = one->dataLength() + two->dataLength();

    SmartPtr<SmartBuffer> result = new SmartBuffer(totalLen);
    u8* data = result->data();
    memcpy(data, one->data(), one->dataLength());
    memcpy(data + one->dataLength(), two->data(), two->dataLength());
    return result;
}
#endif


