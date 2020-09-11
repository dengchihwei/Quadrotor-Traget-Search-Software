/*!
 *     COPYRIGHT NOTICE
 *     Copyright (c) 2013,Ұ��Ƽ�
 *     All rights reserved.
 *     �������ۣ�Ұ���ѧ��̳ http://www.chuxue123.com
 *
 *     ��ע�������⣬�����������ݰ�Ȩ����Ұ��Ƽ����У�δ������������������ҵ��;��
 *     �޸�����ʱ���뱣��Ұ��Ƽ��İ�Ȩ������
 *
 * @file       FIRE_NRF24L0.c
 * @brief      ���ߵ���  ����
 * @author     Ұ��Ƽ�
 * @version    v5.0
 * @date       2014-01-04
 */

#include "common.h"
//#include "MKL_port.h"
//#include "MKL_gpio.h"
//#include "MKL_spi.h"

#include "YRK_NRF24L0.h"


#include "inc/hw_types.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_ints.h"
#include "inc/hw_nvic.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/rom_map.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
   


//������Ӳ������
#define NRF_SSI_PERIPH                  SYSCTL_PERIPH_SSI0
#define NRF_SSI_GPIO_PERIPH             SYSCTL_PERIPH_GPIOA
#define NRF_SSI_GPIO_BASE               GPIO_PORTA_BASE
#define NRF_SSI_BASE                    SSI0_BASE
#define NRF_CSN_PERIPH                  SYSCTL_PERIPH_GPIOA
#define NRF_CSN_BASE                    GPIO_PORTA_BASE
#define NRF_CSN_PIN                     GPIO_PIN_3
#define NRF_CE_PERIPH                   SYSCTL_PERIPH_GPIOC
#define NRF_CE_BASE                     GPIO_PORTC_BASE
#define NRF_CE_PIN                      GPIO_PIN_4
#define NRF_IRQ_PERIPH                  SYSCTL_PERIPH_GPIOC
#define NRF_IRQ_BASE                    GPIO_PORTC_BASE
#define NRF_IRQ_PIN                     GPIO_PIN_5
#define NRF_IRQ_INT_VECTOR              INT_GPIOC
#define NRF_IRQ_INT_PIN                 GPIO_INT_PIN_5



//#define NRF_SPI         SPI1
//#define NRF_CS          SPI_PCS0

//#define NRF_CE_PTXn     PTE5
//#define NRF_IRQ_PTXn    PTC18


//NRF24L01+״̬
typedef enum
{
    NOT_INIT = 0,
    TX_MODE,
    RX_MODE,
} nrf_mode_e;

typedef enum
{
    QUEUE_EMPTY = 0,        //���п�ģʽ��ֻ�������
    QUEUE_NORMAL,           //����ģʽ��������������У������в��ղ���
    QUEUE_FULL,             //������ģʽ�������������ӣ�����������
} nrf_rx_queueflag_e; //�жϽ���ʱ������״̬���λ


//gpio����CE��IRQ
#define NRF_CE_HIGH()       ROM_GPIOPinWrite(NRF_CE_BASE, NRF_CE_PIN, NRF_CE_PIN)
#define NRF_CE_LOW()        ROM_GPIOPinWrite(NRF_CE_BASE, NRF_CE_PIN, 0)          //CE�õ�

#define NRF_CSN_HIGH()      ROM_GPIOPinWrite(NRF_CSN_BASE, NRF_CSN_PIN, NRF_CSN_PIN)
#define NRF_CSN_LOW()       ROM_GPIOPinWrite(NRF_CSN_BASE, NRF_CSN_PIN, 0)          //CE�õ�

#define NRF_Read_IRQ()      GPIOPinRead(NRF_IRQ_BASE,NRF_IRQ_PIN)

// �û����� ���ͺ� ���յ�ַ��Ƶ��

uint8 TX_ADDRESS[5] = {0xff, 0xff, 0xff, 0xff, 0xff};   // ����һ����̬���͵�ַ
uint8 RX_ADDRESS[5] = {0xff, 0xff, 0xff, 0xff, 0xff};

