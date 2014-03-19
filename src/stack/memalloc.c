#include "compiler.h"               //compiler specific
#include <string.h>                 // has memset function
#include "lrwpan_config.h"
#include "hal.h"        //for global interrupt enable/disable
#include "halStack.h"
#include "memalloc.h"




//LRWPAN_HEAPSIZE must be < 64K, can allocate blocks up to this size
// format of block is
// free bit/15 bit size block_UINT8s | free bit/15 bit size block_UINT8s | etc..
// heap merging done on FREE, free adjacent blocks are merged.



//static UINT8 mem_heap[LRWPAN_HEAPSIZE];
#include "halHeapSpace.h"

typedef UINT16 MEMHDR;//�ڴ������ƣ��ڴ�ͷ('��־λ'+'�ڴ��С'��һ�������ֽ�)+�ڴ�ռ�

/*MEMHDR_FREE��־ȡֵ����          MEMHDR_SIZE����*/
/*'1'��ʾ����洢�ռ�Ϊ��          �ж��ٿռ����*/
/*'0'��ʾ����洢�ռ�����          ռ�ö��ٿռ�*/
#define MEMHDR_FREE_MASK 0x80   //����MEMHDR_FREE_MASK
#define MEMHDR_SIZE_MASK 0x7FFF //����MEMHDR_SIZE_MASK

#define MEMHDR_GET_FREE(x) (*(x+1)&MEMHDR_FREE_MASK)  //���MEMHDR_FREE��־λ
#define MEMHDR_CLR_FREE(x)  *(x+1) = *(x+1)&(~MEMHDR_FREE_MASK)  //���MEMHDR_FREE��־λ
#define MEMHDR_SET_FREE(x)  *(x+1) = *(x+1)|MEMHDR_FREE_MASK  //��MEMHDR_FREE��־λΪ1����7λ���ֲ���


UINT16 memhdr_get_size (UINT8 *ptr) {  //���ָ��ptrָ���UINT16��ֻҪ��15λ(*(ptr+1)*256+*ptr)
	UINT16 x;

	x = (UINT8) *ptr;    //ָ��PTRָ������ݴ����16λx�ĵͰ�λ
	x += ((UINT16) *(ptr+1)<< 8);  //�Ƚ�ָ��+1,�ٽ���ָ�����������8λ�����ڸղŵ�x�ĸ߰�λ
	x = x & 0x7FFF;   //�������������15λ��Ϊ�ڴ��Сֵ
	return(x);
}

void memhdr_set_size (UINT8 *ptr, UINT16 size) { //��ֵ(size)�浽ptr

	*ptr = (UINT8) size;   //ָ���ʼ����ǿ��ȡsize�ĵ�8λ
	ptr++;
	*ptr = *ptr & 0x80;  //clear size field ָ��ָ����������λ���ֲ��䣬��7λ��0
	*(ptr) += (size >> 8);  //add in size. ��size�ĸ�8λ����8λ��ӵ�ָ���������ȥ
}

/*
#if 0   //�������νṹ��˴����õ��������Ҫʱ�ٶ���λ1
typedef struct _MEMHDR {  //����ṹ��_MEMHDR
    UINT16 val;

    unsigned int free:1;  //free��ʾval�����λΪ��־λ
	unsigned int size:15;  //size��ʾval�ĵ�15λΪ�ڴ��Сֵ
}MEMHDR;
#endif
*/

#define MINSIZE 16+sizeof(MEMHDR)   //������С�ߴ磬��С�ߴ�Ϊ16��2=18�ֽ�


void MemInit (void) {  //�ڴ��ʼ����
	memset(mem_heap,0,LRWPAN_HEAPSIZE);  //��0�����ڴ棬����Ϊ1024��ָ��ָ��mem_heap
	MEMHDR_SET_FREE(((UINT8 *)&mem_heap[0]));  //��ָ��mem_heap[1]���������λ��1(��ʲô����???)����������
    memhdr_set_size(((UINT8 *)&mem_heap[0]),(LRWPAN_HEAPSIZE - sizeof(MEMHDR)));
	//�����ڴ��С1022�ֱ�浽mem_heap[0]��������һ��ָ��ָ���������,mem_heap[0]=0xFE,mem_heap[1]=0x83
}


