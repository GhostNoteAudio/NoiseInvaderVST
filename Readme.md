# Introduction

It came to my attention that there is a serious lack of good noise gates available for free. To address this need, I decided to design this noise gate algorithm and release it as a free (as in beer and speech) VST plugin to the world. I hope you will find it as useful as I do. 

If you are a developer and you find yourself in need of a decent noise gate for your product, feel free to use this algorithm, it is completely free and comes with no restrictions. 

## Download

Get the latest download from the **[Release page](https://github.com/GhostNoteAudio/NoiseInvaderVST/releases/tag/2022-06-08)**.

This plugin works on 64 bit Windows platforms.
If you get get an error while loading the plugin in your DAW, you may need to download and install the **[Visual C++ Runtime (2019) available directly from Microsoft](https://docs.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170)**. (Direct Download link: https://aka.ms/vs/17/release/vc_redist.x64.exe )

## How to Install and use

A Quick Start guide is **[available online](https://github.com/GhostNoteAudio/NoiseInvaderVST/blob/master/Noise%20Invader%203.0%20-%20Quick%20Start.pdf)**.

# For Developers Only

## Building from Source

In order to build from source you must obtain a copy of the VST SDK from Steinberg, as they do not allow the SDK to be distributed by 3rd parties. This plugin uses VST 2.4, which is included as part of the most recent (version 3) SDK.

Once you've obtained the SDK, modify the include directories to point to the location of the SDK. Finally, one minor change has to be done to the vstpluginmain.cpp file.

Replace the following line:

    #define VST_EXPORT

with

    #define VST_EXPORT __declspec(dllexport)

This will export the vstmain function so that it is available as a function in your dll file. If you don't do this change, you can also specify a list of functions to be exported in VST, but I prefer to use this way instead.

License: MIT License  (see License.txt)
