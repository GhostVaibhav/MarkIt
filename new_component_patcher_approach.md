Introducing a new way to update the application.

Currently, the way of updating the application is just drop and replace. While good for simple things, this quickly falls apart when we have more than 1 file and/or
updating big files.

In order to counter this, we should make a comprehensive updater framework to handle this situation. Since in the future, it's only going to get big.

Therefore, defining a lot of jargons and/or procedures for the same.

This updater arch will be a separate project in itself. Separate all the files and use it as a subdirectory in the main project.

We've to make and package another component called "patcher", all of it's functionalities are defined later in the document.

While updating the files, we can encounter these three conditions - 
- A new file is created in the new release (+)
- An old file is deleted in the new release (-)
- A file is modified in the new release
    - It can add to the file (*+)
    - It can remove from the file (*-)

The patcher tool will perform the following - 
1. It will take two folders (previous release and latest release)
2. It will compute two files from these folders - "patches.bin" and "instruction.set"
3. The "patches.bin" file will contain all the diff info in binary format (not the full files, just the diff which are required for creating the new files)
4. The "instruction.set" file will contain all the instruction sets for performing a patching process on the last release

Format of "instruction.set" file - 

<file_path>:<operation>{+, -, *+, *-}:<file_start_bytes>:<file_end_bytes>:<from_bytes>:<to_bytes>

The "patches.bin" will just be an amalgamated file which will contain the changes in accordance with these operations.

Note - We have to be very sure that the *- operations for any file are done before *+ to maintain consistency.
Note - We can have the same <file_path> multiple times with multiple <operation>'s.
Note - 

When operation = +, this is the format - 
    <file_path>:<operation>{+, -, *+, *-}:<from_bytes>:<to_bytes>

> The new file will be created from "patches.bin"

When operation = -, this is the format -
    <file_path>:<operation>{+, -, *+, *-}

> The file will be simply deleted

When operation = *+, this is the format - 
    <file_path>:<operation>{+, -, *+, *-}:<file_start_bytes>:<from_bytes>:<to_bytes>

> The file will have bytes from the "patches.bin" file (from_bytes, to_bytes) and will be written to the original file (file_start_bytes)

When operation = *-, this is the format -
    <file_path>:<operation>{+, -, *+, *-}:<file_start_bytes>:<file_end_bytes>

> The file will have it's sector (file_start_bytes, file_end_bytes) removed


The new procedures have to be performed by the "markit_updater" project.

The instructions set will be executed one-by-one in a synchronous manner.

"patcher" should not be included in the final release. It's just a utility to generate the patches binary and the instruction set.

In order to create these patched files, we will be using "https://api.github.com/repos/GhostVaibhav/MarkIt/releases" link to get all the releases (be sure that this
also has pagination so we have to get these accordingly).

Then, we can get the latest release and the latest tag associated with that release.

We can then tell git to go to that tag and make the build folder.

We can generate "build_new" and "install_new" folders which will contain the latest code compilations.
We can then generate "build_old" and "install_old" folders which will contain the last tag code compilations.

For updating the application, it will be done in an incremental manner. We will get the releases, sort it by "published_at" date. Then figure out, our release version
number from that list. After that, we will just download the patches zip from all those releases onwards. At the client end, we will follow something like - 
- patch1.tar.gz
- patch2.tar.gz
...

The patcher will push out releases and upload those at artifacts with naming like "patches-linux-x86_64.tar.gz".
