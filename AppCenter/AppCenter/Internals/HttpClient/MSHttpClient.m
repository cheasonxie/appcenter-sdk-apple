// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#import "MSHttpClient.h"
#import "MSAppCenterInternal.h"
#import "MSConstants+Internal.h"
#import "MSHttpCall.h"
#import "MSHttpClientPrivate.h"
#import "MSIngestionUtil.h"
#import "MSLoggerInternal.h"
#import "MSUtility+StringFormatting.h"
#import "MS_Reachability.h"

@implementation MSHttpClient

- (instancetype)init {
  if ((self = [super init])) {
    _session = [NSURLSession new];
    _pendingCalls = [NSMutableSet new];
    _reachability = [MS_Reachability new];
    _retryIntervals = @[ @1.0 ];
  }
  return self;
}

- (instancetype)initWithMaxHttpConnectionsPerHost:(int)maxHttpConnectionsPerHost {
  if ((self = [super init])) {
    NSURLSessionConfiguration *config = [NSURLSessionConfiguration new];
    config.HTTPMaximumConnectionsPerHost = maxHttpConnectionsPerHost;
    _session = [NSURLSession sessionWithConfiguration:config];
    _pendingCalls = [NSMutableSet new];
    _reachability = [MS_Reachability new];
    _retryIntervals = @[ @1.0 ];
  }
  return self;
}

- (instancetype)initWithRetryIntervals:(NSArray *)retryIntervals reachability:(MS_Reachability *)reachability {
  if ((self = [super init])) {
    _session = [NSURLSession new];
    _pendingCalls = [NSMutableSet new];
    _reachability = reachability;
    _retryIntervals = [NSArray arrayWithArray:retryIntervals];
  }
  return self;
}

- (void)sendAsync:(NSURL *)url
               method:(NSString *)method
              headers:(nullable NSDictionary<NSString *, NSString *> *)headers
                 data:(nullable NSData *)data
    completionHandler:(MSHttpRequestCompletionHandler)completionHandler {
  MSHttpCall *call = [[MSHttpCall alloc] initWithUrl:url
                                              method:method
                                             headers:headers
                                                data:data
                                      retryIntervals:self.retryIntervals
                                   completionHandler:completionHandler];
  [self sendCallAsync:call];
}

- (void)sendCallAsync:(MSHttpCall *)call {
  @synchronized(self) {
    [self.pendingCalls addObject:call];
  }
  NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:call.url];
  request.HTTPBody = call.data;
  request.HTTPMethod = call.method;
  request.allHTTPHeaderFields = call.headers;

  // Always disable cookies.
  [request setHTTPShouldHandleCookies:NO];

  // Don't lose time pretty printing headers if not going to be printed.
  if ([MSLogger currentLogLevel] <= MSLogLevelVerbose) {
    MSLogVerbose([MSAppCenter logTag], @"URL: %@", request.URL);
    MSLogVerbose([MSAppCenter logTag], @"Headers: %@", [self prettyPrintHeaders:request.allHTTPHeaderFields]);
  }
  NSURLSessionDataTask *task =
      [self.session dataTaskWithRequest:request
                      completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
                        @synchronized(self) {

                          /*
                           * If the call was canceled, then it won't have been removed above, and thus, we should not call the completion
                           * handler, because the cancelation would have already done so.
                           */
                          if ([self.pendingCalls containsObject:call]) {
                            [self.pendingCalls removeObject:call];
                          } else {
                            return;
                          }
                        }

                        // Unblock the caller now with the outcome of the call.
                        NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;
                        call.completionHandler(data, httpResponse, error);

                        // Log error payload.
                        if (error) {
                          MSLogDebug([MSAppCenter logTag], @"HTTP request error with code: %td, domain: %@, description: %@", error.code,
                                     error.domain, error.localizedDescription);
                        }

                        // Don't lose time pretty printing if not going to be printed.
                        else if ([MSAppCenter logLevel] <= MSLogLevelVerbose) {
                          NSString *payload = [MSUtility prettyPrintJson:data];
                          MSLogVerbose([MSAppCenter logTag], @"HTTP response received with status code: %tu, payload:\n%@",
                                       httpResponse.statusCode, payload);
                        }
                      }];
  [task resume];
}

- (NSString *)obfuscateHeaderValue:(NSString *)value forKey:(NSString *)key {
  if ([key isEqualToString:kMSAuthorizationHeaderKey]) {
    return [MSIngestionUtil hideAuthToken:value];
  } else if ([key isEqualToString:kMSHeaderAppSecretKey]) {
    return [MSIngestionUtil hideSecret:value];
  }
  return value;
}

- (NSString *)prettyPrintHeaders:(NSDictionary<NSString *, NSString *> *)headers {
  NSMutableArray<NSString *> *flattenedHeaders = [NSMutableArray<NSString *> new];
  for (NSString *headerKey in headers) {
    [flattenedHeaders
        addObject:[NSString stringWithFormat:@"%@ = %@", headerKey, [self obfuscateHeaderValue:headers[headerKey] forKey:headerKey]]];
  }
  return [flattenedHeaders componentsJoinedByString:@", "];
}

@end
