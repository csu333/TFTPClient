#include <WiFi.h>
#include "TFTPClient.h"

// WiFi credentials
const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";

// TFTP server configuration
IPAddress tftpServer(192, 168, 1, 100);  // Replace with your TFTP server IP

// TFTP server configuration
IPAddress tftpServer(192, 168, 0, 40);  // Replace with your TFTP server IP

TFTPClient tftp;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("TFTP Upload Examples");
    
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
    
    // Example 1: Active upload management
    Serial.println("\n=== Example 1: Active Upload Management ===");
    activeUploadExample();
    
    // Example 2: Streaming upload 
    Serial.println("\n=== Example 2: Streaming Upload ===");
    streamingUploadExample();
    
}

// Example 1: Active upload management
void activeUploadExample() {
    const char* filename = "active_upload_test.txt";
    
    if (!tftp.beginUpload(tftpServer, filename)) {
        Serial.print("Failed to start upload: ");
        Serial.println(tftp.getLastErrorMessage());
        return;
    }
    
    Serial.println("Upload started successfully");
        
    uint8_t buffer[TFTP_DATA_SIZE];

    // Random filling. Put your data here in blocks of 512 bytes
    for (int i = 0; i < sizeof(buffer); i++) {
        buffer[i] = (uint8_t) random();
    }
    
    for (int i = 0; i < 5; i++) {
        
        int result = tftp.writeBlock(buffer, TFTP_DATA_SIZE);
        
        if (result > 0) {
            Serial.println("OK");
        } else {
            Serial.println("FAILED");
            Serial.println(tftp.getLastErrorMessage());
            break;
        }
    }
    
    // Send final empty block to complete upload. Any block smaller than 512 works as well
    tftp.writeBlock(nullptr, 0);
    
    Serial.println("Data upload completed!");
    Serial.print("Total bytes sent: ");
    Serial.println(tftp.getTotalBytesSent());
    
    tftp.stop();
}

// Example 2: Streaming upload 
void streamingUploadExample() {
    // Simulate uploading a large file by generating data on-the-fly
    const char* filename = "generated_data.csv";
    
    if (!tftp.beginUpload(tftpServer, filename)) {
        Serial.print("Failed to start upload: ");
        Serial.println(tftp.getLastErrorMessage());
        return;
    }
    
    Serial.println("Starting streaming upload of generated CSV data...");
    
    // Generate CSV header
    String csvData = "timestamp,temperature,humidity,pressure\n";
    
    int result = tftp.write((const uint8_t*)csvData.c_str(), csvData.length());
    if (result < 0) {
        Serial.println("Failed to upload header");
        tftp.endUpload();
        return;
    }
    
    // Generate and upload data rows
    unsigned long baseTime = millis();
    int rowCount = 0;
    
    for (int batch = 0; batch < 5; batch++) {  // 5 batches of data
        String batchData = "";
        
        // Generate multiple rows per batch
        for (int row = 0; row < 100; row++) {
            rowCount++;
            unsigned long timestamp = baseTime + (rowCount * 1000);
            float temperature = 20.0 + random(-50, 150) / 10.0;  // 15-35°C
            float humidity = 50.0 + random(-200, 200) / 10.0;    // 30-70%
            float pressure = 1013.25 + random(-100, 100) / 10.0; // ±10 hPa
            
            batchData += String(timestamp) + ",";
            batchData += String(temperature, 1) + ",";
            batchData += String(humidity, 1) + ",";
            batchData += String(pressure, 2) + "\n";
        }
        
        // Upload this batch
        Serial.print("Uploading batch ");
        Serial.print(batch + 1);
        Serial.print(" (");
        Serial.print(batchData.length());
        Serial.print(" bytes, ");
        Serial.print(rowCount);
        Serial.print(" rows)... ");
        
        result = tftp.write((const uint8_t*)batchData.c_str(), batchData.length());
        
        if (result > 0) {
            Serial.println("OK");
        } else {
            Serial.println("FAILED");
            Serial.println(tftp.getLastErrorMessage());
            break;
        }
        
        delay(100);  // Simulate processing time
    }
    
    // Send final block
    tftp.flush();
    
    Serial.println("Streaming upload completed!");
    Serial.print("Total rows: ");
    Serial.println(rowCount);
    Serial.print("Total bytes sent: ");
    Serial.println(tftp.getTotalBytesSent());
    
    tftp.endUpload();
}

void loop() {

}