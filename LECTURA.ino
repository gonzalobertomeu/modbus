bool leerByte(int& ptr) {
  volatile int valor = 0;
  volatile int parity = 0;

  for ( int index = 1; index <= 128; index = index * 2 ) {
    if ( digitalRead(PIN) == ALTA ) {
      valor = valor + index;
      parity++;
    }
    wait_bit();
  }
  ptr = valor;
  parity = ( parity % 2 ) == 0 ? BAJA : ALTA;
  if ( parity == digitalRead(PIN) ) {
    wait_bit();
    return false;
  }
  wait_bit();
  return true;
}


bool procesoLectura( int (&dato)[256], int& index ) {
  modo_escritura(false);
  start();
  while( index < 256 ) {
    if ( !leerByte(dato[index]) ) {
      // Error al leer el byte
      return false;
    }
    index++;
    switch( proximo_bit() ) {
      case 0:
        break;
      case 1: // Trama finalizada
        for ( int i = 0; i < index; i++ ) {
           Serial.print("Byte nro [");
           Serial.print(i);
           Serial.print("] : ");
           Serial.println(dato[i],HEX);
        }
      case -1: // Error en la trama
        return false;
      default: 
        return false;
    }
  }
}
