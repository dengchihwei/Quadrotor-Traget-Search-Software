
#ifndef NRF_DATA_H

#define NRF_DATA_H


#define NRF_DATA_PACKET_INT 13
#define NRF_DATA_PACKET_CHAR NRF_DATA_PACKET_INT*2
#define NRF_DATA_PACKET NRF_DATA_PACKET_CHAR

union NrfBuff
{
  uint8  Data_char[NRF_DATA_PACKET_CHAR];
  int16  Data_int[NRF_DATA_PACKET_INT];
};
typedef union NrfBuff NrfBuff;



#endif  //NRFDATA_H