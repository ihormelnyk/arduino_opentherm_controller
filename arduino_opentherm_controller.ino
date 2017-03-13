int OT_IN_PIN = 19;
int OT_OUT_PIN = 18;

byte req_idx = 0;
int state = 0;
unsigned long time_stamp;

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

void sendRequest(unsigned long request) {
  Serial.println();  
  Serial.print("Request:  ");
  printBinary(request);
  Serial.print(" / ");
  Serial.print(request, HEX);
  Serial.println();
  sendFrame(request);
  time_stamp = millis();
  state = 1;  
}

void waitForResponse() {  
  if (digitalRead(OT_IN_PIN) == HIGH) { //start bit
    delayMicroseconds(1250);
    state = 2;      
  } else if (millis() - time_stamp >= 1000) {
    Serial.println("Response timeout");  
    state = 0;
  }  
}

void readResponse() {
  unsigned long response = 0;  
  for (int i = 0; i < 32; i++) {
    response = (response << 1) | digitalRead(OT_IN_PIN);
    delayMicroseconds(1005); //1ms -10%+15%
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
  state = 0;  
}

void setup() {
  pinMode(OT_IN_PIN, INPUT);
  pinMode(OT_OUT_PIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("Start");     
  activateBoiler(); 
}

void loop() {
  switch(state){
    case 0: sendRequest(requests[req_idx]); break;
    case 1: waitForResponse(); break;
    case 2: readResponse(); req_idx++; if (req_idx >=sizeof(requests)/sizeof(unsigned long)) { req_idx = 0; } delay(950); break;
  }
}
