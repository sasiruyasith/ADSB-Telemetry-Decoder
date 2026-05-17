#include <LiquidCrystal.h>

// Initialize the LCD 
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  lcd.begin(16, 2);
  
  // Show a startup message
  lcd.print("Waiting for");
  lcd.setCursor(0, 1);
  lcd.print("ADS-B Data...");
  
  // Start listening to the USB cable at 9600 baud
  Serial.begin(9600);
  
  // Wait a moment for the Serial connection to stabilize
  delay(1000); 
}

void loop() {
  
  if (Serial.available() > 0) {
    
    // Read the incoming text until it sees a "Newline" 
    String incomingFlightData = Serial.readStringUntil('\n');
    
    // Clear the old message off the screen
    lcd.clear();
    
    // Find where the comma is to split the data into two lines
    int commaIndex = incomingFlightData.indexOf(',');
    
    if (commaIndex != -1) {
      // If there is a comma, split the text
      String row1 = incomingFlightData.substring(0, commaIndex);
      String row2 = incomingFlightData.substring(commaIndex + 1);
      
      // Print to the top row
      lcd.setCursor(0, 0);
      lcd.print(row1);
      
      // Print to the bottom row
      lcd.setCursor(0, 1);
      lcd.print(row2);
    } else {
      // If there is no comma, just print it all on the top row
      lcd.setCursor(0, 0);
      lcd.print(incomingFlightData);
    }
  }
}