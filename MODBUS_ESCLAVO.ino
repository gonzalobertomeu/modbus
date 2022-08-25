int baudios = 9600;
int bit_time = 104;
int char_time = bit_time * 11;
int slave;

int PIN = 3;
int TXRX = 11;

int ALTA = HIGH;
int BAJA = LOW;

void setup() {
  // put your setup code here, to run once:
  slave = 1;
  pinMode(TXRX,OUTPUT);
  modo_escritura(false);
  Serial.begin(baudios);
  polarity();
}

void loop() { 
  // put your main code here, to run repeatedly:
  int dato[256];
  int longitud = 0;
  if ( !procesoLectura( dato, longitud ) ) {
    Serial.println("Hubo un error de la lectura de la trama");
  }
}