#define CHANAL           24                            //Ƶ��ѡ��


// �ڲ����ò���
#define TX_ADR_WIDTH    ADR_WIDTH                       //�����ַ����
#define TX_PLOAD_WIDTH  DATA_PACKET                     //��������ͨ����Ч���ݿ���0~32Byte

#define RX_ADR_WIDTH    ADR_WIDTH                       //���յ�ַ����
#define RX_PLOAD_WIDTH  DATA_PACKET                     //��������ͨ����Ч���ݿ���0~32Byte

/******************************** NRF24L01+ �Ĵ������� �궨��***************************************/

// SPI(nRF24L01) commands , NRF��SPI����궨�壬���NRF����ʹ���ĵ�
#define NRF_READ_REG    0x00    // Define read command to register
#define NRF_WRITE_REG   0x20    // Define write command to register
#define RD_RX_PLOAD     0x61    // Define RX payload register address
#define WR_TX_PLOAD     0xA0    // Define TX payload register address
#define FLUSH_TX        0xE1    // Define flush TX register command
#define FLUSH_RX        0xE2    // Define flush RX register command
#define REUSE_TX_PL     0xE3    // Define reuse TX payload register command
#define NOP             0xFF    // Define No Operation, might be used to read status register

// SPI(nRF24L01) registers(addresses) ��NRF24L01 ��ؼĴ�����ַ�ĺ궨��
#define CONFIG      0x00        // 'Config' register address
#define EN_AA       0x01        // 'Enable Auto Acknowledgment' register address
#define EN_RXADDR   0x02        // 'Enabled RX addresses' register address
#define SETUP_AW    0x03        // 'Setup address width' register address
#define SETUP_RETR  0x04        // 'Setup Auto. Retrans' register address
#define RF_CH       0x05        // 'RF channel' register address
#define RF_SETUP    0x06        // 'RF setup' register address
#define STATUS      0x07        // 'Status' register address
#define OBSERVE_TX  0x08        // 'Observe TX' register address
#define CD          0x09        // 'Carrier Detect' register address
#define RX_ADDR_P0  0x0A        // 'RX address pipe0' register address
#define RX_ADDR_P1  0x0B        // 'RX address pipe1' register address
#define RX_ADDR_P2  0x0C        // 'RX address pipe2' register address
#define RX_ADDR_P3  0x0D        // 'RX address pipe3' register address
#define RX_ADDR_P4  0x0E        // 'RX address pipe4' register address
#define RX_ADDR_P5  0x0F        // 'RX address pipe5' register address
#define TX_ADDR     0x10        // 'TX address' register address
#define RX_PW_P0    0x11        // 'RX payload width, pipe0' register address
#define RX_PW_P1    0x12        // 'RX payload width, pipe1' register address
#define RX_PW_P2    0x13        // 'RX payload width, pipe2' register address
#define RX_PW_P3    0x14        // 'RX payload width, pipe3' register address
#define RX_PW_P4    0x15        // 'RX payload width, pipe4' register address
#define RX_PW_P5    0x16        // 'RX payload width, pipe5' register address
#define FIFO_STATUS 0x17        // 'FIFO Status Register' register address


//������Ҫ��״̬���
#define TX_FULL     0x01        //TX FIFO �Ĵ�������־�� 1 Ϊ ����0Ϊ ����
#define MAX_RT      0x10        //�ﵽ����ط������жϱ�־λ
#define TX_DS       0x20        //��������жϱ�־λ
#define RX_DR       0x40        //���յ������жϱ�־λ



//�ڲ��Ĵ���������������
static  uint8   nrf_writereg(uint8 reg, uint8 dat);
static  uint8   nrf_readreg (uint8 reg, uint8 *dat);

static  uint8   nrf_writebuf(uint8 reg , uint8 *pBuf, uint32 len);
static  uint8   nrf_readbuf (uint8 reg, uint8 *pBuf, uint32  len);


