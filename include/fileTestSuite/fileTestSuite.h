#pragma once

#ifdef _MSC_VER
	#define LIBFILETESTSUITE_EXPORT_API __declspec(dllexport)
	#define LIBFILETESTSUITE_IMPORT_API __declspec(dllimport)
#else
	#ifdef _WIN32
		#ifdef __cplusplus
			#define LIBFILETESTSUITE_EXPORT_API [[gnu::dllexport]]
			#define LIBFILETESTSUITE_IMPORT_API [[gnu::dllimport]]
		#else
			#define LIBFILETESTSUITE_EXPORT_API __attribute__((dllexport))
			#define LIBFILETESTSUITE_IMPORT_API __attribute__((dllimport))
		#endif
	#else
		#ifdef __cplusplus
			#define LIBFILETESTSUITE_EXPORT_API [[gnu::visibility("default")]]
		#else
			#define LIBFILETESTSUITE_EXPORT_API __attribute__((visibility("default")))
		#endif
		#define LIBFILETESTSUITE_IMPORT_API
	#endif
#endif

#ifdef LIBFILETESTSUITE_EXPORTS
	#define LIBFILETESTSUITE_API LIBFILETESTSUITE_EXPORT_API
#else
	#define LIBFILETESTSUITE_API LIBFILETESTSUITE_IMPORT_API
#endif

#ifdef TESTABS_BACKEND_EXPORTS
	#define TESTABS_BACKEND_API LIBFILETESTSUITE_EXPORT_API
#else
	#define TESTABS_BACKEND_API LIBFILETESTSUITE_IMPORT_API
#endif


#include <stdint.h>
#include <stdlib.h>

#define FILETESTSUITE_PACKED __attribute__((packed))


enum {
	CURRENT_VERSION_MAJOR = 0,
	CURRENT_VERSION_MINOR = 0,
};

enum ParsingError{
	PARSING_OK=0,
	PARSING_ERROR_OUT_OF_BOUND=1,
	PARSING_ERROR_INVALID_SIGNATURE=2
};

const uint64_t META_SIGNATURE = 0x006174656d737466, // "ftsmeta\0"
ONTH_SIGNATURE = 0x0068746e6f737466; // "ftsonth\0"

struct FILETESTSUITE_PACKED Version{
	uint8_t minor;
	uint8_t major;
};


struct FILETESTSUITE_PACKED Header_raw{
	uint64_t signature;
	struct Version version;
};

struct FILETESTSUITE_PACKED PasStr_raw{
	uint8_t size;
	char str[0];
};

struct PasStr_ptrs{
	uint8_t *size;
	char *str;
};

struct FILETESTSUITE_PACKED Parameter_raw{
	uint8_t id, size;
	uint8_t value[0];
};

struct Parameter_ptrs{
	uint8_t *id, *size;
	uint8_t *value;
};

struct FILETESTSUITE_PACKED Parameters_raw{
	uint8_t params_count;
	uint8_t parameters[0];
};

struct Parameters_ptrs{
	struct Parameters_raw *p;
	struct Parameter_ptrs *parameters;
};

struct FilesSubset_ptrs{
	struct PasStr_ptrs globMask;
	struct Parameters_ptrs parameters;
};


struct FTSMetadata_ptrs{
	struct Header_raw *header;
	struct PasStr_ptrs rawExt;
	struct PasStr_ptrs processedExt;
	uint8_t *subSets_count;
	struct FilesSubset_ptrs * subSets;
};

struct FILETESTSUITE_PACKED OnthParam_raw{
	uint8_t id;
	uint8_t typ;
	struct PasStr_raw name;
};

struct OnthParam_ptrs{
	uint8_t *id;
	uint8_t *typ;
	struct PasStr_ptrs name;
};

struct FILETESTSUITE_PACKED Onthology_raw{
	struct Header_raw header;
	uint8_t params_count;
	uint8_t params[0];
};

struct Onthology_ptrs{
	struct Header_raw *header;
	uint8_t *params_count;
	void* params[0];
};

#ifdef __cplusplus
extern "C" {
#endif

LIBFILETESTSUITE_API enum ParsingError Metadata_parse(void *memory, void *upperBound, struct FTSMetadata_ptrs *res);

#ifdef __cplusplus
};
#endif
