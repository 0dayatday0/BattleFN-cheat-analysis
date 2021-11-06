# A Formal Analysis of The Average (Detected) Fortnite Cheat

https://github.com/0dayatday0/BattleFN-cheat-analysis/blob/main/cheat-analysis.pdf

# Extra: easy-rw - PoC for reading/writing process memory using BattleFN's cheat kernel modules

![MrXiaoBullshit](https://cdn.discordapp.com/attachments/867531432179007488/906216104689541150/unknown.png)

I've also made a PoC that can let you have r/w access to any process that is backed by any anticheat which strip handles rights.

I made this for two purposes:

1: To show off the analysis being a really accurate resume of the whole BattleFN working mechanism (in contrast to the developers lies where he says that it misses the most important point just to try to save his product)
2: Incase you want a copy and paste ready window to access any protected process memory, you can use that. You'll probably end in making a less detected cheat then its one, using the same stuff he pasted. 

# Instructions

Open a command prompt as administrator and run the command:
** "easy-rw.exe <process id> **

If it success, it will write to the console the first 2 bytes of the image of the process you specified (4D 5A)

# Explaination

There's just one thing to explain which isn't wrote in the PDF:

    typedef void(*initialize_process_pid_prototype)(HANDLE pid);
    initialize_process_pid_prototype initialize_process_pid_function = 
		(initialize_process_pid_prototype)GetProcAddress(LoadLibraryW(L"gdi32.dll"), "D3DKMTVailDisconnect");
    initialize_process_pid_function(process);

The user-mode module the external cheat process uses hooks also some APIs that the host process must call with custom parameters in order to make the User Mode - Kernel Mode communication working.
In order to perform memory operations, you have to set a global variable present in the DLL that describe the process handle the process should use the driver to read or write memory.
The cheat does it by calling gdi32!D3DKMTVailDisconnect with the target process handle as parameter.
If you don't do that, the usermode DLL DLLVMhk wont issue IOCTLs to the driver.
