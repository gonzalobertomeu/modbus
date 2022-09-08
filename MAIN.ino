#include <EEPROM.h>
int baudios = 9600;
int bit_time = 104;
int char_time = bit_time * 11;
int slave;

int PIN = 3;
int TXRX = 11;

int ALTA = HIGH;
int BAJA = LOW;

/*
 * Check de polaridad (para saber si se conecto al reves el USB
 */
void polarity() {
  unsigned long inicio = micros();
  int bit_check = 0;
  while( (micros()-inicio) < (bit_time * 100) ) {
    int readed = (digitalRead(PIN) == HIGH ? 1 : 0);
    bit_check = bit_check + readed;
  }
  if( bit_check < 15 ) {
    Serial.println("Polaridad positiva");
    BAJA = LOW;
    ALTA = HIGH;
  }
  if ( bit_check > 85 ) {
    Serial.println("Polaridad negativa");
    BAJA = HIGH;
    ALTA = LOW;
  }
}

void modo_escritura( bool flag = true ) {
  pinMode(PIN, flag ? OUTPUT : INPUT);
  digitalWrite(TXRX, flag ? HIGH : LOW);
  if(flag){
    digitalWrite(PIN,BAJA);
  }
}

/*
 * Espera el tiempo de bit
 */
void wait_bit(int times = 1) {
  delayMicroseconds(bit_time * times);
}

/*
 * Espera el tiempo de caracter
 */
void wait_char(int times = 1) {
  delayMicroseconds(char_time * times);
}

void start() {
  while(true) {
    if(digitalRead(PIN) == ALTA) {
      wait_bit();
      break;
    }
  }
}

/*
 * Return:
 *  1 - termino de trama
 *  -1 - error en la trama
 *  0 - sigo leyendo
 */
int next() {
  unsigned long inicio = micros();
  int state = 1;
  while( (micros()-inicio) < (char_time * 3.5) ) {
    if ( digitalRead(PIN) == ALTA ) {
      if ( (micros() - inicio) > (char_time * 1.5) ) {
        state = -1; //Proximo bit excedio tiempo
      } else {
        state = 0;
      }
      wait_bit();
      break;
    }
  }
  return state;
}

bool leerByte(int& ptr) {
  volatile int valor = 0;
  volatile int parity = 0;

  for ( int index = 1; index <= 128; index = index * 2 ) {
    if ( digitalRead(PIN) == BAJA ) {
      valor = valor + index;
      parity++;
    }
    wait_bit();
  }
  ptr = valor;
  wait_bit();
//  parity = ( parity % 2 ) == 0 ? ALTA : BAJA;
  return true;
}


bool procesoLectura( int (&dato)[256], int& index ) {
  modo_escritura(false);
  start();
  while( index < 256 ) {
    if ( !leerByte(dato[index]) ) {
      // Error al leer el byte
//      Serial.println("Error al leer el byte");
      return false;
    }
    index++;
    volatile int condicion = next();
    switch( condicion ) {
      case 0:
        continue;
      case 1: // Trama finalizada
          return true;
      case -1: // Error en la trama
        Serial.println("Lectura de bit erronea en next()");
        return false;
      default: 
        Serial.println("Default");
        return false;
    }
  }
}


//void escribirByte( int valor ) {
//  modo_escritura(true);
//  volatile int cociente = valor;
//  volatile int parity = 0;
//  Serial.println(valor,HEX);
////  wait_bit(); // Tiene que esperar?
//
//  for( int i = 0; i < 8; i++ ) {
//    if ( cociente == 0 ) {
//       digitalWrite(3, BAJA);
//       wait_bit();
//       continue;
//    }
//    volatile int resto = cociente % 2;
//    cociente = floor( cociente / 2 );
//    if ( resto == 1 ) {
//      digitalWrite(3, ALTA);
//      parity++;
//    } else {
//      digitalWrite(3,BAJA);
//    }
//    wait_bit();
//  }
//  parity = ( parity % 2 ) == 0 ? BAJA : ALTA;
//  digitalWrite(3, parity);
//  wait_bit();
//  digitalWrite(3, BAJA);
//  wait_bit(); 
//}

//void responderTrama( int (&dato)[256], int &longitud ) {
//  wait_char(3);
//  for( int i = 0; i < longitud; i++ ) {
//    escribirByte(dato[i]);
//  }
//}

void ejecutarFuncion(int dato[256], int index) {
  Serial.print("Trama: ");
  Serial.print("[");
  for ( int k = 0; k < index; k++ ) {           
     Serial.print(dato[k],HEX);
     if (k < index-1) {
      Serial.print(", ");
     }
  }
  Serial.println("]");
  
  int funcion = dato[1];
  int ah,al,bh,bl,address,cant,value;
  switch( funcion ) {
    case 1: // Read coils
      break;
    case 2: // Read discrete inputs
      break;
    case 3: // Read holding registers
      Serial.println("Fn: Read holding registers");
      ah = dato[2];
      al = dato[3];
      address = (ah*256)+al;
      Serial.print("Direccion: "); Serial.println(address);
      bh = dato[4];
      bl = dato[5];
      cant = (bh*256)+bl;

      for( int j = 0; j < cant; j ++ ) {
        value = EEPROM.read(address+j);
        Serial.print("Valor: ");Serial.println(value);
      }
      break;
    case 4: // Read input registers
      break;
    case 5: // Write Single Coil
      break;
    case 6: // Write single register
      Serial.println("Fn: Write single register");
      ah = dato[2];
      al = dato[3];
      address = (ah*256)+al;
      bh = dato[4];
      bl = dato[5];
      value = (bh*256)+bl;
      EEPROM.write(address,value);
      Serial.print("Valor guardado: ");Serial.println(value);
      break;
    case 15: // Write multiple coils
      break;
    case 16: //Write multiple registers
      break;
    default:
      return false;
  }
}

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
  ejecutarFuncion(dato,longitud);
}
