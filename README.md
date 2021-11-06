# A Formal Analysis of The Average (Detected) Fortnite Cheat

[Click here for read the analysis](https://github.com/0dayatday0/BattleFN-cheat-analysis/blob/main/cheat-analysis.pdf) 

Average Fortnite cheat paster trying to save his ass in front of his scammed clients:
![MrXiaoBullshit](https://cdn.discordapp.com/attachments/867531432179007488/906216104689541150/unknown.png)

# Use the stupidness of a paster in your favour!

# easy-kernelmapper - Manual map your driver with a batch using a valid signed driver

The cheat pseudo-developer was so dumb to sign a driver with a valid certificate that manual map any driver present in C:\driver.sys.

## Instructions
Put in a folder together the following files:

mmap_driver.bat

mapper_driver.sys - from the cheat modules repo directory

mapped_driver.sys - your driver to manual map

Run the batch as administrator et voilà

No need to take care of cleaning up the mapper as it already self-unload and delete the service registry key.

## Notes
No additional note to add beside how someone can be that clueless in what hes doing

# easy-rw - PoC for reading/writing process memory using BattleFN's cheat kernel modules

This is a PoC that can let you have r/w access to any process that is backed by any anticheat which strip handles rights, using standard Windows APIs.

I made this for two purposes:

1: To show off the analysis being a really accurate resume of the whole BattleFN working mechanism (in contrast to the developers lies where he says that it misses the most important point just to try to save his product)

2: Incase you want a copy and paste ready window to access any protected process memory, you can use that. You'll probably end in making a less detected cheat then its one, using the same stuff he pasted. 

## Instructions
Build it just by opening the Visual Studio project file.

Copy onto the just builded executable the following files from "cheat modules" repo directory:

vmdrv.sys

mapper_driver.sys

mapped_driver.sys

DLLVMhk.dll

Open a command prompt as administrator and run the command:
**"easy-rw.exe \<process id\>"**

If it success, it will write to the console the first 2 bytes of the image of the process you specified (4D 5A)

## Notes

There's just one thing to explain which isn't wrote in the PDF:

    typedef void(*initialize_process_pid_prototype)(HANDLE pid);
    initialize_process_pid_prototype initialize_process_pid_function = 
		(initialize_process_pid_prototype)GetProcAddress(LoadLibraryW(L"gdi32.dll"), "D3DKMTVailDisconnect");
    initialize_process_pid_function(process);

The user-mode module the external cheat process uses hook also an API that the host process must call in order to make the User Mode - Kernel Mode communication working.
In order to perform memory operations, you have to set a global variable present in the DLL that describe the process handle the process should use the driver to read or write memory.
The cheat does it by calling gdi32!D3DKMTVailDisconnect (the API being hooked) with the target process handle as parameter.
If you don't do that, the usermode DLL DLLVMhk wont issue IOCTLs to the driver.

The pseudo-developer Mr Xiao probably did that in order to try to stop the attempt to let others use his bad coded kernel modules.
