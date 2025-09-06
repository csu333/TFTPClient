#include <WiFi.h>
#include "TFTPClient.h"

// WiFi credentials
const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";

// TFTP server configuration
IPAddress tftpServer(192, 168, 1, 100);  // Replace with your TFTP server IP

const char* filenameLarge = "largefile.bin";   // File to download
const char* filenamesmall = "smallfile.txt";   // File to download

TFTPClient tftp;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("TFTP Active Download Example");
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ");
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println();
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());

    // Initialize TFTP client
    if (!tftp.initialize()) {
        Serial.print("Failed to initialize TFTP client: ");
        Serial.println(tftp.getLastErrorMessage());
        return;
    }

    // Example 1: Small file download
    Serial.println("\n=== Download Small File ===");
    fileDownload();

    // Example 2: Basic passive download
    Serial.println("\n=== Passively Download File ===");
    passiveDownload();
    
    // Example 3: Stream download
    Serial.println("\n=== Stream Download ===");
    streamDownload();
    
    // Example 4: Basic active download
    Serial.println("\n=== Active Download ===");
    activeDownload();
}

// Example 1: Small file download
void fileDownload() {

    char buffer[4096];

    if (!tftp.downloadFile(tftpServer, filenamesmall, (uint8_t *)buffer, sizeof(buffer))) {
        Serial.print("Failed to download file: ");
        Serial.println(tftp.getLastErrorMessage());
        return;
    }

    Serial.print("Total blocks received: ");
    Serial.println(tftp.getCurrentBlockNumber() - 1);
    Serial.print("Total bytes: ");
    Serial.println(tftp.getTotalBytesReceived());

    // Clean up
    tftp.stop();
}

// Example 2: Basic passive download
void passiveDownload() {
    // Start the download
    if (!tftp.beginDownload(tftpServer, filenameLarge)) {
        Serial.print("Failed to start download: ");
        Serial.println(tftp.getLastErrorMessage());
        return;
    }

    tftp.downloadFileWithProgress(tftpServer, filenameLarge, processData);

    Serial.print("Total blocks received: ");
    Serial.println(tftp.getCurrentBlockNumber() - 1);
    Serial.print("Total bytes: ");
    Serial.println(tftp.getTotalBytesReceived());

    // Clean up
    tftp.stop();
}

void processData(uint8_t* buffer, size_t bytesReceived) {
    Serial.print("Block ");
    Serial.print(tftp.getCurrentBlockNumber() - 1);
    Serial.print(": ");
    Serial.print(bytesReceived);
    Serial.println(" bytes");
    
    // Process the data block here
    // For example, write to SD card, process content, etc.
}

// Example 3: Stream download
void streamDownload() {
    // Start the download
    if (!tftp.beginDownload(tftpServer, filenameLarge)) {
        Serial.print("Failed to start download: ");
        Serial.println(tftp.getLastErrorMessage());
        return;
    }
    
    Serial.println("Download started successfully");
    
    char blockBuffer[100];  // Buffer
    int totalBlocks = 0;
    
    // Read blocks until download is complete
    while (tftp.isDownloadActive()) {
        size_t bytesRead = tftp.readBytes(blockBuffer, sizeof(blockBuffer));
        
        if (bytesRead > 0) {
            totalBlocks++;
            // Notice how the block number is not aligned
            Serial.print("Block ");
            Serial.print(tftp.getCurrentBlockNumber() - 1);
            Serial.print(": ");
            Serial.print(bytesRead);
            Serial.println(" bytes");
            Serial.print(blockBuffer); 
            
            // Process the data block here
            // For example, write to SD card, process content, etc.
            
        } else if (bytesRead == 0) {
            // Download completed
            Serial.println("Download completed!");
            break;
        } else {
            // Error occurred
            Serial.print("Error reading block: ");
            Serial.println(tftp.getLastErrorMessage());
            break;
        }
    }
    
    Serial.print("Total blocks received: ");
    Serial.println(totalBlocks);
    Serial.print("Total bytes: ");
    Serial.println(tftp.getTotalBytesReceived());
    
    // Clean up
    tftp.stop();
}

// Example 4: Basic active download
void activeDownload() {
    // Start the download
    if (!tftp.beginDownload(tftpServer, filenameLarge)) {
        Serial.print("Failed to start download: ");
        Serial.println(tftp.getLastErrorMessage());
        return;
    }
    
    Serial.println("Download started successfully");
    
    uint8_t blockBuffer[TFTP_DATA_SIZE];  // Buffer for one TFTP block
    int totalBlocks = 0;
    
    // Read blocks until download is complete
    while (tftp.isDownloadActive()) {
        int bytesRead = tftp.readBlock(blockBuffer, sizeof(blockBuffer));
        
        if (bytesRead > 0) {
            totalBlocks++;
            Serial.print("Block ");
            Serial.print(tftp.getCurrentBlockNumber() - 1);
            Serial.print(": ");
            Serial.print(bytesRead);
            Serial.println(" bytes");
            
            // Process the data block here
            // For example, write to SD card, process content, etc.
            
        } else if (bytesRead == 0) {
            // Download completed
            Serial.println("Download completed!");
            break;
        } else {
            // Error occurred
            Serial.print("Error reading block: ");
            Serial.println(tftp.getLastErrorMessage());
            break;
        }
    }
    
    Serial.print("Total blocks received: ");
    Serial.println(totalBlocks);
    Serial.print("Total bytes: ");
    Serial.println(tftp.getTotalBytesReceived());
    
    // Clean up
    tftp.stop();
}

void loop() {

}