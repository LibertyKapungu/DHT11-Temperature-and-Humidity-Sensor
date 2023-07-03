#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);

//DHT11 values from the Data sheet
#define DHT_DATA_PIN 2
#define LOW_RESPONSE_SIGNAL 54
#define HIGH_RESPONSE_SIGNAL 80

//Button 
#define BUTTON_PIN 3
volatile uint8_t buttonState = HIGH;
volatile uint8_t buttonPressed = HIGH;
volatile unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 140;  
bool isPressed = false;

//RGB LED Module and error LED
#define RED_PIN 9
#define GREEN_PIN 10
#define BLUE_PIN 11
#define ERROR_LED 7

//Humidity and temperature conditions
#define favourable 1
#define unfavourable 0
#define dangerous 2
#define inconclusive 3

//Humidity variables
int hum_integerbits = 0;
int hum_fractionalbits = 0;

//Temperature variables
int temp_integerbits = 0;
int temp_fractionalbits = 0;

int checksum = 0;
bool DHT11_READY = false;

byte degreeSymbol[] = {

  B00100,
  B01010,
  B00100,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

byte plusMinus[] = { B00000, B00100, B01110, B00100, B00000, B01110, B00000, B00000 };

//Functions
void HandlError();
void ButtonPress();
void SendStartPulse();
void CheckResponse();
void IndicateConditions(int conditions);
int readData();
void processData();
void DisplayOnLCD(float humidity, float temperature);


void setup(){

  Serial.begin(9600);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonPress, LOW);

  //Initialise RGB LED Module and error LED
  pinMode(ERROR_LED, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  //Initialise the LCD to display READY
  lcd.init();
  lcd.clear();
  lcd.backlight();

  lcd.createChar(0, degreeSymbol);
  lcd.createChar(1, plusMinus);
  lcd.setCursor(5,0);
  lcd.print("READY");

}


void loop() {

  if(isPressed){

    isPressed = false;

    lcd.clear();
    lcd.setCursor(5,0);
    lcd.print("STANDBY");

    Serial.println("Standby for readings");

    delay(1000); //Delay to let the sensor stabilize before the next reading

    SendStartPulse();

    CheckResponse();

    if(DHT11_READY){

      processData();

      float humidity = hum_integerbits + (hum_fractionalbits/10.00);
      float temperature = temp_integerbits + (temp_fractionalbits/10.00);
      
      Serial.print("Humidity: ");
      Serial.print(humidity, 2);
      Serial.print(" %\t");
      Serial.print("Temperature: ");
      Serial.print(temperature, 2);
      Serial.println(" °C ");

      DisplayOnLCD(humidity, temperature);
    }
  }
}


void HandlError(){

  DHT11_READY = false;

  Serial.println("Critical Error");
  IndicateConditions(inconclusive);
  digitalWrite(ERROR_LED, HIGH);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Error: DHT11");
}


void buttonPress() {

  //Debounce the reset Button
  if ((millis() - lastDebounceTime) < debounceDelay) {
    return;
  }

  lastDebounceTime = millis();

  uint8_t currentState = digitalRead(BUTTON_PIN);

  buttonState = currentState;

  if (buttonState == LOW){
    isPressed = true;
  }else {
    isPressed = false;
  }
}


//Send the starting pulse to the DHT11 
void SendStartPulse(){

  pinMode(DHT_DATA_PIN, OUTPUT);
  digitalWrite(DHT_DATA_PIN, LOW);

  delay(18);

  digitalWrite(DHT_DATA_PIN, HIGH);
  delayMicroseconds(30);
  Serial.println("Start pulse sent");
}


//Checks for the response signal from the DHT11
void CheckResponse(){

  pinMode(DHT_DATA_PIN, INPUT_PULLUP);
  DHT11_READY = false;

  if(digitalRead(DHT_DATA_PIN) == LOW){

    unsigned long responseLowStart = micros();

    while(digitalRead(DHT_DATA_PIN) == LOW){

      if(micros() - responseLowStart > LOW_RESPONSE_SIGNAL){ //Signal timeout
        HandlError();
        return;
      }
    }
  }

  if(digitalRead(DHT_DATA_PIN) == HIGH){
    unsigned long responseHighStart = micros();

    while(digitalRead(DHT_DATA_PIN) == HIGH){
      if(micros() - responseHighStart > HIGH_RESPONSE_SIGNAL){ //Signal timeout
        HandlError();
        return;
      }
    }
  }

  DHT11_READY = true;

  if( (digitalRead(ERROR_LED) == HIGH) ){

    digitalWrite(ERROR_LED, LOW);
  }
}


//Logic for the colour to be displayed by the RGB LED
void IndicateConditions(int conditions){

  if(conditions == favourable){
    analogWrite(RED_PIN, 0);
    analogWrite(GREEN_PIN, 255);
    analogWrite(BLUE_PIN, 0);
  }

  if(conditions == unfavourable){
    analogWrite(RED_PIN, 0);
    analogWrite(GREEN_PIN, 0);
    analogWrite(BLUE_PIN, 255);
  }

  if(conditions == dangerous){
    analogWrite(RED_PIN, 255);
    analogWrite(GREEN_PIN, 0);
    analogWrite(BLUE_PIN, 0);
  }

  if(conditions == inconclusive){
    analogWrite(RED_PIN, 0);
    analogWrite(GREEN_PIN, 0);
    analogWrite(BLUE_PIN, 0);
  }

}


//Transfer the analog signal from DHT11 to a digital signal
int readData(){
  
  int data = 0;

  for(int i = 0; i < 8; i++){

    while(digitalRead(DHT_DATA_PIN) == LOW){
      //Wait till signal goes high
    }

    delayMicroseconds(40);

    if(digitalRead(DHT_DATA_PIN) == HIGH){
      data = (data << 1) | (0x01);            // 1
    }

    else if(digitalRead(DHT_DATA_PIN) == LOW){
      data = (data << 1);                     // 0
    }

    while(digitalRead(DHT_DATA_PIN) == HIGH){
      //Wait till signal goes low
    }
  }

  return data;
}


//Compare the checksum and pick the colour of the LED
void processData(){

  hum_integerbits = readData();
  hum_fractionalbits = readData();
  
  temp_integerbits = readData();
  temp_fractionalbits = readData();
  
  checksum = readData();

  if((temp_integerbits + temp_fractionalbits + hum_integerbits + hum_fractionalbits) != checksum){
    HandlError();
  }

  if(temp_integerbits < 32){
    IndicateConditions(favourable);
  }

  else if((temp_integerbits > 32 && temp_integerbits < 40)){
    IndicateConditions(unfavourable);
  }

  else if( temp_integerbits > 40 ){
    IndicateConditions(dangerous);
  }

}


//Displays the measurments with the relative errors
void DisplayOnLCD(float humidity, float temperature){
  
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("HUM: ");
  lcd.setCursor(6,0);
  lcd.print(humidity,1);
  lcd.setCursor(11,0);
  lcd.write(1);
  lcd.setCursor(13,0);
  lcd.print("5");
  lcd.print("%");

  lcd.setCursor(0,1);
  lcd.print("TEMP: ");
  lcd.setCursor(6,1);
  lcd.print(temperature, 1);
  lcd.setCursor(11,1);
  lcd.write(1);
  lcd.setCursor(13,1);
  lcd.print("2");
  lcd.write(0);
  lcd.print("C");
}
