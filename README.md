# F4DConverter
This application, F4DConverter, is for converting popular 3D model formats into F4D format
which is devised for Mago3D - 3D web geo-platform. (www.mago3d.com).
This project is of Microsoft Visual Studio 2015 C++ project.

## developer's comments before reading more ##
- F4DConverter runs only in Windows 7 or later version of 64-bit OS.
- This is window-based BUT FAKES as it runs in CLI mode. So you have to run this in cmd.exe. During long conversion time due to large size of input data, the window changes as it is crashed, but it is NOT crashed. 
- Full information and Window installer are on www.mago3d.com (F4D_Specification.pdf gives only detailed F4D dataset structures.)
- We've checked if we can open all sample raw data we have(.ifc and .3ds with good quality), but we can't because of license issues. 

## supported input formats ##
- .ifc
- .3ds
- .obj
- .dae

> Beside above formats, other formats which are supported by Assimp may be supported.(NOT TESTED!!)
>
> In this version, .JT(Jupiter Tessellation, a kind of cad design format) is not included.

## necessary libraries for F4DConverter ##
- OpenSceneGraph 3.4.0 : http://www.openscenegraph.org
- ifcplusplus : https://github.com/ifcquery/ifcplusplus
- Carve : https://github.com/ifcquery/ifcplusplus
- Assimp 3.2 : http://assimp.sourceforge.net/main_downloads.html
- boost 1.62 : http://www.boost.org/users/history/version_1_62_0.html
- SOIL : http://www.lonesock.net/soil.html
- glew 2.0 : http://glew.sourceforge.net/

> ifcplusplus, Assimp, SOIL, and glew are for F4DConverter directly.
>
> Carve, boost, and OpenSceneGraph are for ifcplusplus.

## how to use ##
### sample arguments ###
- -inputFolder D:/data/data_3ds/DC_library_del_3DS -outputFolder D:/data/conversionResult -log D:/data/conversionResult/logTest.txt -idPrefix design_ -idSuffix _2017 -oc y -usf 0.01
- -outputFolder D:/dataConverted -indexing y
### detailed information ###
- -inputFolder [rawDataFolder] : an absolute path of the folder where raw data files to be converted are.
- -outputFolder [F4DFolder] : an absolute path of the folder where conversion results(F4D datasets) are created.
- -log [logFileFullPath] : an absolute path of a log file which is created after finishing conversion processes.
- -idPrefix [prefix] : a prefix used in name of an F4D folder.
- -idSuffix [suffix] : a suffix used in name of an F4D folder.
- -oc [one of Y, y, N, n] : whether visibility indices for occlusion culling should be created or not. "NOT created" is default.
- -usf [numericValue] : unit scale factor. Geometries in F4D are described in meter. That is, the unit scale factor of raw data descrived in centimeter is 0.01 for example.
- -indexing [one of Y, y, N, n] : wheter objectIndexFile.ihe should be created or not. "NOT created" is default.
> At least one of "-inputFolder" and "-indexing" is mandatory. Both arguments can be used together.
>
> "-outputFolder" and "-log" are mandatory when -inputFolder is used.
>
> "-outputFolder" is mandatory when -indexing is used. (So "-outputFolder" is mandatory in any case.)
>
> It takes very looooooong time to create visibility indices. If "-oc y" is used, 99% of total conversion time is used in creaing visibility indices.
>
> When "-idPrefix" and/or "-idSuffix" are used, the name of created F4D folder is F4D_|prefix|originalDataFileName|suffix|.
>
> All folder paths injected MUST exist before running the converter. F4DConverter doesn't create folders automatically.

## stuffs under development or to be developed ##
> Priority is not considered.
- extracting parameters used in processing out to arguments.
(ex : resolution of spatial octree / visibility indices, changing coordinate axis, making center of bounding box into local origin, level of triangle sorting in vbo, and so on)
- removing Windows OS dependency, that is, making pure console app which can be run in both Windows and Linux family.
- applying parallel processing, specially on visibility indexing.
- converting multiple raw data files into single F4D folder.
- Supporting lego structures of multiple LOD.
- Designing and implementing tile pyramid made of only lego structures out of all F4D datasets on service.
- Creating a texture image for lego structures regardless of whether raw data has texture images or not.
