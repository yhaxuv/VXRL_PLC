
#include        "config.H"
#include        "main.H"
typedef unsigned char BYTE;
typedef unsigned int WORD;

#define CMD_IDLE    0               // 侍機
#define CMD_READ    1               //IAP讀取資料
#define CMD_PROGRAM 2               //IAP寫入資料
#define CMD_ERASE   3               //IAP清除資料

//#define ENABLE_IAP 0x80           //if SYSCLK<30MHz
//#define ENABLE_IAP 0x81           //if SYSCLK<24MHz
#define ENABLE_IAP  0x82            //if SYSCLK<20MHz
//#define ENABLE_IAP 0x83           //if SYSCLK<12MHz
//#define ENABLE_IAP 0x84           //if SYSCLK<6MHz
//#define ENABLE_IAP 0x85           //if SYSCLK<3MHz
//#define ENABLE_IAP 0x86           //if SYSCLK<2MHz
//#define ENABLE_IAP 0x87           //if SYSCLK<1MHz

//代刚
#define IAP_ADDRESS 0x04000

void Delay(BYTE n);
void IapIdle();
BYTE IapReadByte(WORD addr);
void IapProgramByte(WORD addr, BYTE dat);
void IapEraseSector(WORD addr);
unsigned char id_data;	

unsigned char xdata  disp1_buf[16];
unsigned char xdata  disp2_buf[16];
unsigned char disp_count;

