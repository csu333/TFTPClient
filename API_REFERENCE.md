# TFTPClient API Reference

## Constructor

### `TFTPClient()`
Creates a new TFTP client instance with default settings.

```cpp
TFTPClient tftp;
```

## Initialization

### `bool initialize()`
Initializes the TFTP client and UDP connection. Must be called before any operations.

**Returns:** `true` if successful, `false` on failure

```cpp
TFTPClient tftp;
if (tftp.initialize()) {
    Serial.println("TFTP client ready");
}
```

## Simple File Operations

### `bool downloadFile(IPAddress serverIP, const char* filename, uint8_t* buffer, size_t bufferSize)`
Downloads a complete file into memory buffer. The buffer must be large enough to hold the received file otherwise a Buffer overflow error (-7) will be raised.

**Parameters:**
- `serverIP` - TFTP server IP address
- `filename` - Remote filename to download
- `buffer` - Buffer to store downloaded data
- `bufferSize` - Size of buffer in bytes

**Returns:** `true` if successful, `false` on error

```cpp
uint8_t buffer[1024];
bool success = tftp.downloadFile(IPAddress(192,168,1,100), "config.txt", buffer, sizeof(buffer));
```

### `bool downloadFile(const char* serverHost, const char* filename, uint8_t* buffer, size_t bufferSize)`
Downloads file using hostname instead of IP address.

```cpp
bool success = tftp.downloadFile("tftp.example.com", "data.bin", buffer, sizeof(buffer));
```

## Progress Monitoring

### `bool downloadFileWithProgress(IPAddress serverIP, const char* filename, ProgressCallback callback)`
Downloads file with real-time progress callbacks.

**Parameters:**
- `serverIP` - TFTP server IP address  
- `filename` - Remote filename to download
- `callback` - Function called for each data block received

**Callback Signature:**
```cpp
void callback(uint8_t* data, size_t blockSize);
```

```cpp
void onDataReceived(uint8_t* data, size_t size) {
    Serial.print("Received ");
    Serial.print(size);
    Serial.println(" bytes");
}

tftp.downloadFileWithProgress(serverIP, "largefile.bin", onDataReceived);
```

### `bool downloadFileWithProgress(const char* serverHost, const char* filename, ProgressCallback callback)`
Progress download using hostname.

## Active Download Management

### `bool beginDownload(IPAddress serverIP, const char* filename)`
Starts an active download session for block-by-block processing.

**Returns:** `true` if download request sent successfully

```cpp
if (tftp.beginDownload(IPAddress(192,168,1,100), "data.csv")) {
    // Process blocks individually
}
```

### `bool beginDownload(const char* serverHost, const char* filename)`
Starts active download using hostname.

### `int readBlock(uint8_t* buffer, size_t bufferSize)`
Reads next data block from active download.

**Parameters:**
- `buffer` - Buffer to store block data (must be at least 512 bytes)
- `bufferSize` - Size of buffer

**Returns:**
- **Positive**: Number of bytes read (1-512)
- **0**: Download completed (last block received)
- **Negative**: Error occurred

```cpp
uint8_t blockBuffer[512];
while (tftp.isDownloadActive()) {
    int bytesRead = tftp.readBlock(blockBuffer, sizeof(blockBuffer));
    
    if (bytesRead > 0) {
        processData(blockBuffer, bytesRead);
    } else if (bytesRead == 0) {
        Serial.println("Download complete!");
        break;
    } else {
        Serial.println("Error occurred");
        break;
    }
}
```

### `bool endDownload()`
Ends active download session and cleans up resources.

**Returns:** `true` if there was an active download to end

### `bool isDownloadActive()`
Checks if download session is currently active.

**Returns:** `true` if download is in progress

### `bool isDownloadComplete()`
Checks if download session has completed successfully.

**Returns:** `true` if download finished without errors

### `uint16_t getCurrentBlockNumber()`
Gets current block number being processed (1-based).

### `size_t getTotalBytesReceived()`
Gets total bytes received in current download session.

## Stream Interface

### `int available()`
Returns number of bytes available for reading from download stream.

**Returns:** Number of bytes available or 0 if none

### `int peek()`
Returns next byte without consuming it from download stream.

**Returns:** Next byte value or -1 if no data available

### `int read()`
Reads and consumes next byte from download stream.

**Returns:** Byte value or -1 if no data available

### `size_t readBytes(char* buffer, size_t length)`
Reads specified number of bytes from download stream.

**Parameters:**
- `buffer` - Buffer to store read data
- `length` - Number of bytes to read

**Returns:** Actual number of bytes read

```cpp
char textBuffer[100];
size_t bytesRead = tftp.readBytes(textBuffer, sizeof(textBuffer));
```

### `size_t write(const uint8_t* buffer, size_t size)`
Writes data to active upload stream.

**Parameters:**
- `buffer` - Data to write
- `size` - Number of bytes to write

**Returns:** Number of bytes written

### `size_t write(uint8_t c)`
Writes single byte to active upload stream.

**Parameters:**
- `c` - Byte to write

