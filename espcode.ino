#include <WiFi.h>
#include <WebServer.h>

// WiFi credentials
const char *ssid = "DOBI_G9";
const char *password = "12345678";

// WebServer instance
WebServer server(80);

// Global variables
int weight = 0;
float price = 0.0;
float balance = 0.0;

// Serial2 configurations
#define RXD2 16
#define TXD2 17

void setup() {
  // Initialize Serial2
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  
  // Initialize Serial for debugging
  Serial.begin(115200);

  // Initialize WiFi in AP mode
  WiFi.softAP(ssid, password);
  Serial.println("WiFi AP started!");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/topup", handleTopup);
  server.on("/pay", handlePay);
  server.begin();
}

void loop() {
  // Handle web server
  server.handleClient();
   if (Serial2.available()) {
    String incomingData = Serial2.readString(); // Read the entire incoming string
    Serial.println("Received from STM: " + incomingData); // Print to Serial Monitor
  }
  // Check for weight data from Serial2
  if (Serial2.available()) {
    weight = Serial2.parseInt(); // Parse incoming weight
    Serial.println("Weight Received from STM " + weight);
    price = weight / 1000.0; // Calculate price at RM1 per 1000g
  }
}

// Handle root page
void handleRoot() {
  String page = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>Dobi G9 - Laundry System</title>
      <style>
        body {
          font-family: Arial, sans-serif;
          margin: 0;
          padding: 0;
          background-color: #121212;
          color: #ffffff;
          display: flex;
          flex-direction: column;
          align-items: center;
          justify-content: center;
          height: 100vh;
        }
        .container {
          max-width: 400px;
          width: 100%;
          background: #1f1f1f;
          box-shadow: 0 4px 6px rgba(0, 0, 0, 0.3);
          border-radius: 8px;
          padding: 20px;
          text-align: center;
        }
        h1 {
          font-size: 24px;
          margin-bottom: 20px;
          color: #00FF7F;
        }
        p {
          font-size: 18px;
          margin: 10px 0;
        }
        .button {
          display: block;
          width: 100%;
          padding: 10px;
          margin: 10px 0;
          background-color: #007bff;
          color: white;
          text-decoration: none;
          font-size: 16px;
          border: none;
          border-radius: 4px;
          cursor: pointer;
        }
        .button:hover {
          background-color: #0056b3;
        }
        form input[type="number"] {
          width: 80%;
          padding: 10px;
          margin: 10px 0;
          border: 1px solid #333;
          border-radius: 4px;
          font-size: 16px;
          background-color: #1f1f1f;
          color: white;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h1>Dobi G9 - Laundry System</h1>
        <p><strong>Weight:</strong> <span id="weight">[[WEIGHT]]</span> grams</p>
        <p><strong>Price:</strong> RM <span id="price">[[PRICE]]</span></p>
        <p><strong>Balance:</strong> RM <span id="balance">[[BALANCE]]</span></p>
        <form action="/topup" method="POST">
          <input type="number" name="amount" placeholder="Enter top-up amount (RM)" required>
          <button class="button" type="submit">Top Up</button>
        </form>
        <form action="/pay" method="POST">
          <button class="button" type="submit">Pay Now</button>
        </form>
      </div>
    </body>
    </html>
  )rawliteral";

  // Replace placeholders with actual values
  page.replace("[[WEIGHT]]", String(weight));
  page.replace("[[PRICE]]", String(price, 2));
  page.replace("[[BALANCE]]", String(balance, 2));

  server.send(200, "text/html", page);
}

// Handle top-up
void handleTopup() {
  if (server.hasArg("amount")) {
    float topup = server.arg("amount").toFloat();
    balance += topup; // Add top-up to balance
  }
  server.sendHeader("Location", "/"); // Redirect to root page
  server.send(303);
}

// Handle payment
void handlePay() {
  if (balance >= price) {
    balance -= price; // Deduct the price from the balance
    Serial2.println("PAID"); // Send "PAID" to Serial2
    server.send(200, "text/html", "<h1>Payment Successful</h1><a href='/'>Go Back</a>");
  } else {
    server.send(200, "text/html", "<h1>Insufficient Balance</h1><a href='/'>Top-up and Try Again</a>");
  }
}
