// Copyright (o) 2016-2020 Code 4 Game <develop@c4g.io>

using UnrealBuildTool;
using System.IO;
using System.Collections.Generic;

public class popH264_ue4 : ModuleRules
{
    public popH264_ue4(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.External;

        string popH264Path = System.IO.Path.Combine(ModuleDirectory, "popH264-1.3.38");
        string IncludePath = System.IO.Path.Combine(popH264Path, "include");
        List<string> LibPaths = new List<string>();
        List<string> LibFilePaths = new List<string>();

        if ((Target.Platform == UnrealTargetPlatform.Win32) || (Target.Platform == UnrealTargetPlatform.Win64))
        {
            string PlatformName = "";
#if UE_4_23_OR_LATER
            if (Target.Platform == UnrealTargetPlatform.Win32)
            {
                PlatformName = "win32";
            }
            else if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                PlatformName = "win64";
            }
#else
            switch (Target.Platform)
            {
                case UnrealTargetPlatform.Win32:
                    PlatformName = "win32";
                    break;
                case UnrealTargetPlatform.Win64:
                    PlatformName = "win64";
                    break;
            }
#endif

            string TargetConfiguration = "Release";
            if (Target.Configuration == UnrealTargetConfiguration.Debug)
            {
                TargetConfiguration = "Debug";
            }


            PublicLibraryPaths.Add(Path.Combine(popH264Path, "lib"));
            PublicAdditionalLibraries.Add(Path.Combine(popH264Path, "lib", PlatformName, TargetConfiguration, "PopH264.lib"));
            //PublicAdditionalLibraries.Add(Path.Combine(popH264Path, "lib", PlatformName, TargetConfiguration, "PopH264.dll"));
            //PublicAdditionalLibraries.Add(Path.Combine(popH264Path, "lib", PlatformName, TargetConfiguration, "PopH264.exp"));
            //PublicAdditionalLibraries.Add(Path.Combine(popH264Path, "lib", PlatformName, TargetConfiguration, "PopH264.pdb"));
            //PublicAdditionalLibraries.Add(Path.Combine(popH264Path, "lib", PlatformName, TargetConfiguration, "x264.dll"));
            PublicDelayLoadDLLs.Add("PopH264.dll");
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            LibPaths.Add(System.IO.Path.Combine(popH264Path, "lib", "linux"));

            LibFilePaths.Add("libPopH264.so");
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            LibPaths.Add(System.IO.Path.Combine(popH264Path, "lib", "macos"));

            LibFilePaths.Add("PopH264_Osx.framework");
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            LibPaths.Add(System.IO.Path.Combine(popH264Path, "lib", "android", "armeabi-v7a"));
            LibPaths.Add(System.IO.Path.Combine(popH264Path, "lib", "android", "arm64-v8a"));
            LibPaths.Add(System.IO.Path.Combine(popH264Path, "lib", "android", "x86"));
            LibPaths.Add(System.IO.Path.Combine(popH264Path, "lib", "android", "x86_64"));

            LibFilePaths.Add("libc++_shared.so");
            LibFilePaths.Add("libPopH264.so");
        }
        else if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            LibPaths.Add(System.IO.Path.Combine(popH264Path, "lib", "ios", "ios-arm64"));
            LibPaths.Add(System.IO.Path.Combine(popH264Path, "lib", "ios", "ios-arm64_x86_64-simulator"));

            LibFilePaths.Add("PopH264_Ios.framework");
        }

        PublicIncludePaths.Add(IncludePath);
        PublicLibraryPaths.AddRange(LibPaths);
        PublicAdditionalLibraries.AddRange(LibFilePaths);
    }
}
