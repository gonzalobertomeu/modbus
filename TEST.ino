int number = 0;

void setup(){
    Serial.begin(9600);
}

void loop() {
    delay(500);
    if( number < 1000 ) {
        number = 0;
    }
    Serial.println(number);
}