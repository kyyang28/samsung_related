
#include "s3c2440.h"


void disable_watchdog(void)
{
    WTCON = 0; 
}


void copy2sdram(void)
{
    unsigned int *pdwSrc  = (unsigned int *)2048;
    unsigned int *pdwDest = (unsigned int *)0x30004000;
    
    while (pdwSrc < (unsigned int *)4096)
    {
        *pdwDest = *pdwSrc;
        pdwDest++;
        pdwSrc++;
    }
}


void create_page_table(void)
{
    unsigned long virtualaddr, physicaladdr;
    unsigned long *mmu_tlb_base = (unsigned long *)MMU_TABLE_BASE;
    
    /*
     * Steppingstone����ʼ�����ַΪ0����һ���ֳ������ʼ���е�ַҲ��0��
     * Ϊ���ڿ���MMU���������е�һ���ֵĳ���
     * ��0��1M�������ַӳ�䵽ͬ���������ַ
     */
    virtualaddr = 0;
    physicaladdr = 0;
#ifdef ENABLE_CACHABLE_AND_BUFFERABLE	// Macro is defined in Makefile
    *(mmu_tlb_base + (virtualaddr >> 20)) = physicaladdr | MMU_SECDESC_WB;
#else
    *(mmu_tlb_base + (virtualaddr >> 20)) = physicaladdr | MMU_SECDESC;
#endif

    /*
     * 0x56000000��GPIO�Ĵ�������ʼ�����ַ��
     * GPFCON��GPFDAT�������Ĵ����������ַ0x56000050��0x56000054��
     * GPGCON��GPGDAT�������Ĵ����������ַ0x56000060��0x56000064��
     * Ϊ���ڵڶ����ֳ��������Ե�ַ0xD6000050��0xD6000054������GPFCON��GPFDAT��
     * �Լ��ڵڶ����ֳ��������Ե�ַ0xD6000060��0xD6000064������GPGCON��GPGDAT��
     * �Ѵ�0xD6000000��ʼ��1M�����ַ�ռ�ӳ�䵽��0x56000000��ʼ��1M�����ַ�ռ�
     */
    virtualaddr = (0x56000000 + 0x80000000);    // 0xD6000000
    physicaladdr = 0x56000000;
    *(mmu_tlb_base + (virtualaddr >> 20)) = physicaladdr | MMU_SECDESC;

    /*
     * SDRAM�������ַ��Χ��0x30000000��0x33FFFFFF��
     * �������ַ0xB0000000��0xB3FFFFFFӳ�䵽�����ַ0x30000000��0x33FFFFFF�ϣ�
     * �ܹ�64M���漰64����������
     */
    virtualaddr = (0x30000000 + 0x80000000);    // 0xB0000000
    physicaladdr = 0x30000000;
    while (virtualaddr < 0xB4000000)
    {
#ifdef ENABLE_CACHABLE_AND_BUFFERABLE
	    *(mmu_tlb_base + (virtualaddr >> 20)) = physicaladdr | MMU_SECDESC_WB;
#else
        *(mmu_tlb_base + (virtualaddr >> 20)) = physicaladdr | MMU_SECDESC;
#endif
	    virtualaddr += MMU_SECTION_SIZE;         // 0x100000;
        physicaladdr += MMU_SECTION_SIZE;        // 0x100000;
    }
}


void mmu_init(void)
{
    unsigned long ttb = 0x30000000;

// Ƕ���ࣺLINUX�ں���ȫע��
__asm__(
    "mov    r0, #0\n"
    "mcr    p15, 0, r0, c7, c7, 0\n"    /* ʹ��ЧICaches��DCaches */
    
    "mcr    p15, 0, r0, c7, c10, 4\n"   /* drain write buffer on v4 */
    "mcr    p15, 0, r0, c8, c7, 0\n"    /* ʹ��Чָ�����TLB */
    
    "mov    r4, %0\n"                   /* r4 = ҳ���ַ */
    "mcr    p15, 0, r4, c2, c0, 0\n"    /* ����ҳ���ַ�Ĵ��� */
    
    "mvn    r0, #0\n"                   
    "mcr    p15, 0, r0, c3, c0, 0\n"    /* ����ʿ��ƼĴ�����Ϊ0xFFFFFFFF��
                                         * ������Ȩ�޼�� 
                                         */    
    /* 
     * ���ڿ��ƼĴ������ȶ�����ֵ������������޸ĸ���Ȥ��λ��
     * Ȼ����д��
     */
    "mrc    p15, 0, r0, c1, c0, 0\n"    /* �������ƼĴ�����ֵ */
    
    /* ���ƼĴ����ĵ�16λ����Ϊ��.RVI ..RS B... .CAM
     * R : ��ʾ����Cache�е���Ŀʱʹ�õ��㷨��
     *     0 = Random replacement��1 = Round robin replacement
     * V : ��ʾ�쳣���������ڵ�λ�ã�
     *     0 = Low addresses = 0x00000000��1 = High addresses = 0xFFFF0000
     * I : 0 = �ر�ICaches��1 = ����ICaches
     * R��S : ������ҳ���е�������һ��ȷ���ڴ�ķ���Ȩ��
     * B : 0 = CPUΪС�ֽ���1 = CPUΪ���ֽ���
     * C : 0 = �ر�DCaches��1 = ����DCaches
     * A : 0 = ���ݷ���ʱ�����е�ַ�����飻1 = ���ݷ���ʱ���е�ַ������
     * M : 0 = �ر�MMU��1 = ����MMU
     */
    
    /*  
     * ���������Ҫ��λ����������Ҫ��������������    
     */
                                        /* .RVI ..RS B... .CAM */ 
    "bic    r0, r0, #0x3000\n"          /* ..11 .... .... .... ���V��Iλ */
    "bic    r0, r0, #0x0300\n"          /* .... ..11 .... .... ���R��Sλ */
    "bic    r0, r0, #0x0087\n"          /* .... .... 1... .111 ���B/C/A/M */

    /*
     * ������Ҫ��λ
     */
    "orr    r0, r0, #0x0002\n"          /* .... .... .... ..1. ���������� */
    "orr    r0, r0, #0x0004\n"          /* .... .... .... .1.. ����DCaches */
    "orr    r0, r0, #0x1000\n"          /* ...1 .... .... .... ����ICaches */
    "orr    r0, r0, #0x0001\n"          /* .... .... .... ...1 ʹ��MMU */
    
    "mcr    p15, 0, r0, c1, c0, 0\n"    /* ���޸ĵ�ֵд����ƼĴ��� */
    : /* ����� */
    : "r" (ttb) );
}


