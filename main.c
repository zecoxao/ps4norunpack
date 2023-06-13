#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>

#define ARRAYSIZE(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#define BLOCK_SIZE 0x200

unsigned char active;

typedef struct {
	uint32_t start_lba;
	uint32_t n_sectors;
	uint8_t flag1; // maybe part_id
	uint8_t flag2;
	uint16_t unknown;
	uint64_t padding;
} __attribute__((packed)) partition_t;

typedef struct {
	uint8_t magic[0x20]; // "SONY COMPUTER ENTERTAINMENT INC."
	uint32_t version; // 1
	uint32_t mbr1_start; // ex: 0x10
	uint32_t mbr2_start; // ex: 0x18
	uint32_t unk[4]; // ex: (1, 1, 8, 1)
	uint32_t reserved;
	uint8_t unused[0x1C0];
} __attribute__((packed)) master_block_v1_t;

typedef struct {
	uint8_t magic[0x20]; // "Sony Computer Entertainment Inc."
	uint32_t version; // 4
	uint32_t n_sectors;
	uint64_t reserved;
	uint32_t loader_start; // ex: 0x11, 0x309
	uint32_t loader_count; // ex: 0x267
	uint64_t reserved2;
	partition_t partitions[16];
} __attribute__((packed)) master_block_v4_t;

const char *part_code(int code) {
	static char *codes[] = {
		"empty",
		"idstorage",//1
		"sam_ipl",//2
		"core_os",//3
		"unknown",
		"unknown",
		"bd_hrl",//6
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"emc_ipl",//13
		"eap_kbl",//14
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"emc_ipl",//32
		"eap_kbl",//33
		"nvs",//34
		"unknown",
		"unknown",
		"unknown",
		"wifi",//38
		"vtrm",//39
		"empty",//40
		"C0050100",
	};
	return codes[code];
}

void unpack(master_block_v4_t *master, char* input, char* output){
	char buffer2[256];
	for (size_t i = 0; i < ARRAYSIZE(master->partitions); ++i) {
		partition_t *p = &master->partitions[i];
		printf("Partition %lu,  off=0x%08x, sz=0x%08x, flag1=0x%02x, flag2=0x%02x\n",i,  p->start_lba, p->n_sectors, p->flag1, p->flag2);
		sprintf(buffer2,"%s/%s-%d",output,part_code(p->flag1),p->flag2);
		printf("Unpacking partition %li  offset 0x%lx size 0x%lx to %s\n", i, (uint64_t)p->start_lba * BLOCK_SIZE, (uint64_t)p->n_sectors * BLOCK_SIZE, buffer2);
		
		unsigned char * buffer = (unsigned char *) malloc (p->n_sectors * BLOCK_SIZE);
		FILE *fp = fopen(input,"rb");
		uint64_t total = (uint64_t)(p->start_lba * BLOCK_SIZE);
		fseeko(fp,0x2000 + ((active/0x80) * 0x1000) + total,SEEK_SET);
		fread(buffer,(p->n_sectors * BLOCK_SIZE),1,fp);
		fclose(fp);
		FILE *fl = fopen(buffer2, "wb");
		fwrite(buffer,(p->n_sectors * BLOCK_SIZE),1,fl);
		fclose(fl);
		free(buffer);
		
		
	}
}

int main(int argc, char *argv[]){
	
	if (argc != 3){
		printf("usage: ps4norunpack [sflash0/sflash0s1.crypt] [outdir] \n");
		return 1;
	}
	
	FILE *fp = fopen(argv[1],"rb");
	
	fseek(fp,0x1000,SEEK_SET);
	
	
	fread(&active,1,1,fp);
	if(active==0x00){
		printf("active=%d\n",active);
		fseek(fp,0x2000,SEEK_SET);
	}else{
		printf("active=%d\n",active);
		fseek(fp,0x3000,SEEK_SET);
	}
	
	static master_block_v4_t master;
	fread(&master,1,sizeof(master),fp);
	fclose(fp);
	
	rmdir(argv[2]);
	mkdir(argv[2],S_IRWXU);
	
	unpack(&master,argv[1],argv[2]);

	return 0;
}