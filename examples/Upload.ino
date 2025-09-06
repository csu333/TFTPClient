#include <WiFi.h>
#include "TFTPClient.h"

// WiFi credentials
const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";

// TFTP server configuration
IPAddress tftpServer(192, 168, 1, 100);  // Replace with your TFTP server IP

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
    if (!tftp.begin()) {
        Serial.print("Failed to initialize TFTP client: ");
        Serial.println(tftp.getLastErrorMessage());
        return;
    }
    
    Serial.println("TFTP client initialized");
    
    // Example 1: Simple file upload
    Serial.println("\n=== Example 1: Simple File Upload ===");
    simpleFileUpload();
    
    // Example 2: Active upload management
    Serial.println("\n=== Example 2: Active Upload Management ===");
    activeUploadExample();
    
    // Example 3: Large file upload from SD card
    Serial.println("\n=== Example 3: Streaming Upload ===");
    streamingUploadExample();
    
    // Example 4: Sensor data upload
    Serial.println("\n=== Example 4: Sensor Data Upload ===");
    sensorDataUpload();
}

void simpleFileUpload() {
    // Create some sample data
    String content = "Hello from Arduino!\n";
    content += "Current time: " + String(millis()) + " ms\n";
    content += "Free heap: " + String(ESP.getFreeHeap()) + " bytes\n";
    content += "WiFi RSSI: " + String(WiFi.RSSI()) + " dBm\n";
    
    // Convert to byte array
    const uint8_t* data = (const uint8_t*)content.c_str();
    size_t dataSize = content.length();
    
    Serial.print("Uploading ");
    Serial.print(dataSize);
    Serial.println(" bytes...");
    
    bool success = tftp.uploadFile(tftpServer, "arduino_status.txt", data, dataSize);
    
    if (success) {
        Serial.println("Upload successful!");
    } else {
        Serial.print("Upload failed: ");
        Serial.println(tftp.getLastErrorMessage());
    }
}

void activeUploadExample() {
    const char* filename = "active_upload_test.txt";
    
    if (!tftp.beginUpload(tftpServer, filename)) {
        Serial.print("Failed to start upload: ");
        Serial.println(tftp.getLastErrorMessage());
        return;
    }
    
    Serial.println("Upload started successfully");
    
    // Upload data in chunks
    String chunks[] = {
        "Chunk 1: This is the first part of the file.\n",
        "Chunk 2: Here's some more data to upload.\n",
        "Chunk 3: We can control the upload process precisely.\n",
        "Chunk 4: Each chunk is sent as a separate TFTP block.\n",
        "Chunk 5: This is the final chunk of our test file."
    };
    
    int totalChunks = sizeof(chunks) / sizeof(chunks[0]);
    
    for (int i = 0; i < totalChunks; i++) {
        const uint8_t* chunkData = (const uint8_t*)chunks[i].c_str();
        size_t chunkSize = chunks[i].length();
        
        Serial.print("Uploading chunk ");
        Serial.print(i + 1);
        Serial.print("/");
        Serial.print(totalChunks);
        Serial.print(" (");
        Serial.print(chunkSize);
        Serial.print(" bytes)... ");
        
        int result = tftp.writeBlock(chunkData, chunkSize);
        
        if (result > 0) {
            Serial.println("OK");
        } else {
            Serial.println("FAILED");
            Serial.println(tftp.getLastErrorMessage());
            break;
        }
        
        delay(1000);  // Wait 1 second between readings
    }
    
    // Add footer and complete upload
    String logFooter = "=== End of Sensor Log ===\n";
    logFooter += "Total readings: 10\n";
    logFooter += "Upload completed at: " + String(millis()) + " ms\n";
    
    tftp.writeBlock((const uint8_t*)logFooter.c_str(), logFooter.length());
    
    // Send final empty block to complete upload
    tftp.writeBlock(nullptr, 0);
    
    Serial.println("Sensor data upload completed!");
    Serial.print("Total bytes sent: ");
    Serial.println(tftp.getTotalBytesSent());
    
    tftp.endUpload();
}