static  void    nrf_rx_mode(void);           //�������ģʽ
static  void    nrf_tx_mode(void);           //���뷢��ģʽ

/*!
 *  @brief      NRF24L01+ ģʽ���
 */
volatile uint8  nrf_mode = NOT_INIT;


volatile uint8  nrf_rx_front = 0, nrf_rx_rear = 0;              //����FIFO��ָ��
volatile uint8  nrf_rx_flag = QUEUE_EMPTY;

uint8 NRF_ISR_RX_FIFO[RX_FIFO_PACKET_NUM][DATA_PACKET];         //�жϽ��յ�FIFO


volatile uint8    *nrf_irq_tx_addr      = NULL;
volatile uint32    nrf_irq_tx_pnum      = 0;                    //pnum = (len+MAX_ONCE_TX_NUM -1)  / MAX_ONCE_TX_NUM

volatile uint8      nrf_irq_tx_flag  = 0;                     //0 ��ʾ�ɹ� ��1 ��ʾ ����ʧ��

/*!
 *  @brief      NRF24L01+��ʼ����Ĭ�Ͻ������ģʽ
 *  @return     ��ʼ���ɹ���ǣ�0Ϊ��ʼ��ʧ�ܣ�1Ϊ��ʼ���ɹ�
 *  @since      v5.0
 *  Sample usage:
                     while(!nrf_init())                                     //��ʼ��NRF24L01+ ,�ȴ���ʼ���ɹ�Ϊֹ
                     {
                         printf("\n  NRF��MCU����ʧ�ܣ������¼����ߡ�\n");
                     }

                     printf("\n      NRF��MCU���ӳɹ���\n");
 */
void config_ssi_gpio(void)
{
  /* Config Tx on SSI1, PF0-PF3 + PB0/PB3. */
  ROM_SysCtlPeripheralEnable(NRF_SSI_PERIPH);
  ROM_SysCtlPeripheralEnable(NRF_SSI_GPIO_PERIPH);

  ROM_GPIOPinConfigure(GPIO_PA2_SSI0CLK);
  ROM_GPIOPinConfigure(GPIO_PA4_SSI0RX);
  ROM_GPIOPinConfigure(GPIO_PA5_SSI0TX);
  ROM_GPIOPinTypeSSI(NRF_SSI_GPIO_BASE, GPIO_PIN_2|GPIO_PIN_4|GPIO_PIN_5);
  
  /* CSN pin, high initially */
  ROM_SysCtlPeripheralEnable(NRF_CSN_PERIPH);
  ROM_GPIOPinTypeGPIOOutput(NRF_CSN_BASE, NRF_CSN_PIN);
  ROM_GPIOPinWrite(NRF_CSN_BASE, NRF_CSN_PIN, NRF_CSN_PIN);
  /* CE pin, low initially */
  ROM_SysCtlPeripheralEnable(NRF_CE_PERIPH);
  ROM_GPIOPinTypeGPIOOutput(NRF_CE_BASE, NRF_CE_PIN);
  ROM_GPIOPinWrite(NRF_CE_BASE, NRF_CE_PIN, NRF_CE_PIN);
  /* IRQ pin as input. */
  ROM_SysCtlPeripheralEnable(NRF_IRQ_PERIPH);
  ROM_GPIOPinTypeGPIOInput(NRF_IRQ_BASE, NRF_IRQ_PIN);

}
void config_spi(uint32_t base)
{
  /*
    Configure the SPI for correct mode to read from nRF24L01+.

    We need CLK inactive low, so SPO=0.
    We need to setup and sample on the leading, rising CLK edge, so SPH=0.

    The datasheet says up to 10MHz SPI is possible, depending on load
    capacitance. Let's go with a slightly cautious 8MHz, which should be
    aplenty.
  */

  //ROM_SSIDisable(base);
  ROM_SSIConfigSetExpClk(base, ROM_SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
                         SSI_MODE_MASTER, 9000000, 8);
  ROM_SSIEnable(base);
}
void nrfspiinit()
{
    config_ssi_gpio();
    config_spi(NRF_SSI_BASE);
}
uint8 nrftest()
{
  uint8 data;
  nrf_readreg(FIFO_STATUS,&data);
  return data;
}
uint8 nrf_IRQ_reg(void (*pfnHandler)(void))
{
IntRegister(NRF_IRQ_INT_VECTOR,pfnHandler);
return 0;
}

