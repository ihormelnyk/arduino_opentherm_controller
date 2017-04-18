const int OT_IN_PIN = 19; //3 //Arduino UNO
const int OT_OUT_PIN = 18; //2 //Arduino UNO
const unsigned int bitPeriod = 1000; //1020 //microseconds, 1ms -10%+15%

//P MGS-TYPE SPARE DATA-ID  DATA-VALUE
//0 000      0000  00000000 00000000 00000000
unsigned long requests[] = {
  0x300, //0 get status
  0x90014000, //1 set CH temp //64C
  0x80190000, //25 Boiler water temperature
};

void setIdleState() {
  digitalWrite(OT_OUT_PIN, HIGH);
}

void setActiveState() {
  digitalWrite(OT_OUT_PIN, LOW);
}

void activateBoiler() {
  setIdleState();
  delay(1000);
}

void sendBit(bool high) {
  if (high) setActiveState(); else setIdleState();
  delayMicroseconds(500);
  if (high) setIdleState(); else setActiveState();
  delayMicroseconds(500);
}

void sendFrame(unsigned long request) {
  sendBit(HIGH); //start bit
  for (int i = 31; i >= 0; i--) {
    sendBit(bitRead(request, i));
  }
  sendBit(HIGH); //stop bit  
  setIdleState();
}

void printBinary(unsigned long val) {
  for (int i = 31; i >= 0; i--) {
    Serial.print(bitRead(val, i));
  }
}

unsigned long sendRequest(unsigned long request) {
  Serial.println();
  Serial.print("Request:  ");
  printBinary(request);
  Serial.print(" / ");
  Serial.print(request, HEX);
  Serial.println();
  sendFrame(request);

  if (!waitForResponse()) return 0;

  return readResponse();
}

bool waitForResponse() {
  unsigned long time_stamp = micros();
  while (digitalRead(OT_IN_PIN) != HIGH) { //start bit
    if (micros() - time_stamp >= 1000000) {
      Serial.println("Response timeout");
      return false;
    }
  }
  delayMicroseconds(bitPeriod * 1.25); //wait for first bit
  return true;
}

unsigned long readResponse() {
  unsigned long response = 0;
  for (int i = 0; i < 32; i++) {
    response = (response << 1) | digitalRead(OT_IN_PIN);
    delayMicroseconds(bitPeriod);
  }
  Serial.print("Response: ");
  printBinary(response);
  Serial.print(" / ");
  Serial.print(response, HEX);
  Serial.println();

  if ((response >> 16 & 0xFF) == 25) {
    Serial.print("t=");
    Serial.print(response >> 8 & 0xFF);
    Serial.println("");
  }
  return response;
}

void setup() {
  pinMode(OT_IN_PIN, INPUT);
  pinMode(OT_OUT_PIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("Start");
  activateBoiler();
}

void loop() {
  for (int index = 0; index < (sizeof(requests) / sizeof(unsigned long)); index++) {
    sendRequest(requests[index]);
    delay(950);
  }
}
