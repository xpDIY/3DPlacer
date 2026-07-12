
#include "common.h"
#include <stdarg.h>
#include <string.h>

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

uint8_t calc_checksum(const char *data) {
    uint8_t sum = 0;
    while (*data) {
        sum += (uint8_t)(*data);
        data++;
    }
    return sum;
}

void parse_parameters(char * param, void (*processor)(char*,char*), void (*err)()){
    int paramLen=strlen(param);
    if(paramLen > 0){
        char* curr=param;
        // Detect format: if the first token (up to space/comma/semicolon)
        // contains ':', use colon format; otherwise use space format.
        char* probe = strchr(curr, ';');
        if (probe != NULL) *probe = 0; // temporarily null-terminate
        int hasColon = (strchr(curr, ':') != NULL);
        if (probe != NULL) *probe = ';'; // restore

        if (hasColon) {
            // Colon format: KEY:VALUE,KEY:VALUE;...
            char* kvDel;
            char* endDel;
            char* key;
            char* val;
            while(curr-param<paramLen){
                kvDel = strchr(curr,':');
                if(kvDel!=NULL){
                    *kvDel=0;
                    key = curr;
                    endDel = strchr(kvDel+1,',');
                    if(endDel!=NULL) *endDel=0;
                    else{
                        endDel=strchr(kvDel+1,';');
                        if(endDel==NULL){
                            endDel = strchr(kvDel+1,'\n');
                            if(endDel == NULL){
                                err();
                                return;
                            }
                        }
                        *endDel = 0;
                    }
                    val=kvDel+1;
                    processor(key,val);
                    curr = endDel+1;
                }else{
                    err();
                    return;
                }
            }
        } else {
            // Space format: KEYVALUE KEYVALUE;...  (key=letters, value=rest)
            char* endDel;
            char keyBuf[32];
            char valBuf[128];
            while(curr-param<paramLen && *curr != '\0'){
                // Skip leading spaces
                while (*curr == ' ') curr++;
                if (*curr == '\0' || *curr == ';') break;
                // Find end of token (space or semicolon)
                endDel = strchr(curr, ' ');
                char* semi = strchr(curr, ';');
                if (endDel == NULL || (semi != NULL && semi < endDel)) endDel = semi;
                if (endDel == NULL) endDel = curr + strlen(curr);
                // DEBUG: print raw token starting chars
                int tokLen = endDel - curr;
                // Split token: try 2-letter key first (TC, TR, PI, ST, RT, OX, OY, AD),
                // then fall back to single-letter key (R, C, N, H).
                int keyLen = 1;
                if (tokLen >= 2) {
                    char c0 = curr[0], c1 = curr[1];
                    if ((c0 == 'T' && (c1 == 'C' || c1 == 'R'))
                     || (c0 == 'P' && c1 == 'I')
                     || (c0 == 'S' && c1 == 'T')
                     || (c0 == 'R' && c1 == 'T')
                     || (c0 == 'O' && (c1 == 'X' || c1 == 'Y'))
                     || (c0 == 'A' && c1 == 'D')) {
                        keyLen = 2;
                    }
                }
                if (keyLen > 0) {
                    memcpy(keyBuf, curr, keyLen);
                    keyBuf[keyLen] = '\0';
                    int valLen = tokLen - keyLen;
                    if (valLen >= (int)sizeof(valBuf)) valLen = sizeof(valBuf)-1;
                    memcpy(valBuf, curr + keyLen, valLen);
                    valBuf[valLen] = '\0';
                    processor(keyBuf, valBuf);
                }
                curr = endDel;
                if (*curr == ';') break;
                if (*curr == ' ') curr++;
            }
        }
    }
}

int sprintfcs(char *buf,size_t bufsize, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, bufsize, fmt, args);
    va_end(args);

    if (n < 0 || (size_t)n >= bufsize) {
        // Formatting error or buffer too small
        return -1;
    }

    uint8_t checksum = calc_checksum(buf);
    // Append '*' and checksum as two hex digits
    int written = snprintf(buf + n, bufsize - n, "%02X", checksum);

    if (written < 0 || (size_t)(n + written) >= bufsize) {
        return -1;
    }

    return n + written;
}