uint8 nrf_init(void (*pfnHandler)(void))
{
    //����NRF�ܽŸ���
    //spi_init(NRF_SPI, NRF_CS, MASTER,12500*1000);                     //��ʼ��SPI,����ģʽ
     // gpio_init(NRF_CE_PTXn, GPO, LOW);                               //��ʼ��CE��Ĭ�Ͻ������ģʽ

   // gpio_init(NRF_IRQ_PTXn, GPI, LOW);                              //��ʼ��IRQ�ܽ�Ϊ����
  
    nrfspiinit();
   
    ROM_GPIOIntTypeSet(NRF_IRQ_BASE,NRF_IRQ_PIN,GPIO_FALLING_EDGE);

    GPIOIntEnable(NRF_IRQ_BASE, NRF_IRQ_INT_PIN);
    IntRegister(NRF_IRQ_INT_VECTOR,pfnHandler);
    IntEnable(NRF_IRQ_INT_VECTOR); 
    //port_init_NoALT(NRF_IRQ_PTXn, IRQ_FALLING | PULLUP);            //��ʼ��IRQ�ܽ�Ϊ�½��� �����ж�

    //����NRF�Ĵ���
    NRF_CE_LOW();

    nrf_writereg(NRF_WRITE_REG + SETUP_AW, ADR_WIDTH - 2);          //���õ�ַ����Ϊ TX_ADR_WIDTH

    nrf_writereg(NRF_WRITE_REG + RF_CH, CHANAL);                    //����RFͨ��ΪCHANAL
    nrf_writereg(NRF_WRITE_REG + RF_SETUP, 0x0f);                   //����TX�������,0db����,2Mbps,���������濪��

    nrf_writereg(NRF_WRITE_REG + EN_AA, 0x01);                      //ʹ��ͨ��0���Զ�Ӧ��

    nrf_writereg(NRF_WRITE_REG + EN_RXADDR, 0x01);                  //ʹ��ͨ��0�Ľ��յ�ַ

    //RXģʽ����
    nrf_writebuf(NRF_WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH); //дRX�ڵ��ַ

    nrf_writereg(NRF_WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH);         //ѡ��ͨ��0����Ч���ݿ���

    nrf_writereg(FLUSH_RX, NOP);                                    //���RX FIFO�Ĵ���

    //TXģʽ����
    nrf_writebuf(NRF_WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH); //дTX�ڵ��ַ

    nrf_writereg(NRF_WRITE_REG + SETUP_RETR, 0x08);                 //�����Զ��ط����ʱ��:250us + 86us;����Զ��ط�����:15��

    nrf_writereg(FLUSH_TX, NOP);                                    //���TX FIFO�Ĵ���

    nrf_rx_mode();                                                  //Ĭ�Ͻ������ģʽ

    NRF_CE_HIGH();

    return nrf_link_check();

}

/*!
 *  @brief      NRF24L01+д�Ĵ���
 *  @param      reg         �Ĵ���
 *  @param      dat         ��Ҫд�������
 *  @return     NRF24L01+ ״̬
 *  @since      v5.0
 *  Sample usage:    nrf_writereg(NRF_WRITE_REG + RF_CH, CHANAL);   //����RFͨ��ΪCHANAL
 */
void ssi_cmd(uint8_t *recvbuf, const uint8_t *sendbuf, uint32_t len,
        uint32_t ssi_base)
{
  uint32_t i;
  uint32_t data;

  /* Take CSN low to initiate transfer. */
  NRF_CSN_LOW(); 

  for (i = 0; i < len; ++i)
  {
    ROM_SSIDataPut(ssi_base, sendbuf[i]);
    while (ROM_SSIBusy(ssi_base))
      ;
    ROM_SSIDataGet(ssi_base, &data);
    recvbuf[i] = data;
  }
  NRF_CSN_HIGH(); 
}
void bzero(uint8_t *buf, uint32_t len)
{
  while (len > 0)
  {
    *buf++ = 0;
    --len;
  }
}

