## Multi-threaded File Server

### Objective

Develop a multi-threaded client-server application for remote file
access. The client program interacts with the server to perform various
file operations, including reading, appending, and calculating file
hashes. The server must handle multiple clients concurrently, with
proper synchronization to manage file access.

### Components

1.  **Client Program**:

    -   Connects to the server using TCP/IP.

    -   Sends commands to perform file operations and displays the
        server\'s response.

    -   Commands include opening files for reading or appending, reading
        from or appending to files, closing files, and calculating file
        hashes.

2.  **Server Program**:

    -   Listens for incoming client connections on a specified port.

    -   Handles multiple clients using threads to perform file
        operations concurrently.

    -   Manages file access with synchronization to handle constraints
        such as exclusive access for appending.

### Commands

-   **openRead \<file\>**: Opens a file for reading. Only one file can
    be open for reading at a time per client.

-   **openAppend \<file\>**: Opens a file for appending. Only one file
    can be open for appending at a time per client. If a file is open
    for appending by another client, an error is returned.

-   **read \<n\>**: Reads up to n bytes from the currently opened file
    for reading. The bytes are read from the last position read.

-   **append \<string\>**: Appends a string of bytes to the currently
    opened file for appending. No output is returned for this command.

-   **close \<file\>**: Closes the specified file, making it unavailable
    for further operations until reopened.

-   **getHash \<file\>**: Calculates and returns the MD5 hash of the
    specified file. The file must not be open for appending.

-   **quit**: Ends the client session but does not stop the server.

### Constraints

-   A single client can only open one file for reading or appending at a
    time.

-   Multiple clients can read the same file simultaneously and calculate
    its hash.

-   A file opened for appending by one client cannot be accessed by
    other clients, even for hash calculation.

-   A file can only be opened for appending if no other client has it
    open.

### Running the Programs

-   **Server**:

    -   Command: ./server \<port\>

    -   Starts the server to listen on the specified port and handle
        multiple clients using threads.

-   **Client**:

    -   Command: ./client \<server_address\> \<port\>

    -   Connects to the server and allows user interaction through
        command input.

### Communication and User Interface

-   **Client**:

    -   Displays a \> prompt and sends commands to the server.

    -   Receives and displays responses from the server.

-   **Server**:

    -   Prints received commands and their results.

    -   Manages file access and synchronizes operations across multiple
        threads.

### Implementation Details

-   Commands are case-sensitive.

-   File operations must handle errors such as trying to open a file
    that is already open or attempting to access a file that is open by
    another client.

-   The server must handle synchronization to manage concurrent file
    access properly.

-   Files are assumed to be text files, and the MD5 hash calculation
    code is provided in the assignment materials.


### More Info
For additional details, please refer to the specification document provided as specification.pdf.