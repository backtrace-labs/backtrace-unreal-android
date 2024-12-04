#pragma once

#include "CoreMinimal.h"
#include <unordered_map>
#include <string>
#include <vector>

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include <jni.h>
#endif

// Backtrace log category
DECLARE_LOG_CATEGORY_EXTERN(Backtrace, Log, All);

namespace BacktraceIO
{
    /**
     * Initializes the Backtrace Client using provided attributes and attachments.
     *
     * @param SubmissionUrl Backtrace submission URL.
     * @param Attributes A map of key-value pairs to send as metadata with crash reports.
     * @param Attachments A list of Attachments file paths to include with crash reports.
     * @return True for successful initialization, False otherwise.
     */
    bool FInitializeBacktraceClient(const FString& SubmissionUrl, const TMap<FString, FString>& Attributes, const TArray<FString>& Attachments);

#if PLATFORM_ANDROID
    // JNI Conversion helpers
    std::unordered_map<std::string, std::string> ConvertTMapToStdMap(const TMap<FString, FString>& Map);
    std::vector<std::string> ConvertTArrayToStdVector(const TArray<FString>& Array);
    jobject FStlStringStringMapToJavaHashMap(JNIEnv* Env, const std::unordered_map<std::string, std::string>& Map);
    jobject FStlVectorStringToJavaListString(JNIEnv* Env, const std::vector<std::string>& Vector);
#endif
};