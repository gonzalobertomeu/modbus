int baudios = 9600;
int bit_time = 104;
int char_time = bit_time * 11;
int slave_id;

void setup() {
  slave_id = 1;
  pinMode(3, INPUT);
  pinMode(11, OUTPUT);
  Serial.begin(baudios);
  delayMicroseconds(char_time * 4);
}

void wait_bit(int times = 1) {
  delayMicroseconds(bit_time * times);
}

void wait_char(int times = 1) {
  delayMicroseconds(char_time * times);
}

void start() {
  Serial.println("Esperando que llegue bit de inicio");
  while(true){
//    Serial.println(digitalRead(3) == HIGH ? 1 : 0);
    if(digitalRead(3) == LOW) {
      wait_bit();
      return;
    }
  }
}

int proximo_bit() {
  unsigned long inicio = micros();
  /**
   * state=0 : Sigo leyendo bytes
   * state=1 : Termino la trama
   * state=-1 : Error en la trama
   */
  int state = 1;
  //Serial.println("Esperando el proximo byte");
  while( (micros() - inicio) < (char_time * 3.5) ) {
    if ( digitalRead(3) == LOW ) {
      if ( (micros() - inicio) > (char_time * 1.5) ) {
        state = -1; //PROXIMO BIT EXCEDIDO
      } else {
        state = 0; //PROXIMO BIT EN TIEMPO
      }
      wait_bit();
      break;
    }
  }
  return state;
}

boolean bit_stop( int quantity = 2) {
  return true;
}

boolean leerByte(int& ptr) {
  volatile int valor = 0;
  volatile int parity = 0;
  for( int index = 1; index <= 128; index = index * 2 ) {
    if ( digitalRead(3) == HIGH ) {
      valor = valor + index;
      parity++;
    }
    wait_bit();
  }
  ptr = valor;
//  Serial.print("Paridad: ");
//  Serial.println(parity);
  parity = (parity % 2) == 0 ? HIGH : LOW;
  if ( parity == digitalRead(3) ) {
    return false;
  }
  wait_bit();
  return true;
}

void escribirByte(int valor) {
  volatile int cociente = valor;
  digitalWrite(3,LOW); // Bit de inicio
  wait_bit();

  volatile int parity = 0;
  for ( int i = 0; i < 8 ; i++ ) {
    if ( cociente == 0 ) {
      digitalWrite( 3, HIGH );
      wait_bit();
      continue; //evita las divisiones y el chequeo si ya se completo el numero. sirve para rellenar los 8 bits
    }
    volatile int resto = cociente % 2;
    cociente = floor(cociente / 2);
    if ( resto == 1 ) {
      digitalWrite( 3, LOW );  
      parity++;
    } else {
      digitalWrite( 3, HIGH );
    }
    wait_bit();
  }
  parity = (parity % 2) == 0 ? HIGH : LOW;
  digitalWrite(3, parity);
  wait_bit();
  digitalWrite(3,HIGH);
}

boolean procesoLectura(int (&dato)[256], int& index){
  Serial.println("Proceso lectura");
  digitalWrite(11,LOW);
  pinMode(3, INPUT);
  start(); //espero el primer bit de inicio
  while( index < 256) {
    if( ! leerByte(dato[index]) ) {
      Serial.println("Error al leer byte");
      return false;
    }
    //Serial.println(dato[index]);
    index++;
    switch( proximo_bit() ) { //espero el bit de inicio del siguiente byte
      case 0: //sigo leyendo
        // Serial.println("Sigo leyendo");
        break;
      case 1: //trama finalizada
       Serial.println("TRAMA FINALIZADA");
       for ( int i = 0; i < index; i++ ) {
         Serial.print("Byte nro [");
         Serial.print(i);
         Serial.print("] : ");
         Serial.println(dato[i],HEX);
       }
        return true;
      case -1: //error de trama
        Serial.println("Error en la trama");
        return false;
      default: //algun error extraño
        Serial.println("Un error extranio");
        return false;
    }
  }
}

// void responder(int (&dato)[256], int &longitud) {
//   Serial.println("Responde a maestro");
//   pinMode(3, OUTPUT);
//   digitalWrite(11,HIGH);
//   digitalWrite(3, HIGH);
//   wait_char(4);
//   for( int i = 0; i < longitud; i++ ) {
//     escribirByte(dato[i]);
//   }
//   return;
// }

// void responder( int dato[256], int longitud ) {
//   for( int i = 0 ; i < longitud ; i++ ) {
//     Serial.println(dato[i]);
//   }
// }

void loop() {
  int dato[256];
  int longitud = 0;
  if ( ! procesoLectura(dato,longitud) ) {
    Serial.println("Hubo un error en la lectura de la trama");
  }
}
