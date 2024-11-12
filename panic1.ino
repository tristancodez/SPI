#include <WiFi.h>
#include <HTTPClient.h>

// Wi-Fi credentials
const char* ssid = "Tristan";
const char* password = "tristan20";

// Twilio credentials
const String account_sid = "Enter your sid";
const String auth_token = "API key";
const String from_number = "Sender number";
const String to_number = "Reciever number";  // Replace with the recipient's phone number

// GPIO for panic button
const int panicButtonPin = 2;

// API endpoint for IP-based geolocation
const char* geoAPI = "http://ip-api.com/json/";

void setup() {
  Serial.begin(115200);
  pinMode(panicButtonPin, INPUT_PULLUP);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  
  // Check if panic button is pressed
  if (digitalRead(panicButtonPin) == LOW) {
    String location = getLocation();  // Get location from IP
    sendAlertSMS(location);           // Send SMS with location
    delay(5000);  // Add delay to prevent multiple SMS sends due to button debounce
  }
}

String getLocation() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(geoAPI);

    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {  // Check for successful response
      String payload = http.getString();
      Serial.println("Location data retrieved:");
      Serial.println(payload);
      http.end();
      return payload;  // Return location data as JSON string
    } else {
      Serial.print("Error retrieving location: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi not connected.");
  }
  return "";
}

void sendAlertSMS(String location) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://api.twilio.com/2010-04-01/Accounts/" + account_sid + "/Messages.json";
    
    http.begin(url);
    http.setAuthorization(account_sid.c_str(), auth_token.c_str());

    // Format SMS with panic message and location
    String messageBody = "Panic button activated! Help needed! Location: " + location;
    String postData = "To=" + to_number + "&From=" + from_number + "&Body=" + messageBody;

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("SMS sent successfully!");
      Serial.println(response);
    } else {
      Serial.print("Error sending SMS: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi not connected.");
  }
}