void loop() {
    // Example of periodic data uploads
    static unsigned long lastUpload = 0;
    const unsigned long uploadInterval = 30000;  // Upload every 30 seconds
    
    if (millis() - lastUpload > uploadInterval) {
        lastUpload = millis();
        
        Serial.println("\n--- Periodic Status Upload ---");
        
        // Create status report
        String statusReport = "Arduino Status Report\n";
        statusReport += "=====================\n";
        statusReport += "Timestamp: " + String(millis()) + " ms\n";
        statusReport += "Uptime: " + String(millis() / 1000) + " seconds\n";
        statusReport += "Free Heap: " + String(ESP.getFreeHeap()) + " bytes\n";
        statusReport += "WiFi Status: " + String(WiFi.status()) + "\n";
        statusReport += "WiFi RSSI: " + String(WiFi.RSSI()) + " dBm\n";
        statusReport += "Local IP: " + WiFi.localIP().toString() + "\n";
        
        // Add some sensor data
        statusReport += "\nSensor Readings:\n";
        statusReport += "Analog A0: " + String(analogRead(A0)) + "\n";
        statusReport += "Temperature: " + String((analogRead(A0) * 3.3 / 4095.0 - 0.5) * 100, 1) + "°C\n";
        
        statusReport += "\nEnd of Report\n";
        
        // Upload using simple method
        const uint8_t* data = (const uint8_t*)statusReport.c_str();
        size_t dataSize = statusReport.length();
        
        String filename = "status_" + String(millis() / 1000) + ".txt";
        
        if (tftp.uploadFile(tftpServer, filename.c_str(), data, dataSize)) {
            Serial.print("Status uploaded successfully as: ");
            Serial.println(filename);
        } else {
            Serial.print("Status upload failed: ");
            Serial.println(tftp.getLastErrorMessage());
        }
    }
    
    // Example of conditional uploads based on sensor thresholds
    static unsigned long lastThresholdCheck = 0;
    if (millis() - lastThresholdCheck > 5000) {  // Check every 5 seconds
        lastThresholdCheck = millis();
        
        int sensorValue = analogRead(A0);
        
        // Upload alert if sensor value exceeds threshold
        if (sensorValue > 3000) {  // Adjust threshold as needed
            Serial.println("Sensor threshold exceeded! Uploading alert...");
            
            String alertMessage = "SENSOR ALERT\n";
            alertMessage += "============\n";
            alertMessage += "Timestamp: " + String(millis()) + " ms\n";
            alertMessage += "Sensor Value: " + String(sensorValue) + "\n";
            alertMessage += "Threshold: 3000\n";
            alertMessage += "Action Required: Check sensor conditions\n";
            
            String alertFilename = "alert_" + String(millis()) + ".txt";
            
            if (tftp.uploadFile(tftpServer, alertFilename.c_str(), 
                               (const uint8_t*)alertMessage.c_str(), 
                               alertMessage.length())) {
                Serial.println("Alert uploaded successfully!");
            } else {
                Serial.print("Alert upload failed: ");
                Serial.println(tftp.getLastErrorMessage());
            }
        }
    }
    
    delay(1000);
} > 0) {
            Serial.println("OK");
            Serial.print("Block ");
            Serial.print(tftp.getCurrentUploadBlockNumber() - 1);
            Serial.print(", Total sent: ");
            Serial.print(tftp.getTotalBytesSent());
            Serial.println(" bytes");
        } else {
            Serial.println("FAILED");
            Serial.print("Error: ");
            Serial.println(tftp.getLastErrorMessage());
            break;
        }
        
        // Small delay between chunks (optional)
        delay(100);
    }
    
    // Send final empty block to signal end of file
    int result = tftp.writeBlock(nullptr, 0);
    if (result >= 0) {
        Serial.println("Upload completed successfully!");
        Serial.print("Total bytes sent: ");
        Serial.println(tftp.getTotalBytesSent());
    } else {
        Serial.println("Failed to complete upload");
    }
    
    tftp.endUpload();
}

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
    
    int result = tftp.writeBlock((const uint8_t*)csvData.c_str(), csvData.length());
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
        for (int row = 0; row < 10; row++) {
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
        
        result = tftp.writeBlock((const uint8_t*)batchData.c_str(), batchData.length());
        
        if (result > 0) {
            Serial.println("OK");
        } else {
            Serial.println("FAILED");
            Serial.println(tftp.getLastErrorMessage());
            break;
        }
        
        delay(200);  // Simulate processing time
    }
    
    // Send final empty block
    tftp.writeBlock(nullptr, 0);
    
    Serial.println("Streaming upload completed!");
    Serial.print("Total rows: ");
    Serial.println(rowCount);
    Serial.print("Total bytes sent: ");
    Serial.println(tftp.getTotalBytesSent());
    
    tftp.endUpload();
}

void sensorDataUpload() {
    // Simulate real-time sensor data collection and upload
    const char* filename = "sensor_log.txt";
    
    if (!tftp.beginUpload(tftpServer, filename)) {
        Serial.print("Failed to start sensor data upload: ");
        Serial.println(tftp.getLastErrorMessage());
        return;
    }
    
    Serial.println("Starting real-time sensor data upload...");
    
    String logHeader = "=== Arduino Sensor Log ===\n";
    logHeader += "Started at: " + String(millis()) + " ms\n\n";
    
    tftp.writeBlock((const uint8_t*)logHeader.c_str(), logHeader.length());
    
    // Collect and upload sensor readings every second
    for (int reading = 1; reading <= 10; reading++) {
        // Simulate sensor readings
        int analogValue = analogRead(A0);  // Read analog pin
        int digitalValue = digitalRead(2); // Read digital pin
        long uptime = millis();
        
        String logEntry = "Reading " + String(reading) + ":\n";
        logEntry += "  Uptime: " + String(uptime) + " ms\n";
        logEntry += "  Analog A0: " + String(analogValue) + "\n";
        logEntry += "  Digital Pin 2: " + String(digitalValue) + "\n";
        logEntry += "  Free Heap: " + String(ESP.getFreeHeap()) + " bytes\n";
        logEntry += "  WiFi RSSI: " + String(WiFi.RSSI()) + " dBm\n\n";
        
        Serial.print("Uploading sensor reading ");
        Serial.print(reading);
        Serial.print("/10... ");
        
        int result = tftp.writeBlock((const uint8_t*)logEntry.c_str(), logEntry.length());
        
        if (result