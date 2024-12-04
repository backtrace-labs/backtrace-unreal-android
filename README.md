Integrate the [backtrace-android](https://github.com/backtrace-labs/backtrace-android) error reporting library with your Unreal Engine game for Android written in Java or Kotlin.

1. Download [BacktraceWrapper](https://github.com/backtrace-labs/backtrace-unreal-android/releases) package.
1. In the directory for your Unreal Engine project, locate your app or game's `Build.cs` file.
1. Place the `BacktraceAndroid_UPL.xml` file in the same directory with the `Build.cs` file.
1. In the `Build.cs` file, add the following lines at the end of the `ModuleRules` class constructor:

   ```
   if (Target.Platform == UnrealTargetPlatform.Android)
   {
     string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
     AdditionalPropertiesForReceipt.Add("AndroidPlugin", System.IO.Path.Combine(PluginPath, "BacktraceAndroid_UPL.xml"));
   }
   ```

1. Place the `BacktraceWrapper.h` and `BacktraceWrapper.cpp` files in the same directory and add BacktraceWrapper to your GameInstance.
1. To initialize the Backtrace client, use `BacktraceIO::FInitializeBacktraceClient` with the name of your [subdomain and a submission token](https://docs.saucelabs.com/error-reporting/platform-integrations/unreal/setup/#what-youll-need) as a SubmissionUrl.
     ```cpp
     FString SubmissionUrl = TEXT("https://submit.backtrace.io/{subdomain}/{submission-token}/json");
     BacktraceIO::FInitializeBacktraceClient(SubmissionUrl, Attributes, Attachments);
     ```   
   
   >**Note:**
   >It's recommended to initialize the client from the `GameInstance::OnStart()` method. However, if the method is not available, you can initialize the client with any method you use to start your app or game process.
   >
   >Optionally, you can specify custom attributes and file attachment paths to submit with your error reports. If you choose to specify file attachment paths, they must be specified as Android paths. For example, to specify a file attachment path for your `ProjectSavedDir()`, use:
   ```
   if (Target.Platform == UnrealTargetPlatform.Android)
   #include "Misc/App.h"
   #if PLATFORM_ANDROID
   extern FString GFilePathBase;
    FString FileName = TEXT("MyFileName.txt");
    FilePath = GFilePathBase + FString("/UE5Game/") + FApp::GetName() + TEXT("/") + FApp::GetName() + TEXT("/Saved/") + FileName;
   #endif
   ```
   For more details on how to convert your Unreal Engine paths to Android paths, see the conversion functions for `FAndroidPlatformFile::PathToAndroidPaths` in the `AndroidPlatformFile.cpp` file.

To change the default configuration settings for the Backtrace client, you can change the settings in the `BacktraceAndroid_UPL.xml` file. For more information, see [Configuring Backtrace for Android](https://docs.saucelabs.com/error-reporting/platform-integrations/android/configuration/) for the backtrace-android library.