uint8 nrf_writereg(uint8 reg, uint8 dat)
{
    uint8 buff[2];

    buff[0] = reg;          //�ȷ��ͼĴ���
    buff[1] = dat;          //�ٷ�������

    ssi_cmd(buff, buff, 2,NRF_SSI_BASE); //����buff�����ݣ����ɼ��� buff��

    /*����״̬�Ĵ�����ֵ*/
    return buff[0];
}

/*!
 *  @brief      NRF24L01+���Ĵ���
 *  @param      reg         �Ĵ���
 *  @param      dat         ��Ҫ��ȡ�����ݵĴ�ŵ�ַ
 *  @return     NRF24L01+ ״̬
 *  @since      v5.0
 *  Sample usage:
                    uint8 data;
                    nrf_readreg(STATUS,&data);
 */
uint8 nrf_readreg(uint8 reg, uint8 *dat)
{

    uint8 buff[2];

    buff[0] = reg;          //�ȷ��ͼĴ���

    ssi_cmd(buff, buff, 2,NRF_SSI_BASE); //����buff���ݣ������浽buff��

    *dat = buff[1];                         //��ȡ�ڶ�������

    /*����״̬�Ĵ�����ֵ*/
    return buff[0];
}

/*!
 *  @brief      NRF24L01+д�Ĵ���һ������
 *  @param      reg         �Ĵ���
 *  @param      pBuf        ��Ҫд������ݻ�����
 *  @param      len         ��Ҫд�����ݳ���
 *  @return     NRF24L01+ ״̬
 *  @since      v5.0
 *  Sample usage:    nrf_writebuf(NRF_WRITE_REG+TX_ADDR,TX_ADDRESS,TX_ADR_WIDTH);    //дTX�ڵ��ַ
 */
uint8 nrf_writebuf(uint8 reg , uint8 *pBuf, uint32 len)
{
    uint8_t sendbuf[33], recvbuf[33];

  if (len > 32)
    len = 32;
  sendbuf[0] = reg;
  memcpy(&sendbuf[1], pBuf, len);
  ssi_cmd(recvbuf, sendbuf, len+1, NRF_SSI_BASE);
   // ssi_cmd(NRF_SPI, NRF_CS, &reg , NULL, pBuf, NULL, 1 , len); //���� reg ��pBuf ���ݣ�������
  return reg;    //����NRF24L01��״̬
}


/*!
 *  @brief      NRF24L01+���Ĵ���һ������
 *  @param      reg         �Ĵ���
 *  @param      dat         ��Ҫ��ȡ�����ݵĴ�ŵ�ַ
 *  @param      len         ��Ҫ��ȡ�����ݳ���
 *  @return     NRF24L01+ ״̬
 *  @since      v5.0
 *  Sample usage:
                    uint8 data;
                    nrf_readreg(STATUS,&data);
 */
uint8 nrf_readbuf(uint8 reg, uint8 *pBuf, uint32 len)
{
    //spi_mosi_cmd(NRF_SPI, NRF_CS, &reg , NULL, NULL, pBuf, 1 , len); //����reg�����յ�buff
  uint8_t sendbuf[33], recvbuf[33];
  if (len > 32)
    len = 32;
  sendbuf[0] = reg;
  bzero(&sendbuf[1], len);
  ssi_cmd(recvbuf, sendbuf, len+1, NRF_SSI_BASE);
  memcpy(pBuf, &recvbuf[1], len);

    return reg;    //����NRF24L01��״̬
}