void load_disp(void);
void app_command(void);
//-----------------------------------------------------------//
void main(void)
{
 
    UART_config();
    GPIO_config();
    Timer_config();
    ADC_config();
    EA = 1;
    GET_ID();
	SBUF=ID; 
    ch=2;
	
    Delay(10);   //蛓入ID
	id_data=IapReadByte(0x4000); 
	ID=id_data;
	
    while (1)
    {
        if(temp1>200){
            temp1=0;
            temp2++;
			temp3++;
            if(temp2>100){
              temp2=0;

            }
        }
		if(temp3<10){
			P32=0;
			add1=0;
		}
		else if(temp3<800){
			P32=1;
		}
		else if(temp3<810){
			if(temp3==800){
				Cal_DHT11();
				temp3++;
			}			
		}
		else{			
			temp3=0;
			load_disp();		
		}
        if(rx1_to>=400){
          rx1_to=0;
          if(rx1_cnt>0){
            modbus_s();
            rx1_cnt=0;
          }
          GET_ADC();
		  if(count2>=500){
			count2=0;  
		    Cal_adc();
		  }
        }
		if(rx2_to>=400){
          rx2_to=0;
          if(rx2_cnt>0){
			app_command();
            rx2_cnt=0;			
          }
		}  
        output();

    }
}
//-----------------------------------------------------------//
void load_disp(void){
	 disp_count++;
	 
     P27=1;
     tx2_end=17;
     tx2_cnt=1;
	 TX2_Buffer[0]=0x00; //頭碼*3	
	 TX2_Buffer[1]=0x00;
	 TX2_Buffer[2]=0x00;	
	 TX2_Buffer[3]=0x10;  //長度
	 TX2_Buffer[6]=0x20;	 
     if(disp_count==1){
		TX2_Buffer[4]=0x33; 
 		TX2_Buffer[5]=P34;
		TX2_Buffer[5]+=0x30;
	
	 } 
	 else{
		disp_count=0;
		TX2_Buffer[4]=0x34; 
  		TX2_Buffer[5]=P35;
		TX2_Buffer[5]+=0x30;
		
	 }
     S2BUF=0x00;	
}
//-----------------------------------------------------------//
void app_command(void){
//unsigned char buf;	
	 if(rx2_cnt>0){
		   if(RX2_Buffer[0]==0){
			 RX2_Buffer[0]=0x01;  
		     if(RX2_Buffer[1]==0){
		       if(RX2_Buffer[2]==0){

			     if(RX2_Buffer[3]==0x30){
					id_data = RX2_Buffer[6]&0x0f;
					id_data += (RX2_Buffer[5]&0x0f)*10;
					id_data += (RX2_Buffer[4]&0x0f)*100;	
			        IapEraseSector(0x4000);    //清除資料
                    IapProgramByte(0x4000, id_data);	
					id_data=IapReadByte(0x4000); 
					ID=id_data;
				 }
			     else if(RX2_Buffer[3]==0x31){
					if(P34==0){          //繼電器1
						Modbuf_Buffer[1]&=0xfe;
					}
					else{
						Modbuf_Buffer[1]|=0x01;
					}
				 }	
				 else if(RX2_Buffer[3]==0x32){
					if(P35==0){          //繼電器2 
						Modbuf_Buffer[1]&=0xfd;
					}
					else{
						Modbuf_Buffer[1]|=0x02;
					} 				 
				 }				 
		       }			   
		     }			   
		   }		 
		 
         /*  P27=1;
           tx2_end=17;
           tx2_cnt=1;
	       buf=TX2_Buffer[0]=0x00; //頭碼*3	
		   TX2_Buffer[1]=0x00;
		   TX2_Buffer[2]=0x00;
		   TX2_Buffer[3]=0x10;  //長度
		   //TX2_Buffer[4]=ID;    //站號
		   TX2_Buffer[4]=id_data;    //站號		   
		   TX2_Buffer[5]=~P34;  //繼電器1
		   TX2_Buffer[5]+=0x30;
		   TX2_Buffer[6]=~P35;  //繼電器2
		   TX2_Buffer[6]+=0x30;		   
		   TX2_Buffer[7]=Modbuf_Buffer[10];   //CH1電流
		   TX2_Buffer[8]=Modbuf_Buffer[11];		
		   TX2_Buffer[9]=Modbuf_Buffer[14];   //CH2電流
		   TX2_Buffer[10]=Modbuf_Buffer[15];
		   TX2_Buffer[11]=Modbuf_Buffer[18];  //CH1電壓
		   TX2_Buffer[12]=Modbuf_Buffer[19];
		   TX2_Buffer[13]=Modbuf_Buffer[20];  //CH2電壓
		   TX2_Buffer[14]=Modbuf_Buffer[21];	
		   TX2_Buffer[15]=0x0d;	//尾碼
		   TX2_Buffer[16]=0x0a;	
           S2BUF=buf;
		   */
	 }
}
//-----------------------------------------------------------//
void UART_config(void){
	 SCON = 0x50;		//8bit and variable baudrate
	 AUXR |= 0x40;		//Timer1's clock is Fosc (1T)
	 AUXR &= 0xFE;		//Use Timer1 as baudrate generator
	 TMOD &= 0x0F;		//Set Timer1 as 16-bit auto reload mode
	 TL1 = 0xC7;		//Initial timer value
	 TH1 = 0xFE;		//Initial timer value
	 ET1 = 0;		//Disable Timer1 interrupt
	 TR1 = 1;		//Timer1 running
     ES = 1;

     S2CON = 0x50;           //8bit and variable baudrate
     AUXR |= 0x04;           //Timer2's clock is Fosc (1T)
     TL2 = 0xC7;             //Initial timer value
     TH2 = 0xFE;             //Initial timer value
     AUXR |= 0x10;           //Timer2 running
     UART2_INT_ENABLE();

     P27 = 0;
     P55 = 0;
}
//-----------------------------------------------------------//
void GPIO_config(void){
     P1M1 = 0x3c;
     P1M0 = 0x3c;
     P0M1 = 0x00;
     P0M0 = 0x00;
     P2M1 = 0x00;
     P2M0 = 0x00;
     P3M1 = 0x00;
     P3M0 = 0x00;
	 P5M1 = 0x00;
     P5M0 = 0x00;
	 Modbuf_Buffer[1]=3;
}
//-----------------------------------------------------------//
void Timer_config(void){        //10us@12.000MHz
        TL0 = 0x88;		//Initial timer value
  	    TH0 = 0xFF;		//Initial timer value
        TF0 = 0;                //Clear TF0 flag
        TR0 = 1;                //Timer0 start run
        Timer0_1T();
        Timer0_16bitAutoReload();
        Timer0_InterruptEnable();
        Timer0_Run();
}
//-----------------------------------------------------------//
void ADC_config(void){
     P1ASF = 0x3c;                   //砞﹚P1AD
     ADC_RES = 0;                    //睲埃挡狦盚竟
     ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ch | ADC_START;
}
//-----------------------------------------------------------//
void timer0_int (void) interrupt TIMER0_VECTOR
{
        temp1++;
        if(P21==1){
          read=1;
          if(count1<10){
            count1++;
          }
        }
        else{
          if(read==1){
            read=0;
            DHT11_buf[add1]=count1;
            add1++;
            count1=0;
          }
        }
        rx1_to++;
		rx2_to++;
}

