#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include <fileTestSuite/fileTestSuite.h>


uint8_t * root_ptr = 0;


size_t PasStr_getSize(struct PasStr_ptrs *v){
	uint8_t sz = *v->size;
	sz = (sz? sz + 1 : 0);  // 1 is for terminator
	return sizeof(struct PasStr_raw) + sz;
}

enum ParsingError PasStr_parse(void *memory, void *upperBound, struct PasStr_ptrs *res){
	memset(res, 0, sizeof(struct PasStr_ptrs));
	struct PasStr_raw *raw = (struct PasStr_raw *) memory;
	if((uint8_t *) raw + sizeof(struct PasStr_raw) > upperBound){
		return PARSING_ERROR_OUT_OF_BOUND;
	}
	res->size = &raw->size;
	res->str = (char *) &raw->str;
	if(raw->size){
		if( ((uint8_t *) res->str + *res->size) > upperBound){
			return PARSING_ERROR_OUT_OF_BOUND;
		}
	}
	return PARSING_OK;
}

void* PasStr_getLastOffset(struct PasStr_ptrs *v){
	uint8_t sz = *v->size;
	sz = (sz? sz + 1 : 0);  // 1 is for terminator
	return v->str + sz;
}

size_t Parameter_getSize(struct Parameter_ptrs *v){
	return sizeof(struct Parameter_raw) + *v->size;
}

void* Parameter_getLastOffset(struct Parameter_ptrs *v){
	return v->value + *v->size;
}

void* Parameters_getLastOffset(struct Parameters_ptrs *v){
	return Parameter_getLastOffset(&v->parameters[v->p->params_count - 1]);
}

size_t Parameters_getSize(struct Parameters_ptrs *v){
	return (size_t)((uint8_t*) Parameters_getLastOffset(v) - (uint8_t*) v->p);
}

size_t Parameters_getPtrsSize(struct Parameters_raw *v){
	return sizeof(struct Parameters_ptrs) + sizeof(struct Parameter_ptrs) * v->params_count;
}

size_t FilesSubset_getSize(struct FilesSubset_ptrs *v){
	return PasStr_getSize(&v->globMask) + Parameters_getSize(&v->parameters);
}

enum ParsingError Parameter_parse(void *memory, void *upperBound, struct Parameter_ptrs *res){
	memset(res, 0, sizeof(struct Parameter_ptrs));
	struct Parameter_raw *param = (struct Parameter_raw *) memory;
	res->id = &param->id;
	res->size = &param->size;
	res->value = (uint8_t*) &param->value;
	return PARSING_OK;
}

enum ParsingError Parameters_parse(void *memory, void *upperBound, struct Parameters_ptrs *res){
	memset(res, 0, sizeof(struct Parameters_ptrs));
	struct Parameters_raw *raw = (struct Parameters_raw *) memory;
	res->p = raw;

	uint8_t * lastParam = ((uint8_t *) raw) + sizeof(struct Parameters_raw);

	res->parameters = (struct Parameter_ptrs *) malloc(sizeof(struct Parameter_ptrs) * raw->params_count);

	enum ParsingError currentError = PARSING_OK;

	for(uint8_t i=0; i < raw->params_count; ++i){
		currentError = Parameter_parse(lastParam, upperBound, &res->parameters[i]);
		if(currentError){
			return currentError;
		}
		lastParam += Parameter_getSize(&res->parameters[i]);
	}

	return currentError;
};

enum ParsingError SubSet_parse(void *memory, void *upperBound, struct FilesSubset_ptrs *res){
	memset(res, 0, sizeof(struct FilesSubset_ptrs));
	enum ParsingError currentError = PARSING_OK;
	currentError = PasStr_parse(memory, upperBound, &res->globMask);
	if(currentError){
		return currentError;
	}

	currentError = Parameters_parse(PasStr_getLastOffset(&res->globMask), upperBound, &res->parameters);

	return currentError;
};

enum ParsingError Metadata_parse(void *memory, void *upperBound, struct FTSMetadata_ptrs *res){
	memset(res, 0, sizeof(struct FTSMetadata_ptrs));
	res->header = (struct Header_raw *) memory;
	root_ptr = memory;
	if(res->header->signature != META_SIGNATURE){
		return PARSING_ERROR_INVALID_SIGNATURE;
	}

	enum ParsingError currentError = PARSING_OK;
	currentError = PasStr_parse((uint8_t*) memory + sizeof(struct Header_raw), upperBound, &res->rawExt);
	if(currentError){
		return currentError;
	}
	currentError = PasStr_parse(PasStr_getLastOffset(&res->rawExt), upperBound, &res->processedExt);
	if(currentError){
		return currentError;
	}

	res->subSets_count = (uint8_t*) PasStr_getLastOffset(&res->processedExt);

	if(*res->subSets_count){
		uint8_t * lastSubSet = res->subSets_count + sizeof(*res->subSets_count);
		res->subSets = (struct FilesSubset_ptrs *) malloc(sizeof(struct FilesSubset_ptrs) * (*res->subSets_count));
		for(uint8_t i=0; i < *res->subSets_count; ++i){
			currentError = SubSet_parse(lastSubSet, upperBound, &(res->subSets[i]));
			if(currentError){
				return currentError;
			}
			lastSubSet += FilesSubset_getSize(&res->subSets[i]);
		}
	}

	return currentError;
}