/*!
*  @brief      ���NRF24L01+��MCU�Ƿ���������
*  @return     NRF24L01+ ��ͨ��״̬��0��ʾͨ�Ų�������1��ʾ����
*  @since      v5.0
*/
uint8 nrf_link_check(void)
{
#define NRF_CHECH_DATA  0xC2        //��ֵΪУ������ʱʹ�ã����޸�Ϊ����ֵ

    uint8 reg;

    uint8 buff[5] = {NRF_CHECH_DATA, NRF_CHECH_DATA, NRF_CHECH_DATA, NRF_CHECH_DATA, NRF_CHECH_DATA};
    uint8 i;


    reg = NRF_WRITE_REG + TX_ADDR;
    //spi_mosi_cmd(NRF_SPI, NRF_CS, &reg, NULL , buff, NULL, 1 , 5); //д��У������
    nrf_writebuf(reg , buff, 5);
    //ssi_cmd(recvbuf, sendbuf, len+1, NRF_SSI_BASE);

    reg = TX_ADDR;
    //spi_mosi_cmd(NRF_SPI, NRF_CS, &reg, NULL , NULL, buff, 1 , 5); //��ȡУ������
    nrf_readbuf(reg , buff, 5);


    /*�Ƚ�*/
    for(i = 0; i < 5; i++)
    {
        if(buff[i] != NRF_CHECH_DATA)
        {
            return 0 ;        //MCU��NRF����������
        }
    }
    return 1 ;             //MCU��NRF�ɹ�����
}

/*!
*  @brief      NRF24L01+�������ģʽ
*  @since      v5.0
*/
void nrf_rx_mode(void)
{
    NRF_CE_LOW();

    nrf_writereg(NRF_WRITE_REG + EN_AA, 0x01);          //ʹ��ͨ��0���Զ�Ӧ��

    nrf_writereg(NRF_WRITE_REG + EN_RXADDR, 0x01);      //ʹ��ͨ��0�Ľ��յ�ַ

    nrf_writebuf(NRF_WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH); //дRX�ڵ��ַ


    nrf_writereg(NRF_WRITE_REG + CONFIG, 0x0B | (IS_CRC16 << 2));       //���û�������ģʽ�Ĳ���;PWR_UP,EN_CRC,16BIT_CRC,����ģʽ

    /* ����жϱ�־*/
    nrf_writereg(NRF_WRITE_REG + STATUS, 0xff);

    nrf_writereg(FLUSH_RX, NOP);                    //���RX FIFO�Ĵ���

    /*CE���ߣ��������ģʽ*/
    NRF_CE_HIGH();

    nrf_mode = RX_MODE;
}

/*!
*  @brief      NRF24L01+���뷢��ģʽ
*  @since      v5.0
*/
void nrf_tx_mode(void)
{
    volatile uint32 i;

    NRF_CE_LOW();
    //DELAY_MS(1);

    nrf_writebuf(NRF_WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH); //дTX�ڵ��ַ

    nrf_writebuf(NRF_WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH); //����RX�ڵ��ַ ,��ҪΪ��ʹ��ACK

    nrf_writereg(NRF_WRITE_REG + CONFIG, 0x0A | (IS_CRC16 << 2)); //���û�������ģʽ�Ĳ���;PWR_UP,EN_CRC,16BIT_CRC,����ģʽ,���������ж�


    /*CE���ߣ����뷢��ģʽ*/
    NRF_CE_HIGH();

    nrf_mode = TX_MODE;

    i = 0x0fff;
    while(i--);         //CEҪ����һ��ʱ��Ž��뷢��ģʽ

    //DELAY_MS(1);


}




