#include <WiFi.h>
#include "TFTPClient.h"

// WiFi credentials
const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";

// TFTP server configuration
IPAddress tftpServer(192, 168, 1, 100);  // Replace with your TFTP server IP
const char* filename = "largefile.bin";   // File to download

TFTPClient tftp;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("TFTP Active Download Example");
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    
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
    
    Serial.println("TFTP client initialized");
    
    // Example 1: Basic active download
    Serial.println("\n=== Example 1: Basic Active Download ===");
    basicActiveDownload();
    
    // Example 2: Processing data as it arrives
    Serial.println("\n=== Example 2: Processing Data Blocks ===");
    processDataBlocks();
    
    // Example 3: Download with real-time analysis
    Serial.println("\n=== Example 3: Real-time Analysis ===");
    realTimeAnalysis();
}

void basicActiveDownload() {
    // Start the download
    if (!tftp.beginDownload(tftpServer, filename)) {
        Serial.print("Failed to start download: ");
        Serial.println(tftp.getLastErrorMessage());
        return;
    }
    
    Serial.println("Download started successfully");
    
    uint8_t blockBuffer[512];  // Buffer for one TFTP block
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
    
    // Clean up
    tftp.endDownload();
    
    Serial.print("Total blocks received: ");
    Serial.println(totalBlocks);
    Serial.print("Total bytes: ");
    Serial.println(tftp.getTotalBytesReceived());
}

void processDataBlocks() {
    const char* textFile = "config.txt";
    
    if (!tftp.beginDownload(tftpServer, textFile)) {
        Serial.print("Failed to start download: ");
        Serial.println(tftp.getLastErrorMessage());
        return;
    }
    
    Serial.print("Processing file: ");
    Serial.println(textFile);
    
    uint8_t blockBuffer[512];
    String fileContent = "";
    
    while (tftp.isDownloadActive()) {
        int bytesRead = tftp.readBlock(blockBuffer, sizeof(blockBuffer));
        
        if (bytesRead > 0) {
            // Process text content block by block
            for (int i = 0; i < bytesRead; i++) {
                if (blockBuffer[i] >= 32 && blockBuffer[i] <= 126) {
                    fileContent += (char)blockBuffer[i];
                } else if (blockBuffer[i] == '\n') {
                    fileContent += '\n';
                }
            }
            
            Serial.print(".");  // Progress indicator
            
        } else if (bytesRead == 0) {
            Serial.println("\nDownload completed!");
            break;
        } else {
            Serial.print("\nError: ");
            Serial.println(tftp.getLastErrorMessage());
            break;
        }
    }
    
    tftp.endDownload();
    
    // Display the complete file content
    Serial.println("\nFile content:");
    Serial.println("=============");
    Serial.println(fileContent);
}

void realTimeAnalysis() {
    const char* dataFile = "sensor_data.csv";
    
    if (!tftp.beginDownload(tftpServer, dataFile)) {
        Serial.print("Failed to start download: ");
        Serial.println(tftp.getLastErrorMessage());
        return;
    }
    
    Serial.print("Analyzing file in real-time: ");
    Serial.println(dataFile);
    
    uint8_t blockBuffer[512];
    String lineBuffer = "";
    int lineCount = 0;
    float averageValue = 0;
    float totalValue = 0;
    int valueCount = 0;
    
    while (tftp.isDownloadActive()) {
        int bytesRead = tftp.readBlock(blockBuffer, sizeof(blockBuffer));
        
        if (bytesRead > 0) {
            // Process CSV data line by line
            for (int i = 0; i < bytesRead; i++) {
                char c = (char)blockBuffer[i];
                
                if (c == '\n' || c == '\r') {
                    if (lineBuffer.length() > 0) {
                        processCSVLine(lineBuffer, lineCount, totalValue, valueCount);
                        lineBuffer = "";
                        lineCount++;
                    }
                } else {
                    lineBuffer += c;
                }
            }
            
            // Update progress
            if (lineCount % 10 == 0 && lineCount > 0) {
                Serial.print("Processed ");
                Serial.print(lineCount);
                Serial.println(" lines");
            }
            
        } else if (bytesRead == 0) {
            // Process last line if it doesn't end with newline
            if (lineBuffer.length() > 0) {
                processCSVLine(lineBuffer, lineCount, totalValue, valueCount);
                lineCount++;
            }
            Serial.println("Real-time analysis completed!");
            break;
        } else {
            Serial.print("Error: ");
            Serial.println(tftp.getLastErrorMessage());
            break;
        }
    }
    
    tftp.endDownload();
    
    // Display final statistics
    Serial.println("\nAnalysis Results:");
    Serial.print("Total lines processed: ");
    Serial.println(lineCount);
    Serial.print("Total data points: ");
    Serial.println(valueCount);
    if (valueCount > 0) {
        Serial.print("Average value: ");
        Serial.println(totalValue / valueCount, 2);
    }
}

void processCSVLine(const String& line, int lineNumber, float& totalValue, int& valueCount) {
    // Simple CSV processing - assumes numeric data in first column
    int commaIndex = line.indexOf(',');
    if (commaIndex > 0) {
        String valueStr = line.substring(0, commaIndex);
        float value = valueStr.toFloat();
        if (value != 0 || valueStr == "0") {  // Valid number
            totalValue += value;
            valueCount++;
        }
    }
}

void loop() {
    // Example of periodic downloads
    static unsigned long lastDownload = 0;
    const unsigned long downloadInterval = 60000;  // Download every minute
    
    if (millis() - lastDownload > downloadInterval) {
        lastDownload = millis();
        
        Serial.println("\n--- Periodic Download Check ---");
        
        // Quick download to check for updates
        if (tftp.beginDownload(tftpServer, "status.txt")) {
            uint8_t blockBuffer[512];
            String statusContent = "";
            
            while (tftp.isDownloadActive()) {
                int bytesRead = tftp.readBlock(blockBuffer, sizeof(blockBuffer));
                if (bytesRead > 0) {
                    for (int i = 0; i < bytesRead; i++) {
                        statusContent += (char)blockBuffer[i];
                    }
                } else if (bytesRead == 0) {
                    break;
                } else {
                    Serial.println("Download failed");
                    break;
                }
            }
            
            tftp.endDownload();
            
            if (statusContent.length() > 0) {
                Serial.print("Status update: ");
                Serial.println(statusContent);
            }
        }
    }
    
    delay(1000);
}