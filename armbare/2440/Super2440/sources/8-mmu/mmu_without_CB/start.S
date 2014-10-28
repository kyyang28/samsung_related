
.text
.global _start
_start:
    ldr sp, =4096                       
    bl  disable_watchdog               
    bl  sdram_init                      
    bl  copy2sdram               
    
    bl  create_page_table               
    bl  mmu_init                        
    ldr sp, =0xB4000000                
    ldr pc, =mymain

infinite_loop:
    b   infinite_loop

