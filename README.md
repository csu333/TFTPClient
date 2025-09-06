A complete TFTP (Trivial File Transfer Protocol) client library for Arduino ESP32 and other WiFi-enabled Arduino boards. This library allows you to download and upload files from TFTP servers over WiFi. 

TFTP transfer are ideally suited for OTA of systems that spend a lot of time in deep sleep. It does not require complex http server setup as all you need on Linux is:
```bash
sudo dnsmasq --port=0 --enable-tftp --tftp-root=/path/to/folder --tftp-no-blocksize --user=root --group=root
```

If upload to the server is needed, [tftp-hpa](https://help.ubuntu.com/community/TFTP) is easy to install and configure. Just make sure to give the `--create` parameter.

## Features

- **Simple API** - Easy to use with minimal setup
- **Active Download Management** - Control downloads with begin/read/end methods
- **Active Upload Management** - Control uploads with begin/write/end methods
- **Robust Protocol Implementation** - Full TFTP RFC 1350 compliance
- **Error Handling** - Comprehensive error reporting and recovery
- **Progress Callbacks** - Monitor download progress in real-time
- **Streaming Uploads** - Upload data without storing entire files in memory
- **Timeout & Retry Logic** - Configurable timeouts and automatic retries

## Installation

1. Download the library files (`TFTPClient.h` and `TFTPClient.cpp`)
2. Place them in your Arduino project directory
3. Include the header in your sketch: `#include "TFTPClient.h"`

## Requirements

- ESP32, ESP8266, or other WiFi-enabled Arduino board
- WiFi connection
- TFTP server on your network

## TFTP Protocol Notes

- TFTP uses UDP port 69 for initial connection
- Data transfer uses a random high port assigned by the server
- Files are transferred in 512-byte blocks (except the last block)
- Each block must be acknowledged before the next is sent
- Binary mode ("octet") is used for all transfers
- Maximum theoretical file size is ~33MB (65535 blocks × 512 bytes)

## Usage

Several modes of use are possible. They are demonstrated in the examples:
 - **Small file download**: Applicable for files that hold in memory. The buffer needs to be at least as large than the received file
 - **Passive file download**: Easiest way to use the library to process the data as it is received
 - **Stream download/upload**: Enables to use the standard stream methods
 - **Active download/upload**: If you want to manage the transfer on your own

## Troubleshooting

1. **Connection Issues**
   - Ensure WiFi is connected before calling `begin()`
   - Check if TFTP server is running and accessible
   - Verify firewall settings allow TFTP traffic

2. **Download Failures**
   - Check if the file exists on the server
   - Ensure buffer is large enough for the file
   - Try increasing timeout for slow networks

3. **Memory Issues**
   - Use smaller buffers for large files
   - Process files in chunks if needed
   - Monitor available heap memory
