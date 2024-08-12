## Client & Server Programming

### Objective

Create a client-server application to query stock market information.
The client will interact with the server to fetch stock prices for
Pfizer (PFE) and Moderna (MRNA) using historical data provided in CSV
files.

### Components

1.  **Client Program**:

    -   Connects to the server using TCP/IP.

    -   Sends commands to the server and displays the response.

    -   Commands include querying stock prices on a specific date and
        calculating maximum possible profit/loss within a time span.

2.  **Server Program**:

    -   Listens for client connections on a specified port.

    -   Reads and parses CSV files containing historical stock data.

    -   Processes client queries and returns the relevant stock
        information or calculated profit/loss.

### Commands

-   **PricesOnDate \<date\>**: Returns the stock prices of both PFE and
    MRNA on the specified date.

-   **MaxPossible \<profit/loss\> \<stock\> \<start_date\>
    \<end_date\>**: Computes the maximum profit or loss for the given
    stock within the specified date range.

-   **quit**: Ends the client program.

### Running the Programs

-   **Server**:

    -   Command: ./server PFE.csv MRNA.csv \<port\>

    -   Reads stock data from CSV files.

    -   Listens for client requests on the specified port.

-   **Client**:

    -   Command: ./client \<server_address\> \<port\>

    -   Sends commands to the server and displays responses.

### Communication Format

-   **Messages**: Each message between the client and server must
    include:

    -   Byte 0: Length of the string (n)

    -   Bytes 1 -- n: Characters of the string

### User Interface

-   **Client**:

    -   Prompts the user with \>.

    -   Handles input commands and displays responses.

-   **Server**:

    -   Prints received commands and responses.

    -   Continues to run until manually terminated.

### Implementation Details

-   Commands are case-sensitive.

-   Dates must be in \"YYYY-MM-DD\" format.

-   Stock symbols must be PFE or MRNA.

-   Handle invalid commands and unknown dates appropriately.

### More Info
For additional details, please refer to the specification document provided as specification.pdf.