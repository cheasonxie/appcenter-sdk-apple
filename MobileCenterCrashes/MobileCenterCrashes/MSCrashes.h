#import "MSErrorReport.h"
#import "MSServiceAbstract.h"

@class MSCrashesDelegate;

/**
 * Custom block that handles the alert that prompts the user whether crash reports need to be processed or not.
 * @return Returns YES to discard crash reports, otherwise NO.
 */
typedef BOOL (^MSUserConfirmationHandler)(NSArray<MSErrorReport *> *_Nonnull errorReports);

/**
 * Error Logging status.
 */
typedef NS_ENUM(NSUInteger, MSErrorLogSetting) {
  /**
   * Crash reporting is disabled.
   */
  MSErrorLogSettingDisabled = 0,

  /**
   * User is asked each time before sending error logs.
   */
  MSErrorLogSettingAlwaysAsk = 1,

  /**
   * Each error log is send automatically.
   */
  MSErrorLogSettingAutoSend = 2
};

/**
 * Crash Manager alert user input
 */
typedef NS_ENUM(NSUInteger, MSUserConfirmation) {
  /**
   * User chose not to send the crash report
   */
  MSUserConfirmationDontSend = 0,
  /**
   * User wants the crash report to be sent
   */
  MSUserConfirmationSend = 1,
  /**
   * User wants to send all error logs
   */
  MSUserConfirmationAlways = 2
};

@protocol MSCrashesDelegate;

@interface MSCrashes : MSServiceAbstract

///-----------------------------------------------------------------------------
/// @name Testing Crashes Feature
///-----------------------------------------------------------------------------

/**
 * Lets the app crash for easy testing of the SDK.
 *
 * The best way to use this is to trigger the crash with a button action.
 *
 * Make sure not to let the app crash in `applicationDidFinishLaunching` or any
 * other
 * startup method! Since otherwise the app would crash before the SDK could
 * process it.
 *
 * Note that our SDK provides support for handling crashes that happen early on
 * startup.
 * Check the documentation for more information on how to use this.
 *
 * If the SDK detects an App Store environment, it will _NOT_ cause the app to
 * crash!
 */
+ (void)generateTestCrash;

///-----------------------------------------------------------------------------
/// @name Helpers
///-----------------------------------------------------------------------------

/**
 * Check if the app has crashed in the last session.
 *
 * @return Returns YES is the app has crashed in the last session.
 */
+ (BOOL)hasCrashedInLastSession;

/**
 * Provides details about the crash that occurred in the last app session
 */
+ (nullable MSErrorReport *)lastSessionCrashReport;

///-----------------------------------------------------------------------------
/// @name Configuration
///-----------------------------------------------------------------------------

/**
 * Disable to trap fatal signals via a Mach exception server.
 *
 * By default, the SDK uses the Mach exception handler to catch fatal signals, e.g. stack overflows, via a Mach
 * exception server. If you want or need to disable the Mach exception handler by any reasons, you _MUST_ call this
 * method _BEFORE_ starting the SDK. Your typical setup code would look like this:
 *
 * `[MSCrashes disableMachExceptionHandler]`;
 * `[MSMobileCenter start:@"YOUR_APP+_ID" withServices:@[[MSCrashes class]]];`
 *
 * or if you are using Swift:
 *
 * `MSCrashes.disableMachExceptionHandler()`
 * `MSMobileCenter.start("YOUR_APP+_ID", withServices: [MSAnalytics.self, MSCrashes.self])`
 *
 * Default: _YES_
 */
+ (void)disableMachExceptionHandler;

/**
 * Set the delegate
 *
 * Defines the class that implements the optional protocol
 * `MSCrashesDelegate`.
 *
 * @see MSCrashesDelegate
 */
+ (void)setDelegate:(_Nullable id<MSCrashesDelegate>)delegate;

/**
 * Set a user confirmation handler that is invoked right before processing crash reports to
 * determine whether sending crash reports or not.
 *
 * @param userConfirmationHandler A handler for user confirmation.
 * @see MSUserConfirmationHandler
 */
+ (void)setUserConfirmationHandler:(_Nullable MSUserConfirmationHandler)userConfirmationHandler;

/**
 * Notify SDK with a confirmation to handle the crash report.
 *
 * @param userConfirmation A user confirmation.
 * @see MSUserConfirmation.
 */
+ (void)notifyWithUserConfirmation:(MSUserConfirmation)userConfirmation;

@end
