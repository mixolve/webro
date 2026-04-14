#pragma once

#import <Foundation/NSObject.h>

NS_ASSUME_NONNULL_BEGIN

@interface NSGarbageCollector : NSObject

+ (nullable NSGarbageCollector*) defaultCollector;
+ (BOOL) isEnabled;

- (void) collectExhaustively;
- (void) collectIfNeeded;
- (void) disable;
- (void) disableCollectorForPointer: (const void*) pointer;
- (void) enable;
- (void) enableCollectorForPointer: (const void*) pointer;

@end

FOUNDATION_EXPORT NSString* const NSGarbageCollectorInfo;

NS_ASSUME_NONNULL_END