uint32  nrf_rx(uint8 *rxbuf, uint32 len)
{
    uint32 tmplen = 0;
    uint8 tmp;

    while( (nrf_rx_flag != QUEUE_EMPTY) && (len != 0) )
    {
        if(len < DATA_PACKET)
        {
            memcpy(rxbuf, (uint8 *)&(NRF_ISR_RX_FIFO[nrf_rx_front]), len);///////12.1����

            NRF_CE_LOW();           //�������״̬

            nrf_rx_front++;                //���ڷǿգ����Կ���ֱ�ӳ�����

            if(nrf_rx_front >= RX_FIFO_PACKET_NUM)
            {
                nrf_rx_front = 0;          //��ͷ��ʼ
            }
            tmp =  nrf_rx_rear;
            if(nrf_rx_front == tmp)       //׷��ƨ���ˣ����ն��п�
            {
                nrf_rx_flag = QUEUE_EMPTY;
            }
            NRF_CE_HIGH();          //�������ģʽ

            tmplen += len;
            return tmplen;
        }

        memcpy(rxbuf, (uint8 *)&(NRF_ISR_RX_FIFO[nrf_rx_front]), DATA_PACKET);
        rxbuf   += DATA_PACKET;
        len     -= DATA_PACKET;
        tmplen  += DATA_PACKET;

        NRF_CE_LOW();           //�������״̬

        nrf_rx_front++;                //���ڷǿգ����Կ���ֱ�ӳ�����

        if(nrf_rx_front >= RX_FIFO_PACKET_NUM)
        {
            nrf_rx_front = 0;          //��ͷ��ʼ
        }
        tmp  = nrf_rx_rear;
        if(nrf_rx_front == tmp)       //׷��ƨ���ˣ����ն��п�
        {
            nrf_rx_flag = QUEUE_EMPTY;
        }
        else
        {
            nrf_rx_flag = QUEUE_NORMAL;
        }
        NRF_CE_HIGH();          //�������ģʽ
    }

    return tmplen;
}

uint8    nrf_tx(uint8 *txbuf, uint32 len)
{
    nrf_irq_tx_flag = 0;        //��λ��־λ

    if((txbuf == 0 ) || (len == 0))
    {
        return 0;
    }

    if(nrf_irq_tx_addr == 0 )
    {
        //
        nrf_irq_tx_pnum = (len - 1) / DATA_PACKET;        // �� 1 ��� �� ����Ŀ
        nrf_irq_tx_addr = txbuf;

        if( nrf_mode != TX_MODE)
        {
            nrf_tx_mode();
        }

        //��Ҫ �ȷ���һ�����ݰ������ �жϷ���

        /*ceΪ�ͣ��������ģʽ1*/
        NRF_CE_LOW();

        /*д���ݵ�TX BUF ��� 32���ֽ�*/
        nrf_writebuf(WR_TX_PLOAD, txbuf, DATA_PACKET);

        /*CEΪ�ߣ�txbuf�ǿգ��������ݰ� */
        NRF_CE_HIGH();

        return 1;
    }
    else
    {
        return 0;
    }
}

nrf_tx_state_e nrf_tx_state ()
{
    /*
    if(nrf_mode != TX_MODE)
    {
        return NRF_NOT_TX;
    }
    */

    if((nrf_irq_tx_addr == 0) && (nrf_irq_tx_pnum == 0))
    {
        //�������
        if(nrf_irq_tx_flag)
        {
            return NRF_TX_ERROR;
        }
        else
        {
            return NRF_TX_OK;
        }
    }
    else
    {
        return NRF_TXING;
    }
}

