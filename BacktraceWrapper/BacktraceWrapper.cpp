#include "BacktraceWrapper.h"

DEFINE_LOG_CATEGORY(Backtrace);

namespace BacktraceIO
{
#if PLATFORM_ANDROID
    // Helper to convert Unreal TMap to std::unordered_map
    std::unordered_map<std::string, std::string> ConvertTMapToStdMap(const TMap<FString, FString>& Map)
    {
        std::unordered_map<std::string, std::string> Result;
        for (const auto& Pair : Map)
        {
            std::string Key = TCHAR_TO_UTF8(*Pair.Key);
            std::string Value = TCHAR_TO_UTF8(*Pair.Value);
            Result[Key] = Value;
        }
        return Result;
    }

    // Helper to convert Unreal TArray to std::vector
    std::vector<std::string> ConvertTArrayToStdVector(const TArray<FString>& Array)
    {
        std::vector<std::string> Result;
        for (const FString& String : Array)
        {
            Result.push_back(TCHAR_TO_UTF8(*String));
        }
        return Result;
    }

    // Convert std::unordered_map to Java HashMap
    jobject FStlStringStringMapToJavaHashMap(JNIEnv* Env, const std::unordered_map<std::string, std::string>& Map)
    {
        jclass HashMapClass = Env->FindClass("java/util/HashMap");
        jmethodID HashMapConstructor = Env->GetMethodID(HashMapClass, "<init>", "()V");
        jmethodID HashMapPut = Env->GetMethodID(HashMapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

        jobject JavaHashMap = Env->NewObject(HashMapClass, HashMapConstructor);

        for (const auto& Pair : Map)
        {
            jstring Key = Env->NewStringUTF(Pair.first.c_str());
            jstring Value = Env->NewStringUTF(Pair.second.c_str());
            Env->CallObjectMethod(JavaHashMap, HashMapPut, Key, Value);
            Env->DeleteLocalRef(Key);
            Env->DeleteLocalRef(Value);
        }
        
        if (Env->ExceptionCheck())
            {
                UE_LOG(Backtrace, Error, TEXT("Exception occurred while putting entries into HashMap"));
                Env->ExceptionDescribe();
                Env->ExceptionClear();
                Env->DeleteLocalRef(JavaHashMap);
                return nullptr;
            }

        return JavaHashMap;
    }

    // Convert std::vector to Java ArrayList
    jobject FStlVectorStringToJavaListString(JNIEnv* Env, const std::vector<std::string>& Vector)
    {
        jclass ArrayListClass = Env->FindClass("java/util/ArrayList");
        if (!ArrayListClass)
        {
            UE_LOG(Backtrace, Error, TEXT("Failed to find java/util/ArrayList class"));
            return nullptr;
        }

        jmethodID ArrayListConstructor = Env->GetMethodID(ArrayListClass, "<init>", "()V");
        jmethodID ArrayListAdd = Env->GetMethodID(ArrayListClass, "add", "(Ljava/lang/Object;)Z");
        if (!ArrayListConstructor || !ArrayListAdd)
        {
            UE_LOG(Backtrace, Error, TEXT("Failed to get ArrayList constructor or add method"));
            return nullptr;
        }

        jobject JavaArrayList = Env->NewObject(ArrayListClass, ArrayListConstructor);
        if (!JavaArrayList)
        {
            UE_LOG(Backtrace, Error, TEXT("Failed to create new ArrayList object"));
            return nullptr;
        }        

        for (const std::string& String : Vector)
        {
            jstring JavaString = Env->NewStringUTF(String.c_str());
            if (!JavaString)
            {
                UE_LOG(Backtrace, Error, TEXT("Failed to create Java string for list entry"));
                Env->DeleteLocalRef(JavaArrayList);
                return nullptr;
            }      
              
            Env->CallBooleanMethod(JavaArrayList, ArrayListAdd, JavaString);
            Env->DeleteLocalRef(JavaString);
        }

        if (Env->ExceptionCheck())
        {
            Env->ExceptionDescribe();
            Env->ExceptionClear();
            Env->DeleteLocalRef(JavaArrayList);
            return nullptr;
        }
        

        return JavaArrayList;
    }

    // Initializes the Backtrace Client
    bool FInitializeBacktraceClient(const FString& SubmissionUrl, const TMap<FString, FString>& Attributes, const TArray<FString>& Attachments)
    {
        JNIEnv* Env = FAndroidApplication::GetJavaEnv();
        if (!Env)
        {
            UE_LOG(Backtrace, Error, TEXT("Failed to get JNI environment"));
            return false;
        }

        jobject Activity = FAndroidApplication::GetGameActivityThis();
        if (!Activity)
        {
            UE_LOG(Backtrace, Error, TEXT("Failed to get GameActivity instance"));
            return false;
        }

        jclass ActivityClass = Env->GetObjectClass(Activity);

        if (!ActivityClass)
        {
            UE_LOG(Backtrace, Error, TEXT("Failed to find GameActivity class"));
            return false;
        }

        jmethodID InitializeMethod = Env->GetMethodID(ActivityClass, "initializeBacktraceClient", "(Ljava/lang/String;Ljava/util/Map;Ljava/util/List;)V");
        if (!InitializeMethod)
        {
            UE_LOG(Backtrace, Error, TEXT("Failed to find 'initializeBacktraceClient' method"));
            return false;
        }

        // Convert Unreal to Java objects
        auto StdAttributes = ConvertTMapToStdMap(Attributes);
        auto StdAttachments = ConvertTArrayToStdVector(Attachments);

        jobject JavaAttributes = FStlStringStringMapToJavaHashMap(Env, StdAttributes);
        jobject JavaAttachments = FStlVectorStringToJavaListString(Env, StdAttachments);

        if (!JavaAttributes || !JavaAttachments)
        {
            UE_LOG(Backtrace, Error, TEXT("Failed to create Java objects for attributes or attachments"));
            return false;
        }
        
        std::string SubmissionUrlStd = TCHAR_TO_UTF8(*SubmissionUrl);
        jstring JavaSubmissionUrl = Env->NewStringUTF(SubmissionUrlStd.c_str());
        if (!JavaSubmissionUrl)
        {
            UE_LOG(Backtrace, Error, TEXT("Failed to convert SubmissionUrl to Java string"));
            Env->DeleteLocalRef(JavaAttributes);
            Env->DeleteLocalRef(JavaAttachments);
            return false;
        }

        // Call Java method
        Env->CallVoidMethod(Activity, InitializeMethod, JavaSubmissionUrl, JavaAttributes, JavaAttachments);

        if (Env->ExceptionCheck())
        {
            UE_LOG(Backtrace, Error, TEXT("JNI exception occurred during Backtrace client initialization"));
            Env->ExceptionDescribe();
            Env->ExceptionClear();

            Env->DeleteLocalRef(JavaSubmissionUrl);
            Env->DeleteLocalRef(JavaAttributes);
            Env->DeleteLocalRef(JavaAttachments);
            return false;
        }

        // Clean up
        Env->DeleteLocalRef(JavaAttributes);
        Env->DeleteLocalRef(JavaAttachments);

        UE_LOG(Backtrace, Log, TEXT("Backtrace client initialized successfully"));
        return true;
    }
#endif
}