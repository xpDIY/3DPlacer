
#include "common.h"


int8_t flash_erase(){
  uint32_t SECTORError = 0;
  FLASH_EraseInitTypeDef EraseInitStruct;
  // Erase type = sector
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGEERASE;
  // Erase address start
  EraseInitStruct.PageAddress = FLASH_USER_START_ADDR;
  // Number of sectors
  EraseInitStruct.NbPages = 1;
  // Erase
  if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK)
  {
    return -1;
  }
  return 0;  
}

void write_flash(uint32_t* data, uint32_t length){
  // Unlock flash
  HAL_FLASH_Unlock();
  flash_erase();
  
  uint32_t flash_program_start = FLASH_USER_START_ADDR ;
  uint32_t flash_program_end = (FLASH_USER_START_ADDR + length);
  uint32_t *src = data;

  while (flash_program_start < flash_program_end)
  {
    // Write to flash
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_PAGE, flash_program_start, src) == HAL_OK)
    {
      // Move flash point to next page
      flash_program_start += FLASH_PAGE_SIZE;
      // Move data point
      src += FLASH_PAGE_SIZE / 4;
    }
  }
}

//read flash into buffer, align to boundary 4
void read_flash(uint32_t* data,uint32_t length){
    for(int i=0;i<length/4;++i){
        *(data+i) = HW32_REG(FLASH_USER_START_ADDR + i*4);
    }
}


char b64buf[17]="";
const char b64d[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_";
char* uid_to_string(uint32_t w1, uint32_t w2, uint32_t w3){
    b64buf[0]=b64d[w1&0x3f]; b64buf[1]=b64d[(w1&(0x3f<<6))>>6];b64buf[2]=b64d[(w1&(0x3f<<12))>>12];b64buf[3]=b64d[(w1&(0x3f<<18))>>18];b64buf[4]=b64d[(w1&(0x3f<<24))>>24];
    b64buf[5]=b64d[w2&0x3f]; b64buf[6]=b64d[(w2&(0x3f<<6))>>6];b64buf[7]=b64d[(w2&(0x3f<<12))>>12];b64buf[8]=b64d[(w2&(0x3f<<18))>>18];b64buf[9]=b64d[(w2&(0x3f<<24))>>24];    
    b64buf[10]=b64d[w3&0x3f]; b64buf[11]=b64d[(w3&(0x3f<<6))>>6];b64buf[12]=b64d[(w3&(0x3f<<12))>>12];b64buf[13]=b64d[(w3&(0x3f<<18))>>18];b64buf[14]=b64d[(w3&(0x3f<<24))>>24];    
    b64buf[15]=b64d[(w1&(0x03<<30))>>30 & (w2&(0x03<<30))>>28 & (w3&(0x03<<30))>>26];
    b64buf[16]=0;
    return b64buf;
}

void parse_parameters(char * param, void (*processor)(char*,char*), void (*err)()){
    int paramLen=strlen(param);
    if(paramLen > 0){
        char* curr=param;
        char* kvDel=param;
        char* endDel=param;
        char* key=param;
        char* val=param;
        while(curr-param<paramLen){
            kvDel = strchr(curr,':');
            if(kvDel!=NULL){
                    *kvDel=0; //make it 0 ending string
                if(strlen(curr)>0) key=curr;
                endDel = strchr(kvDel+1,',');
                if(endDel!=NULL) *endDel=0;//make val 0 ending string
                else{
                    endDel=strchr(kvDel+1,';');
                    if(endDel==NULL){
                        endDel = strchr(kvDel+1,'\n');
                        if(endDel == NULL){
                            err();
                            return;
                        }
                    }
                    //if here, we are good, mark end of val
                    *endDel = 0;                        
                }
                if(strlen(kvDel+1)>0) val=kvDel+1; //get value string
                //now we have key and value
                processor(key,val);
                curr = endDel+1;
            }else{
                err();
                return;
            }
        }
    }
}
