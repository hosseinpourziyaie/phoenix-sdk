## PHOENIX-SDK
software development framework for overlaying directx11 graphical api display. it presents variety of features including developer console and component loading system.

its mainly designed to help user with developing multi-purpose graphical window overlay while maintaining high performance; specially implemented diagnostic logs saves developer's life.


## Abstract of Features
- **Directx11 Hook** : detour directx api present to draw overlay on top of host window
- **Key Catcher** : Hook window input and catch key strikes and intercept them if needed
- **Developer Console** : easy to use command console inspired by call of duty franchise's developer console
- **Engine Handler** : Conductor between host game engine(should be programmed to function) and phoenix
- **Diagnostic Tools** : Software Diagnosting Helping Class being worked on[wip]

## Showcase
![code](https://raw.githubusercontent.com/hosseinpourziyaie/phoenix-sdk/master/assets/screenshot_0001.jpg)
![code](https://raw.githubusercontent.com/hosseinpourziyaie/phoenix-sdk/master/assets/screenshot_0002.jpg)
![code](https://raw.githubusercontent.com/hosseinpourziyaie/phoenix-sdk/master/assets/screenshot_0003.jpg)
![code](https://raw.githubusercontent.com/hosseinpourziyaie/phoenix-sdk/master/assets/screenshot_0004.jpg)

## Directx12 Support
will be added eventually... feel free to contribute to project if you studied and have good sense of how directx12 overlaying works; its much appreciated

## Compile
project is structed super-neat and takes advantage of premake5 solution generation tool. dependencies are connected to main project and are compiled automatically prior to it; You just need VS2022 to do so.

## Notes
take in mind this project is a work in progress and far from being ideal sdk

## Credits
- [ImGui](https://github.com/ocornut/imgui) : graphical interface library by Ocornut
- [MinHook](https://github.com/TsudaKageyu/minhook) : software code-execution-flow hooking library by TsudaKageyu
- [Momo5502](https://github.com/momo5502) : component loader and slight portion of developer-console code
- I will try and add all inviduals that helped me in any way with this project over time

## Disclaimer
This software has been created purely for the purposes of academic research. It is not intended to be used to attack other systems. Project maintainers are not responsible or liable for misuse of the software. Use responsibly.


