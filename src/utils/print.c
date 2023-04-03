/*******************************************************************************
* printf utilities (color, hex dump, ...)
	
	printf in color 
    Black \033[0;30m
    Red \033[0;31m
    Green \033[0;32m
    Yellow \033[0;33m
    Blue \033[0;34m
    Purple \033[0;35m
    Cyan \033[0;36m
    White \033[0;37m
	
*******************************************************************************/
#include "print.h"

#define printfred(x) printf("\033[0;31m")
#define printfgreen(x) printf("\033[0;32m")
//#define printfyellow(x) printf("\033[1;33m")
#define printfreset(x) printf("\033[0m");

/*******************************************************************************
*
*******************************************************************************/
void printf_red(char *format, ...) {
 va_list args;

 printfred();

 va_start(args, format);
 vprintf(format, args);
 va_end(args);

 printfreset();
}

/*******************************************************************************
*
*******************************************************************************/
void printf_green(char *format, ...) {
 va_list args;

 printfgreen();

 va_start(args, format);
 vprintf(format, args);
 va_end(args);

 printfreset();
}

/*******************************************************************************
* not really necessary, since the colored ones return to default...
*******************************************************************************/
/*void printf_default(char *format, ...) {
 va_list args;

 printfreset();

 va_start(args, format);
 vprintf(format, args);
 va_end(args);
}*/

/*******************************************************************************
*
*******************************************************************************/
/* To test
int main () {
  printf_red("NOT OK: this is not ok !\n");
  printf("OK: this is ok...\n");
  //WriteFrmtd("SUPPORTED: %d - 0x%02X\n", 1, 20);
  return 0;
}
*/

/*******************************************************************************
*
*******************************************************************************/
void printHexDump (FILE* fout, uint8_t* pdata, uint32_t size_in_bytes, uint8_t swapmode, uint8_t colwidth)
{//TODO: use the swapmode, Luke !
 uint32_t i,j,k;
 unsigned char c;

 if (!fout) return;

 //fprintf (fout, "begin data dump %d bytes\n", size_in_bytes);
 for (i=0; i < size_in_bytes;){
	//the data in hex
	for (j=i; (j < size_in_bytes) && (j < i+colwidth); j++){
		fprintf (fout, "%02X ", pdata[j]);
	}
	
	//add some spaces if we didn't finish a row of colwidth bytes
	if (j % colwidth){
		for (k = 0; k < colwidth - (j % colwidth); k++){
			fprintf(fout, "   ");
		}
	}
	//add some space between the 2 blocks
	fprintf(fout, "  ");
	
	//the data in visible chars (if ascii)
	for (j=i; (j < size_in_bytes) && (j < i+colwidth); j++){
		c = pdata[j];
		if ((c >= 0x20) && (c < 0x7F)) fprintf (fout, "%c", c);
		else fprintf (fout, "%c", ' ');
	}			
	i += colwidth;
	fprintf (fout, "\r\n");
 }		

}

