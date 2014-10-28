SECTIONS 
{
    . = 0x52000000;
    .text : 
    { 
        start.o 
        * (.text)
    } 
    
    . = ALIGN(4);
    .rodata :
    {
    	* (.rodata)
    }
    
    . = ALIGN(4);
	.data :
	{
		* (.data)
	}

    . = ALIGN(4);	
    
    __bss_start = . ;
    
	.bss :
	{
		* (.bss)     
		* (.common)
	}
	
	__bss_end = . ;   
}

