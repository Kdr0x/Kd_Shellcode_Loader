# Shellcode_Loader (aka "RunSc")
A malware analyst's tool for helping to analyze shellcode!

Name: 		Shellcode Loader

Authors: 	Gary "kd" Contreras, Evan Dygert

Date: 		3/15/2021 (original)

Current version directory: /current_2021-Mar-18/

Initial Thoughts:

Many thanks and kudos to Lenny Zeltser for an outstanding SANS FOR610 course. I just didn't like the "jmp2it" tool, sorry! :)

Summary:

This program is a malware analysis tool intended to make it easy to launch shellcode prior to debugging. Source code 
has been included in the GitHub repository.

Future Updates:

No scheduled updates at this time. I think the changes Evan made to the current release pretty much covered all the necessities. Thanks Evan!

Details:

The Mar 18, 2021 update now features more error detection, ability to easily compile in 32-bit or 64-bit variations, command 
line arguents, a cleaner code format, and a document handle as may be required by the shellcode. Use the "-h" command line 
switch to get the help information!

The tool launches with no arguments from the user; as such there is no help output! Upon launching, it will first check for 
admin (elevated) rights and recommend that the user relaunches the program with such rights if they were not detected.

Next, it retrieves its current directory path and checks for the presence of a file named "shellcode" under this path. If 
the "shellcode" file is found, the tool then opens it and gets the size of the file.

When the file size is obtained, the tool allocates at least one (or more) pages of memory sufficient to hold the payload 
and sets the region's protections to PAGE_EXECUTE_READWRITE. The memory location is printed for the analyst's knowledge.

Once the contents are read from the file into the buffer, a thread is created with the base address of the shellcode as its 
starting address. The location of the start of the buffer is passed in as a parameter, in case the shellcode checks for it.

The shellcode thread is launched in a suspended state, allowing the analyst to easily attach to the process with a 
debugger such as x64dbg (even without breaking upon attachment). The thread ID is printed and the tool waits an "infinite" 
amount of time for the new thread to finish; after all, there is no rush! Grab a cup of coffee if you want to...

The analyst needs only search for the base address of the shellcode, which was printed out by the tool, and set a breakpoint 
on the start of the shellcode; then resume the thread that was also printed out by the tool (or just resume all threads).

To make things even more convenient, I included the Winsock 2 library so that the analyst can immediately set breakpoints on 
those functions without waiting for a call to LoadLibrary.

The command for resuming all threads is, conveniently, "resumeallthreads" in x64dbg.

Enjoy and happy hunting!