/*��������:�����ڴ溯������С����ռ���16������ռ䲻��������NULL*/
UINT8 * MemAlloc (UINT16 size) {  //�ڴ����

	UINT8 *free_blk, *next_blk;
	UINT16 offset;
	UINT16 remainder;
    BOOL  gie_status;



    if (!size) return(NULL);      //illegal size ���sizeΪ��,����NULLֵ
	if (size < MINSIZE) size = MINSIZE;  //ȡsizeֵ��������18�ֽڻ�Сʱ��ȡ18�ֽ�

        SAVE_AND_DISABLE_GLOBAL_INTERRUPT(gie_status);  //�����жϱ�־����ֹ�ж�
	free_blk = mem_heap;   //��ʼ��ָ���block
	offset = 0;  //ƫ������ʼ��
	while (1) {
		if (MEMHDR_GET_FREE(((UINT8 *)free_blk)) &&
			(memhdr_get_size((UINT8 *)free_blk) >= size)) break; //����block��־λΪ1����block�Ĵ�С���ڵ���size������ѭ��
		//found block   advance to next block
		offset = offset + memhdr_get_size((UINT8 *)free_blk) + sizeof(MEMHDR);  //ȡoffset����СΪ��block�Ĵ�С����2�ֽ�
                if (offset >= LRWPAN_HEAPSIZE) {  //��ƫ�������ڵ���1024ʱ
                  RESTORE_GLOBAL_INTERRUPT(gie_status);  //�ָ�ȫ���ж�
                  return(NULL); // no free blocks  û�пյ�block
                }
		free_blk = mem_heap + offset;  //����ָ���block
	}
	remainder =  memhdr_get_size((UINT8 *)free_blk) - size;  //��ʼ��ʣ���ڴ��С����СΪ��block�Ĵ�С��ȥsize
	if (remainder < MINSIZE) {  //��ʣ���ڴ�С����С�ߴ�ʱ��
		//found block, mark as not-free   //����block,��־Ϊ�ǿ�
		MEMHDR_CLR_FREE((UINT8 *)free_blk);  //�����block�ı�־λ����־Ϊ�ǿ�
        RESTORE_GLOBAL_INTERRUPT(gie_status);  //�ָ�ȫ���ж�
		return(free_blk + sizeof(MEMHDR));  //����һ��ֵ����ֵ�Ĵ�СΪ��block�Ĵ�С��2�ֽ�
	}
	//remainder is large enough to support a new block   ʣ��ռ��㹻����֧��һ���µ�block
	//adjust allocated block to requested size   �����Ѿ������block��Ҫ��Ĵ�С
	memhdr_set_size(((UINT8 *)free_blk),size);  //���size�Ĵ�С
	//format next blk    ������һ��block
	next_blk = free_blk+size+sizeof(MEMHDR);    //ָ����һ��block��
	MEMHDR_SET_FREE((UINT8 *) next_blk);   //������һ��block�ı�־λΪ1����־Ϊ��
	memhdr_set_size(((UINT8 *) next_blk), (remainder - sizeof(MEMHDR)));  //�����һ��block�Ĵ�С

	MEMHDR_CLR_FREE((UINT8 *)free_blk); //mark allocated block as non-free   ��־�ѷ����blockΪ�ǿ�
    RESTORE_GLOBAL_INTERRUPT(gie_status);  //�ָ�ȫ���ж�
	return(free_blk + sizeof(MEMHDR));      //return new block  ����һ��ֵ����ֵ�Ĵ�СΪ��block�Ĵ�С��2�ֽ�
}

void MemFree(UINT8 *ptr) {  //�ͷ��ڴ�
	UINT8 *hdr;
	UINT16 offset, tmp;
        BOOL  gie_status;


    if (ptr == NULL) return;  //ָ��Ϊ�գ�����
    SAVE_AND_DISABLE_GLOBAL_INTERRUPT(gie_status);   //����
	hdr = ptr - sizeof(MEMHDR); //��ʼ��ָ��hdr����СΪptr-2�ֽ�
	//free this block   �ͷŸ�block
	MEMHDR_SET_FREE((UINT8 *)hdr);   //����hdrָ��ָ�����ݱ�־λΪ1����־Ϊ��
	//now merge    //�ϲ�
	offset = 0;    //��ʼ��offset
	hdr = mem_heap;   //����ָ��hdr
	//loop until blocks that can be merged are merged   //ѭ��ֱ���ܺϲ���block���ϲ���
	while (1) {
		if (MEMHDR_GET_FREE((UINT8 *)hdr)) {  //�����ñ�־λ����ʾΪ�գ�����־ָ��mem_heapΪ��
			//found a free block, see if we can merge with next block   ����һ����block�����ܷ�����һ��block�ϲ�
			tmp = offset +  memhdr_get_size((UINT8 *)hdr) + sizeof(MEMHDR);  //�����ݴ�ֵ����СΪoffset+2�ֽڣ�mem_heap��С
			if (tmp >= LRWPAN_HEAPSIZE) break; //at end of heap, exit loop �ݴ�ֵ����1024������ѭ��
			ptr = mem_heap + tmp; //point at next block   ָ��ָ����һ��block
			if (MEMHDR_GET_FREE((UINT8 *)ptr)) {   //��־��һ��blockΪ��
				//next block is free, do merge by adding size of next block  ��һ��blockΪ�գ��ϲ�
	            memhdr_set_size(((UINT8 *)hdr),(memhdr_get_size((UINT8 *)hdr)+ memhdr_get_size((UINT8 *)ptr)
					                            + sizeof(MEMHDR)));  //���úϲ�����ڴ��С�������ָ��hdr��
				// after merge, do not change offset, try to merge again �ϲ���block�󣬳����ٺϲ���һ��
				//next time through loop
				continue; //back to top of loop   �ص�ѭ����ʼ
			}			
		}
		// next block
		offset = offset + memhdr_get_size((UINT8 *)hdr) + sizeof(MEMHDR);  //ȡ�µ�block��offsetֵ
		if (offset >= LRWPAN_HEAPSIZE) break;  //at end of heap, exit loop   offset���ڵ���1024������ѭ��
		hdr = mem_heap + offset;  //����ָ��hdr
	}
	 RESTORE_GLOBAL_INTERRUPT(gie_status);  //�ָ�ȫ���ж�
}

