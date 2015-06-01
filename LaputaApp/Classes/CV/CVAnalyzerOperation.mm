//
//  CVAnalyzerOperation.mm
//  Laputa
//
//  Created by Howard Wang on 15-5-7.
//
//

#import "CVAnalyzerOperation.hh"
#include "CVAnalyzer.h"

const int MAX_NUMBER_CONCURRENT_OPERATIONS = 1;

@interface CVAnalyzerOperation ()
{
    NSOperationQueue *myQueue;
    CVAnalyzer cvAnalyzer_[MAX_NUMBER_CONCURRENT_OPERATIONS];
    bool lock[MAX_NUMBER_CONCURRENT_OPERATIONS];
}
@end

static void getMat(CVPixelBufferRef pixelBuffer, Mat& image) {
    CVPixelBufferLockBaseAddress( pixelBuffer, 0 );
    
    unsigned char *base = (unsigned char *)CVPixelBufferGetBaseAddress( pixelBuffer );
    size_t width = CVPixelBufferGetWidth( pixelBuffer );
    size_t height = CVPixelBufferGetHeight( pixelBuffer );
    size_t stride = CVPixelBufferGetBytesPerRow( pixelBuffer );
    size_t extendedWidth = stride / sizeof( uint32_t ); // each pixel is 4 bytes/32 bits
    assert(width == extendedWidth);
    // Since the OpenCV Mat is wrapping the CVPixelBuffer's pixel data, we must do all of our modifications while its base address is locked.
    // If we want to operate on the buffer later, we'll have to do an expensive deep copy of the pixel data, using memcpy or Mat::clone().
    
    // Use extendedWidth instead of width to account for possible row extensions (sometimes used for memory alignment).
    // We only need to work on columms from [0, width - 1] regardless.
    // OpenCV works in BGRA format
    Mat frame = cv::Mat( (int)height, (int)extendedWidth, CV_8UC4, base );
    CVPixelBufferUnlockBaseAddress( pixelBuffer, 0 );
    
    //then resize
    //original scale is: 16/9 = 640/360
    cv::Size size(640, 360);
    resize( frame, image, size );
    
    //then flip and convert color
    flip(image, image, 1);
    
    //cv::Size s = image.size();
    //cout<<" image width="<<s.width << " height="<<s.height;
}

@implementation CVAnalyzerOperation

- (instancetype)init
{
    self = [super init];
    if ( self ) {
        myQueue = [[NSOperationQueue alloc] init];
        myQueue.name = @"CV analyzer Queue";
        myQueue.MaxConcurrentOperationCount = MAX_NUMBER_CONCURRENT_OPERATIONS;
        for(int i=0; i<MAX_NUMBER_CONCURRENT_OPERATIONS; i++) {
            cvAnalyzer_[i].init("");
        }
    }
    return self;
}

-(void)processImage:(CVPixelBufferRef)pixelBuffer
{
    __block Mat image;
    getMat(pixelBuffer, image);
    
    if ( !image.empty() ) {
        [myQueue cancelAllOperations];
        [myQueue addOperationWithBlock: ^ {
            
            int idleAnalyzer = -1;
            @synchronized(self) {
                for( int i=0; i<MAX_NUMBER_CONCURRENT_OPERATIONS; i++ ) {
                    if( !lock[i] ) {
                        idleAnalyzer = i;
                        lock[i] = true;
                        break;
                    }
                }
            }
            if( idleAnalyzer != -1 ) {
                NSLog(@"idleAnalyzer %d starts, total queue size=%ld", idleAnalyzer, (unsigned long)[myQueue.operations count]);
                cvAnalyzer_[idleAnalyzer].processImage(image, false); //TODO later
                /*
                 // Update UI on the main thread.
                 [[NSOperationQueue mainQueue] addOperationWithBlock: ^ {
                 weakSelf.imageView.image = image;
                 }];
                 */
            }
            @synchronized(self) {
                lock[idleAnalyzer] = false;
            }
        }];
    }
}
@end
