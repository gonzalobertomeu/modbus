void escribirByte( int valor ) {
  modo_escritura(true);
  volatile int cociente = valor;
  volatile int parity = 0;
//  wait_bit(); // Tiene que esperar?

  for( int i = 0; i < 8; i+= ) {
    if ( cociente == 0 ) {
       digitalWrite(3, BAJA);
       wait_bit();
       continue;
    }
    volatile int resto = cociente % 2;
    cociente = floor( cociente / 2 );
    if ( resto == 1 ) {
      digitalWrite(3, ALTA);
      parity++;
    } else {
      digitalWrite(3,BAJA);
    }
    wait_bit();
  }
  parity = ( parity % 2 ) == 0 ? BAJA : ALTA;
  digitalWrite(3, parity);
  wait_bit();
  digitalWrite(3, LOW); 
}

void responderTrama( int (&dato)[256], int &longitud ) {
  wait_char(3);

  for( int i = 0; i < longitud; i++ ) {
    escribirByte(dato[i]);
  }
}