#ifdef LRWPAN_COMPILER_NO_RECURSION  //������û�еݹ鷽ʽ

//this supports the HI-TECH compiler, which does not support recursion  �ñ�����֧��HI-TECH����������֧�ֵݹ�
//������γ���ִ�еĹ��̺������Ѿ�ע�͵�һ��������ע�Ͳμ����ϳ���

UINT16 ISR_memhdr_get_size (UINT8 *ptr) {
	UINT16 x;

	x = (UINT8) *ptr;
	x += ((UINT16) *(ptr+1)<< 8);
	x = x & 0x7FFF;
	return(x);
}

void ISR_memhdr_set_size (UINT8 *ptr, UINT16 size) {

	*ptr = (UINT8) size;
	ptr++;
	*ptr = *ptr & 0x80;  //clear size field
	*(ptr) += (size >> 8);  //add in size.
}



UINT8 * ISRMemAlloc (UINT16 size) {

	UINT8 *free_blk, *next_blk;
	UINT16 offset;
	UINT16 remainder;
        BOOL  gie_status;



        if (!size) return(NULL);      //illegal size
	if (size < MINSIZE) size = MINSIZE;

        SAVE_AND_DISABLE_GLOBAL_INTERRUPT(gie_status);
	free_blk = mem_heap;
	offset = 0;
	while (1) {
		if (MEMHDR_GET_FREE(((UINT8 *)free_blk)) &&
			(ISR_memhdr_get_size((UINT8 *)free_blk) >= size)) break; //found block
		//advance to next block
		offset = offset + ISR_memhdr_get_size((UINT8 *)free_blk) + sizeof(MEMHDR);
                if (offset >= LRWPAN_HEAPSIZE) {
                  RESTORE_GLOBAL_INTERRUPT(gie_status);
                  return(NULL); // no free blocks
                }
		free_blk = mem_heap + offset;
	}
	remainder =  ISR_memhdr_get_size((UINT8 *)free_blk) - size;
	if (remainder < MINSIZE) {
		//found block, mark as not-free
		MEMHDR_CLR_FREE((UINT8 *)free_blk);
        RESTORE_GLOBAL_INTERRUPT(gie_status);
		return(free_blk + sizeof(MEMHDR));
	}
	//remainder is large enough to support a new block
	//adjust allocated block to requested size
	ISR_memhdr_set_size(((UINT8 *)free_blk),size);
	//format next blk
	next_blk = free_blk+size+sizeof(MEMHDR);
	MEMHDR_SET_FREE((UINT8 *) next_blk);
	ISR_memhdr_set_size(((UINT8 *) next_blk), (remainder - sizeof(MEMHDR)));

	MEMHDR_CLR_FREE((UINT8 *)free_blk); //mark allocated block as non-free
    RESTORE_GLOBAL_INTERRUPT(gie_status);
	return(free_blk + sizeof(MEMHDR));      //return new block
}

void ISRMemFree(UINT8 *ptr) {
	UINT8 *hdr;
	UINT16 offset, tmp;
        BOOL  gie_status;


        SAVE_AND_DISABLE_GLOBAL_INTERRUPT(gie_status);
	hdr = ptr - sizeof(MEMHDR);
	//free this block
	MEMHDR_SET_FREE((UINT8 *)hdr);
	//now merge
	offset = 0;
	hdr = mem_heap;
	//loop until blocks that can be merged are merged
	while (1) {
		if (MEMHDR_GET_FREE((UINT8 *)hdr)) {
			//found a free block, see if we can merge with next block
			tmp = offset +  ISR_memhdr_get_size((UINT8 *)hdr) + sizeof(MEMHDR);
			if (tmp >= LRWPAN_HEAPSIZE) break; //at end of heap, exit loop
			ptr = mem_heap + tmp; //point at next block
			if (MEMHDR_GET_FREE((UINT8 *)ptr)) {
				//next block is free, do merge by adding size of next block
	            ISR_memhdr_set_size(((UINT8 *)hdr),(ISR_memhdr_get_size((UINT8 *)hdr)+ ISR_memhdr_get_size((UINT8 *)ptr)
					                            + sizeof(MEMHDR)));
				// after merge, do not change offset, try to merge again
				//next time through loop
				continue; //back to top of loop
			}			
		}
		// next block
		offset = offset + ISR_memhdr_get_size((UINT8 *)hdr) + sizeof(MEMHDR);
		if (offset >= LRWPAN_HEAPSIZE) break;  //at end of heap, exit loop
		hdr = mem_heap + offset;
	}
	 RESTORE_GLOBAL_INTERRUPT(gie_status);
}




#endif







