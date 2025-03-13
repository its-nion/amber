:: Compile compute shader to runtime format (.spv)
%VULKAN_SDK%\Bin\glslc.exe %~dp0warpedFbm.comp -o %~dp0warpedFbm.spv

pause