/********************* UART1い耞ㄧΑ************************/
void UART1_int (void) interrupt UART1_VECTOR
{
//    unsigned char buf;
    if(RI)
    {
        RI = 0;
        rx1_to=0;
        if(rx1_cnt<128){
          RX1_Buffer[rx1_cnt]=SBUF;
          rx1_cnt++;
        }
    }
    if(TI)
    {
        TI = 0;
        if(tx1_cnt<tx1_end){
          SBUF=RX1_Buffer[tx1_cnt];
          tx1_cnt++;
        }
        else{
          P55=0;
        }
    }
} 
/********************* UART2い耞ㄧΑ************************/
void UART2_int (void) interrupt UART2_VECTOR
{
    //unsigned char buf;

    if(RI2)
    {
        CLR_RI2();
        //buf=S2BUF;
        //P27=1;
        //S2BUF=buf;
        rx2_to=0;
        if(rx2_cnt<128){
          RX2_Buffer[rx2_cnt]=S2BUF;
          rx2_cnt++;
        }		
    }
    if(TI2)
    {
        CLR_TI2();
        //P27=0;
        if(tx2_cnt<tx2_end){
          S2BUF=TX2_Buffer[tx2_cnt];
          tx2_cnt++;
        }
        else{
          P27=0;
        }		
    }

} 
/**********************************************/
void GET_ID(void)
{
     if(P36==0){
        Modbuf_Buffer[0]|=0x01; 
     }
     else{
        Modbuf_Buffer[0]&=0xfe; 
     }
     if(P37==0){
        Modbuf_Buffer[0]|=0x02; 
     }
     else{
        Modbuf_Buffer[0]&=0xfd; 
     }
     if(P20==0){
        Modbuf_Buffer[0]|=0x04; 
     }
     else{
        Modbuf_Buffer[0]&=0xfb; 
     }
     if(P21==0){
        Modbuf_Buffer[0]|=0x08; 
     }
     else{
        Modbuf_Buffer[0]&=0xf7; 
     }  
     if(P22==0){
        Modbuf_Buffer[0]|=0x10; 
     }
     else{
        Modbuf_Buffer[0]&=0xef; 
     }
     if(P23==0){
        Modbuf_Buffer[0]|=0x20; 
     }
     else{
        Modbuf_Buffer[0]&=0xdf; 
     }
     if(P24==0){
        Modbuf_Buffer[0]|=0x40; 
     }
     else{
        Modbuf_Buffer[0]&=0xbf; 
     }  
     if(P25==0){
        Modbuf_Buffer[0]|=0x80; 
     }
     else{
        Modbuf_Buffer[0]&=0x7f; 
     }  
     ID=Modbuf_Buffer[0];    
}
/**********************************************/
unsigned int usMBCRC16(unsigned int usLen )
{
    unsigned char           ucCRCHi = 0xFF;
    unsigned char          ucCRCLo = 0xFF;
    unsigned int          iIndex;
    unsigned char j;   j=0;
    
    while( usLen-- ){
        iIndex = ucCRCLo ^ RX1_Buffer[j];
        j++;
        ucCRCLo = ( unsigned char )( ucCRCHi ^ aucCRCHi[iIndex] );
        ucCRCHi = aucCRCLo[iIndex];
    }
    return ( unsigned int )( ucCRCHi << 8 | ucCRCLo );
}
/**********************************************/ 
void modbus_s(void){
 unsigned int crc,crc_buf,addr,len,i;   
     if(RX1_Buffer[0]==ID){
       crc_buf=usMBCRC16((rx1_cnt-2));
       crc=(RX1_Buffer[rx1_cnt-1]*256)+RX1_Buffer[rx1_cnt-2];
       if(crc==crc_buf){
         addr=RX1_Buffer[2];
         addr<<=8;
         addr+=RX1_Buffer[3]; 
         addr<<=1;
         if(RX1_Buffer[1]==3){
           len=RX1_Buffer[rx1_cnt-3]*2;
           RX1_Buffer[1]=3;
           RX1_Buffer[2]=len;
           for(i=3;i<len+3;i++){
             RX1_Buffer[i]=Modbuf_Buffer[addr];
             addr++;
           }
           P55=1;
           crc_buf=usMBCRC16(len+3);
           RX1_Buffer[len+4]=crc_buf/256;
           RX1_Buffer[len+3]=crc_buf%256;
           tx1_end=len+5;
           tx1_cnt=1;
           SBUF=ID;
         }
         else if(RX1_Buffer[1]==6){
           Modbuf_Buffer[addr]=RX1_Buffer[4];
           addr++;
           Modbuf_Buffer[addr]=RX1_Buffer[5];
           P55=1;
           tx1_end=rx1_cnt;
           tx1_cnt=1;
           SBUF=ID; 
         }
         else if(RX1_Buffer[1]==16){
           len=RX1_Buffer[6]+7;
           for(i=7; i<len; i++){
              Modbuf_Buffer[addr]=RX1_Buffer[i];
              addr++;
           }
           crc_buf=usMBCRC16(6);
           RX1_Buffer[7]=crc_buf/256;
           RX1_Buffer[6]=crc_buf%256;
           P55=1;
           tx1_end=8;
           tx1_cnt=1;
           SBUF=ID; 
         }
       }
     }

}
/**********************************************/
void output(void){
 
     if(Modbuf_Buffer[1]&1){
       P34=0;
     }
     else{
       P34=1;
     }
     if(Modbuf_Buffer[1]&2){
       P35=0;
     }
     else{
       P35=1;
     }	 
}
/**********************************************/
void GET_ADC(void){
     if(ch==4){
       adc1=ADC_RES;
	   adc1<<=2;
	   adc1+=ADC_RESL;
       Modbuf_Buffer[2]=adc1/256;
       Modbuf_Buffer[3]=adc1%256;
       ch=5;
	   ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ch | ADC_START;
	   if(adc1>A_max1){
         A_max1=adc1;
       }
       else if(adc1<A_min1){
         A_min1=adc1;
       }
     }
     else if(ch==5){
       adc2=ADC_RES;
	   adc2<<=2;
	   adc2+=ADC_RESL;
       Modbuf_Buffer[4]=adc2/256;
       Modbuf_Buffer[5]=adc2%256;
       ch=2;
	   ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ch | ADC_START;
	   if(adc2>A_max2){
         A_max2=adc2;
       }
       else if(adc2<A_min2){
         A_min2=adc2;
       }
     }	 
     else if(ch==2){
       adc3=ADC_RES;
	   adc3<<=2;
	   adc3+=ADC_RESL;
       Modbuf_Buffer[6]=adc3/256;
       Modbuf_Buffer[7]=adc3%256;
       ch=3;
	   ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ch | ADC_START;
	   if(adc3>V_max1){
         V_max1=adc3;		 
       }
	   count2++;
     }	 
     else if(ch==3){
       adc4=ADC_RES;
	   adc4<<=2;
	   adc4+=ADC_RESL;
       Modbuf_Buffer[8]=adc4/256;
       Modbuf_Buffer[9]=adc4%256;
       ch=4;
	   ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ch | ADC_START;
	   if(adc4>V_max2){
         V_max2=adc4;	 
       }
	   count2++;
     }
}
/**********************************************/
void Cal_adc(void){
	 if(A_max1>520){
       A_max1=((A_max1-520)*10)/22;
       Modbuf_Buffer[10]=A_max1/256;
       Modbuf_Buffer[11]=A_max1%256;             
     }
	 
     if(A_min1<520){
       A_min1=((520-A_min1)*10)/22;
       Modbuf_Buffer[12]=A_min1/256;
       Modbuf_Buffer[13]=A_min1%256;             
     }
	 
	 if(A_max2>520){
       A_max2=((A_max2-520)*10)/22;
       Modbuf_Buffer[14]=A_max2/256;
       Modbuf_Buffer[15]=A_max2%256;             
     }
	 
     if(A_min2<520){
       A_min2=((520-A_min2)*10)/22;
       Modbuf_Buffer[16]=A_min2/256;
       Modbuf_Buffer[17]=A_min2%256;             
     }
	 if(V_max1<=340){
		V_max1=V_max1/3; 
	 }
	 else{
		V_max1=(V_max1*4)/17;
	 }
  	 Modbuf_Buffer[18]=V_max1/256;
     Modbuf_Buffer[19]=V_max1%256;

	 if(V_max2<=340){
		V_max2=V_max2/3; 
	 }
	 else{
		V_max2=(V_max2*4)/17;
	 } 
	 Modbuf_Buffer[20]=V_max2/256;
     Modbuf_Buffer[21]=V_max2%256;	
	 
	 A_min1=520;
     A_max1=0;
     V_max1=0;	
	 A_min2=520;
     A_max2=0;
     V_max2=0;	 
}
/**********************************************/
void Cal_DHT11(void){
unsigned char i,j;
     j=0;
     for(i=2;i<10;i++){
		 j<<=1;
		if(DHT11_buf[i]>3){
			j|=1;
		} 
	 }	 
	 Modbuf_Buffer[22]=j;
 	 j=0;
     for(i=10;i<18;i++){
		 j<<=1;
		if(DHT11_buf[i]>3){
			j|=1;
		} 
	 }	 
	 Modbuf_Buffer[23]=j;
     for(i=18;i<26;i++){
		 j<<=1;
		if(DHT11_buf[i]>3){
			j|=1;
		} 
	 }	 
	 Modbuf_Buffer[24]=j;
 	 j=0;
     for(i=26;i<34;i++){
		 j<<=1;
		if(DHT11_buf[i]>3){
			j|=1;
		} 
	 }	 
	 Modbuf_Buffer[25]=j;	 
}
/**********************************************/
/*----------------------------
?件延?
----------------------------*/
void Delay(BYTE n)
{
    WORD x;

    while (n--)
    {
        x = 0;
        while (++x);
    }
}