void nrf_handler(void)
{
    uint8 state;
    uint8 tmp;
    /*��ȡstatus�Ĵ�����ֵ  */
    nrf_readreg(STATUS, &state);

    /* ����жϱ�־*/
    nrf_writereg(NRF_WRITE_REG + STATUS, state);

    if(state & RX_DR) //���յ�����
    {
        NRF_CE_LOW();

        if(nrf_rx_flag != QUEUE_FULL)
        {
            //��û�������������
            //printf("+");
            nrf_readbuf(RD_RX_PLOAD, (uint8 *)&(NRF_ISR_RX_FIFO[nrf_rx_rear]), RX_PLOAD_WIDTH); //��ȡ����

            nrf_rx_rear++;

            if(nrf_rx_rear >= RX_FIFO_PACKET_NUM)
            {
                nrf_rx_rear = 0;                            //��ͷ��ʼ
            }
            tmp = nrf_rx_front;
            if(nrf_rx_rear == tmp)                 //׷��ƨ���ˣ�����
            {
                nrf_rx_flag = QUEUE_FULL;
            }
            else
            {
                nrf_rx_flag = QUEUE_NORMAL;
            }
        }
        else
        {
            nrf_writereg(FLUSH_RX, NOP);                    //���RX FIFO�Ĵ���
        }
        NRF_CE_HIGH();                                      //�������ģʽ
    }

    if(state & TX_DS) //����������
    {
        if(nrf_irq_tx_pnum == 0)
        {
            nrf_irq_tx_addr = 0;

            // ע��: nrf_irq_tx_pnum == 0 ��ʾ ���� �Ѿ�ȫ�����͵�FIFO �� nrf_irq_tx_addr == 0 ���� ȫ����������

            //������ɺ� Ĭ�� ���� ����ģʽ
#if 1
            if( nrf_mode != RX_MODE)
            {
                nrf_rx_mode();
            }
#endif

            //���ͳ��� Ϊ 0���������������
            //nrf_writereg(FLUSH_TX, NOP);                        //���TX FIFO�Ĵ���
        }
        else
        {
            if( nrf_mode != TX_MODE)
            {
                nrf_tx_mode();
            }

            //��û������ɣ��ͼ�������
            nrf_irq_tx_addr += DATA_PACKET;    //ָ����һ����ַ
            nrf_irq_tx_pnum --;                 //����Ŀ����

            /*ceΪ�ͣ��������ģʽ1*/
            NRF_CE_LOW();

            /*д���ݵ�TX BUF ��� 32���ֽ�*/
            nrf_writebuf(WR_TX_PLOAD, (uint8 *)nrf_irq_tx_addr, DATA_PACKET);

            /*CEΪ�ߣ�txbuf�ǿգ��������ݰ� */
            NRF_CE_HIGH();
        }
    }

    if(state & MAX_RT)      //���ͳ�ʱ
    {
        nrf_irq_tx_flag = 1;                            //��Ƿ���ʧ��
        nrf_writereg(FLUSH_TX, NOP);                    //���TX FIFO�Ĵ���


        //�п����� �Է�Ҳ���� ����״̬

        //�������η���
        nrf_irq_tx_addr = 0;
        nrf_irq_tx_pnum = 0;

        nrf_rx_mode();                                  //���� ����״̬


        //printf("\nMAX_RT");
    }

    if(state & TX_FULL) //TX FIFO ��
    {
        //printf("\nTX_FULL");

    }
}


//��� ����FIFO ������  (0 û���չ� ��1 Ϊ������ȷ)
uint8  nrf_rx_fifo_check(uint32 offset,uint16 * val)
{
    uint8 rx_num = (offset + 1 + DATA_PACKET - 1 ) / DATA_PACKET;   //��1 ����Ϊ����2���ֽڣ����һ���Լ����ڵİ�����
                                                                    //+ DATA_PACKET - 1 ����������
    uint8 tmp;
    if(nrf_rx_flag == QUEUE_EMPTY)
    {
        return 0;
    }

    if(rx_num > RX_FIFO_PACKET_NUM)                                 //ƫ��̫�󣬳��� FIFO ����
    {
        return 0;
    }

    rx_num = nrf_rx_front + rx_num - 1;                             //Ŀ���ѯ�� ����λ��
    tmp =  nrf_rx_rear;
    if(nrf_rx_front <  tmp)                                 //���������� һȦ֮��
    {
        if(rx_num >= nrf_rx_rear )                                  //û�����㹻������
        {
            return 0;
        }

        //��ȡ����

    }
    else                                                            //Խ��һȦ
    {
        if(rx_num >= RX_FIFO_PACKET_NUM)                            //����һȦ
        {
            rx_num -= RX_FIFO_PACKET_NUM;

            if( rx_num >= nrf_rx_rear )                             //��û���չ�
            {
                return 0;
            }
        }
        //��ȡ����
    }

    //��ȡ����
    *val = *(uint16 *)((char *)&NRF_ISR_RX_FIFO + ( rx_num*DATA_PACKET + (offset % DATA_PACKET - 2) )) ;
    return 1;

}

