#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <fileTestSuite/fileTestSuite.h>


int main(int argc, char **argv){
	printf("Not implemented yet!\n");
	if(argc != 2){
		printf("ftsBin2Json meta.ftsmata\n");
		return 1;
	}

	int fd = open(argv[1], 0);

	struct stat inpFileInfo;
	memset(&inpFileInfo, 0, sizeof(inpFileInfo));
	stat(argv[1], &inpFileInfo);

	uint8_t *map = NULL;
	map = (uint8_t *) mmap(NULL, inpFileInfo.st_size, PROT_READ, MAP_SHARED, fd, 0);
	uint8_t *upperBound = map + inpFileInfo.st_size;

	struct FTSMetadata_ptrs res;
	Metadata_parse(map, upperBound, &res);
	printf("{\n");
	printf("\t\"rawExt\": ");
	if(*res.rawExt.size){
		printf("\"%s\"", res.rawExt.str);
	}else{
		printf("\"\"");
	}
	printf(",\n\t\"processedExt\": ");
	if(*res.processedExt.size){
		printf("\"%s\"", res.processedExt.str);
	}else{
		printf("\"\"");
	}
	printf("\n}\n");
	munmap(map, inpFileInfo.st_size);
	close(fd);
	return 0;
}
