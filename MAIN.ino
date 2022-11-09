#include <EEPROM.h>
int baudios = 9600;
int bit_time = 104;
int char_time = bit_time * 11;
int slave;

int IN = 3;
int OUT = 4;
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
    int readed = (digitalRead(IN) == HIGH ? 1 : 0);
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

unsigned int crc_calc( unsigned char *buf, unsigned int len )
{
  unsigned int crc = 0xFFFF;
  unsigned int i = 0;
  char bit = 0;

  for( i = 0; i < len - 2; i++ )
  {
    crc ^= buf[i];

    for( bit = 0; bit < 8; bit++ )
    {
      if( crc & 0x0001 )
      {
        crc >>= 1;
        crc ^= 0xA001;
      }
      else
      {
        crc >>= 1;
      }
    }
  }
  unsigned int A = crc / 256;
  unsigned int B = crc % 256;

//  Serial.println(buf[len-1],HEX);
//  Serial.println(buf[len-2],HEX);
  Serial.print("CRC CHECK: ");
  Serial.println((A == buf[len-1] && B == buf[len-2]) ? "OK" : "FAIL");
  
  crc = ( B * 256 ) + A;
  return crc;
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
void wait_char(float times = 1) {
  delayMicroseconds(char_time * times);
}

void start() {
  while(true) {
    if(digitalRead(IN) == ALTA) {
      wait_bit();
      break;
    }
  }
}

int dread(){
  return digitalRead(IN);
}
void dwrite(int value){
  digitalWrite(OUT,value);
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
    if ( dread() == ALTA ) {
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

bool leerByte( unsigned char& ptr) {
  volatile int valor = 0;
  volatile int parity = 0;

  for ( int index = 1; index <= 128; index = index * 2 ) {
    if ( dread() == BAJA ) {
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


bool procesoLectura( unsigned char (&dato)[256], unsigned int& index ) {
  start();
  while( index < 256 ) {
    if ( !leerByte(dato[index]) ) {
      // Error al leer el byte
      //Serial.println("Error al leer el byte");
      return false;
    }
    index++;
    volatile int condicion = next();
    switch( condicion ) {
      case 0:
        continue;
      case 1: // Trama finalizada
           unsigned int crc = crc_calc(dato,index);
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


void escribirByte( int valor ) {
  Serial.println("Escribiendo byte:");
  Serial.println(valor,HEX);
  digitalWrite(TXRX, HIGH);
  volatile int cociente = valor;
  volatile int parity = 0;
//  Serial.println(valor,HEX);
  dwrite(ALTA);  //ORIGINAL EN ALTA
//  Serial.print("-");
  wait_bit(); // bit inicio

  for( int i = 0; i < 8; i++ ) {
    if ( cociente == 0 ) {
       dwrite(BAJA); //ORIGINAL BAJA
//       Serial.print(0);
       wait_bit();
       continue;
    }
    volatile int resto = cociente % 2;
    cociente = floor( cociente / 2 );
    if ( resto == 1 ) {
      dwrite(ALTA);  //ORIGINAL ALTA
//      Serial.print(1);
      parity++;
    } else {
//      Serial.print(0);
      dwrite(BAJA); //ORIGINAL EN BAJA
    }
    wait_char();
  }
//  parity = ( parity % 2 ) == 0 ? BAJA : ALTA;
//  dwrite(parity);
//  Serial.print( parity == ALTA ? 0 : 1);
//  wait_bit();
  dwrite(ALTA);  //ORIGINAL EN ALTA
//  Serial.print("-"); //bit stop
  wait_bit();
//  Serial.print("-"); //bit stop
  wait_bit();
//  Serial.println("");
//  dwrite(BAJA);
//  wait_bit(); 
}

//void responderTrama( int (&dato)[256], int &longitud ) {
//  wait_char(3);
//  for( int i = 0; i < longitud; i++ ) {
//    escribirByte(dato[i]);
//  }
//}

void ejecutarFuncion(unsigned char dato[256], unsigned int index) {
  Serial.print("Trama: ");
  Serial.print("[");
  for ( int k = 0; k < index; k++ ) {           
     Serial.print(dato[k],HEX);
     if (k < index-1) {
      Serial.print(", ");
     }
  }
  Serial.println("]");

  if ( dato[0] != slave ) {
    Serial.println("No es para mi");
    return;
  }
  
  int funcion = dato[1];
  int ah,al,bh,bl,address,cant,value;
  switch( funcion ) {
    case 1: // Read coils
//      for( int z = 0; z < index; z++ ) {
//        escribirByte(dato[z]);
        //wait_char(1); // Tiempo de espera entre bytes??
//      }
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
      ah = dato[2];
      al = dato[3];
      address = (ah*256)+al;

      bh = dato[4];
      bl = dato[5];
      if ( bl == 0x00 && ( bh == 0xFF || bh == 0x00 ) ) {
        EEPROM.write(address, bh == 0xFF ? 1 : 0 );
        Serial.print("Se guardo un: ");Serial.println(bh == 0xFF ? 1 : 0);
        int crch = dato[6];
        int crcl = dato[7];
        unsigned char response[256] = {
          slave,
          funcion,
          ah,al,
          bh,bl,
          crch,
          crcl
        };
        for( int z = 0; z < 8; z++ ) {
          escribirByte(dato[z]); // Tiempo de espera entre bytes??
        }
      } else {
        Serial.println("error en el valor");
      }
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
  pinMode(IN, INPUT);
  pinMode(OUT, OUTPUT);
  Serial.begin(baudios);
  polarity();
}

void loop() { 
  // put your main code here, to run repeatedly:
  unsigned char dato[256];
  unsigned int longitud = 0;
  if ( !procesoLectura( dato, longitud ) ) {
    Serial.println("Hubo un error de la lectura de la trama");
  }
  ejecutarFuncion(dato,longitud);
}