**Returns:** 1 if successful, 0 if failed

### `void flush()`
Flushes any buffered upload data to the server. Because of how TFTP works, nothing can be written after this call.

```cpp
tftp.write((uint8_t*)"Hello", 5);
tftp.flush();  // Ensure data is sent
```

## Active Upload Management

### `bool beginUpload(IPAddress serverIP, const char* filename)`
Starts an active upload session for streaming data to server.

**Returns:** `true` if upload request accepted by server

```cpp
if (tftp.beginUpload(IPAddress(192,168,1,100), "sensor_data.txt")) {
    // Send data blocks
}
```

### `bool beginUpload(const char* serverHost, const char* filename)`
Starts active upload using hostname.

### `int writeBlock(const uint8_t* data, size_t dataSize)`
Writes data block to active upload.

**Parameters:**
- `data` - Pointer to data to send (use `nullptr` to end upload)
- `dataSize` - Size of data in bytes (use 0 to end upload)

**Returns:**
- **Positive**: Number of bytes successfully sent
- **0**: Block sent successfully (when ending upload)
- **Negative**: Error occurred

```cpp
String data1 = "First block of data\n";
int result = tftp.writeBlock((uint8_t*)data1.c_str(), data1.length());

// End upload with empty block
tftp.writeBlock(nullptr, 0);
```

### `bool endUpload()`
Ends active upload session and cleans up resources.

### `bool isUploadActive()`
Checks if upload session is currently active.

### `uint16_t getCurrentUploadBlockNumber()`
Gets current upload block number (1-based).

### `size_t getTotalBytesSent()`
Gets total bytes sent in current upload session.

## Configuration

### `void setTimeout(unsigned long timeout)`
Sets timeout for TFTP operations in milliseconds.

**Default:** 5000ms

```cpp
tftp.setTimeout(10000);  // 10 second timeout
```

### `void setMaxRetries(int retries)`
Sets maximum number of retries for failed operations.

**Default:** 3 retries

```cpp
tftp.setMaxRetries(5);  // Allow 5 retries
```

## Error Handling

### `int getLastErrorCode()`
Gets the last error code from TFTP operations.

**Returns:**
- **0**: Success
- **Negative**: Library error codes
- **Positive**: TFTP protocol error codes

### `const char* getLastErrorMessage()`
Gets human-readable description of last error.

```cpp
if (!tftp.downloadFile(serverIP, "test.txt", buffer, sizeof(buffer))) {
    Serial.print("Error ");
    Serial.print(tftp.getLastErrorCode());
    Serial.print(": ");
    Serial.println(tftp.getLastErrorMessage());
}
```

## Cleanup

### `void stop()`
Stops TFTP client, ends any active transfers, and releases resources.

```cpp
tftp.stop();  // Clean shutdown
```

## Error Codes Reference

### Library Errors (Negative Values)
- `-1` - UDP initialization failed
- `-2` - Failed to resolve hostname  
- `-3` - Invalid buffer parameters
- `-4` - Failed to send request
- `-5` - Timeout waiting for response
- `-6` - Invalid packet size
- `-7` - Unexpected packet format
- `-8` - Buffer overflow
- `-9` - Memory allocation failed
- `-10` - Download already active
- `-11` - No active download
- `-12` - Packet from wrong IP
- `-13` - Packet from wrong port
- `-14` - Wrong block number
- `-15` - Unexpected packet type
- `-16` - Upload already active
- `-17` - Cannot start upload during download
- `-18` - Server error response
- `-19` - Upload start failed after retries
- `-20` - No active upload
- `-21` - Upload already completed
- `-22` - Failed to send data packet
- `-23` - Server error during upload
- `-24` - Timeout waiting for ACK

### TFTP Protocol Errors (Positive Values)
- `1` - File not found
- `2` - Access violation
- `3` - Disk full or allocation exceeded
- `4` - Illegal TFTP operation
- `5` - Unknown transfer ID
- `6` - File already exists
- `7` - No such user

## Usage Patterns

### Memory-Efficient Large File Download
```cpp
tftp.beginDownload(serverIP, "firmware.bin");
File file = SD.open("/update.bin", FILE_WRITE);

uint8_t buffer[512];
while (tftp.isDownloadActive()) {
    int bytes = tftp.readBlock(buffer, sizeof(buffer));
    if (bytes > 0) {
        file.write(buffer, bytes);
    } else if (bytes == 0) {
        break;  // Complete
    }
}
file.close();
tftp.endDownload();
```

### Streaming Data Upload
```cpp
tftp.beginUpload(serverIP, "sensor_log.csv");

String header = "timestamp,temp,humidity\n";
tftp.writeBlock((uint8_t*)header.c_str(), header.length());

for (int i = 0; i < 100; i++) {
    String row = String(millis()) + "," + String(readTemp()) + "," + String(readHumidity()) + "\n";
    tftp.writeBlock((uint8_t*)row.c_str(), row.length());
}

tftp.writeBlock(nullptr, 0);  // End upload
tftp.endUpload();
```