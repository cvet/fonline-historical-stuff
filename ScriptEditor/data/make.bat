cl.exe /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Fo".\\" /Fd".\\" /FD /c ".\\dll.cpp"
link.exe /nologo /dll /incremental:no /pdb:".\\dll.pdb" /machine:I386 /out:".\\dll.dll" /implib:".\\dll.lib" ".\\dll.obj"
del ".\\dll.obj"
del ".\\dll.exp"
del ".\\dll.lib"
del ".\\vc60.idb"