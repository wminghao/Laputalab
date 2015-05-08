//
//  CVAnalyzerOperation.h
//  Laputa
//
//  Created by Howard Wang on 15-5-7.
//
//

#import <Foundation/Foundation.h>

@interface CVAnalyzerOperation : NSObject
-(void)processImage:(CVPixelBufferRef) pixelBuffer andOnTap:(bool) onTap;
@end