/*----------------------------
??IAP
----------------------------*/
void IapIdle()
{
    IAP_CONTR = 0;                  //??IAP功能
    IAP_CMD = 0;                    //清除指令寄存器
    IAP_TRIG = 0;                   //清除觸?寄存器
    IAP_ADDRH = 0x80;               //?位址?置到非IAP?網域
    IAP_ADDRL = 0;
}

/*----------------------------
?ISP/IAP/EEPROM?網域?取一字?
----------------------------*/
BYTE IapReadByte(WORD addr)
{
    BYTE dat;                       //?據???

    IAP_CONTR = ENABLE_IAP;         //使能IAP
    IAP_CMD = CMD_READ;             //?置IAP指令
    IAP_ADDRL = addr;               //?置IAP低位址
    IAP_ADDRH = addr >> 8;          //?置IAP高位址
    IAP_TRIG = 0x5a;                //?觸?指令(0x5a)
    IAP_TRIG = 0xa5;                //?觸?指令(0xa5)
    _nop_();                        //等待ISP/IAP/EEPROM作業完成
    dat = IAP_DATA;                 //?ISP/IAP/EEPROM?據
    IapIdle();                      //??IAP功能

    return dat;                     //返回
}

/*----------------------------
?一字??據到ISP/IAP/EEPROM?網域
----------------------------*/
void IapProgramByte(WORD addr, BYTE dat)
{
    IAP_CONTR = ENABLE_IAP;         //使能IAP
    IAP_CMD = CMD_PROGRAM;          //?置IAP指令
    IAP_ADDRL = addr;               //?置IAP低位址
    IAP_ADDRH = addr >> 8;          //?置IAP高位址
    IAP_DATA = dat;                 //?ISP/IAP/EEPROM?據
    IAP_TRIG = 0x5a;                //?觸?指令(0x5a)
    IAP_TRIG = 0xa5;                //?觸?指令(0xa5)
    _nop_();                        //等待ISP/IAP/EEPROM作業完成
    IapIdle();
}

/*----------------------------
扇?擦除
----------------------------*/
void IapEraseSector(WORD addr)
{
    IAP_CONTR = ENABLE_IAP;         //使能IAP
    IAP_CMD = CMD_ERASE;            //?置IAP指令
    IAP_ADDRL = addr;               //?置IAP低位址
    IAP_ADDRH = addr >> 8;          //?置IAP高位址
    IAP_TRIG = 0x5a;                //?觸?指令(0x5a)
    IAP_TRIG = 0xa5;                //?觸?指令(0xa5)
    _nop_();                        //等待ISP/IAP/EEPROM作業完成
    IapIdle();
}
