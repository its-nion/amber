:: Volumetric Clouds
%VULKAN_SDK%\Bin\glslc.exe %~dp0warpedFbm.comp -o %~dp0warpedFbm.spv

pause
::exit

D:\Portable\bin2cpp_3.0.0\bin\bin2cpp.exe --file=warpedFbm.spv --output=%~dp0..\visual-studio\shader

pause
::exit