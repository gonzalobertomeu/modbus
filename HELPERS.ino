/*
 * Check de polaridad (para saber si se conecto al reves el USB
 */
void polarity() {
  unsigned long inicio = micros();
  int bit_check = 0
  while( (micros()-inicio) < bit_time * 100 ) {
    int readed = (digitalRead(PIN) == HIGH ? 1 : 0);
    bit_check = bit_check + readed;
  }
  if( bit_check < 15 ) {
    BAJA = LOW;
    ALTA = HIGH;
  }
  if ( bit_check > 85 ) {
    BAJA = HIGH;
    ALTA = LOW;
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
        state = -1 //Proximo bit excedio tiempo
      } else {
        state = 0
      }
      wait_bit();
      break;
    }
  }
  return state;
}

void modo_escritura( bool flag = true ) {
  pinMode(PIN, flag ? OUTPUT : INPUT);
  digitalWrite(TXRX, flag ? HIGH : LOW);
  if(flag){
    digitalWrite(PIN,BAJA);
  }
}
