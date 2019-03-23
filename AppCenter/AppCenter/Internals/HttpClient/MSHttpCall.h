// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#import "MSHttpClientProtocol.h"
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface MSHttpCall : NSObject

/**
 * Request body.
 */
@property(nonatomic) NSData *data;

/**
 * Request headers.
 */
@property(nonatomic) NSDictionary *headers;

/**
 * Request URL.
 */
@property(nonatomic) NSURL *url;

/**
 * HTTP method.
 */
@property(nonatomic, copy) NSString *method;

/**
 * Call completion handler used for communicating with calling component.
 */
@property(nonatomic) MSHttpRequestCompletionHandler completionHandler;

/**
 * A timer source which is used to flush the queue after a certain amount of time.
 */
@property(nonatomic) dispatch_source_t timerSource;

/**
 * Number of retries performed for this call.
 */
@property(nonatomic) int retryCount;

/**
 * Retry intervals for each retry.
 */
@property(nonatomic) NSArray *retryIntervals;

/**
 * Initialize a call with specified retry intervals.
 *
 * @param url The endpoint to use in the HTTP request.
 * @param method The HTTP method (verb) to use for the HTTP request (e.g. GET, POST, etc.).
 * @param headers HTTP headers.
 * @param data A data instance that will be transformed request body.
 * @param retryIntervals Retry intervals used in case of recoverable errors.
 * @param completionHandler Completion handler.
 *
 * @return A retriable call instance.
 */
- (instancetype)initWithUrl:(NSURL *)url
                     method:(NSString *)method
                    headers:(nullable NSDictionary<NSString *, NSString *> *)headers
                       data:(nullable NSData *)data
             retryIntervals:(NSArray *)retryIntervals
          completionHandler:(MSHttpRequestCompletionHandler)completionHandler;

/**
 * Indicate if the limit of maximum retries has been reached.
 *
 * @return YES if the limit of maximum retries has been reached, NO otherwise.
 */
- (BOOL)hasReachedMaxRetries;

/**
 * Reset and stop retrying.
 */
- (void)resetRetry;

@end

NS_ASSUME_NONNULL_END
