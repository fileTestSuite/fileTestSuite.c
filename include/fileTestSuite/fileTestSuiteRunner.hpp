#pragma once

#ifdef _MSC_VER
	#define LIBFILETESTSUITE_FWK_EXPORT_API __declspec(dllexport)
	#define LIBFILETESTSUITE_FWK_IMPORT_API __declspec(dllimport)
#else
	#ifdef _WIN32
		#define LIBFILETESTSUITE_FWK_EXPORT_API [[gnu::dllexport]]
		#define LIBFILETESTSUITE_FWK_IMPORT_API [[gnu::dllimport]]
	#else
		#define LIBFILETESTSUITE_FWK_EXPORT_API [[gnu::visibility("default")]]
		#define LIBFILETESTSUITE_FWK_IMPORT_API
	#endif
#endif

#ifdef LIBFILETESTSUITE_FWK_EXPORTS
	#define LIBFILETESTSUITE_FWK_API LIBFILETESTSUITE_FWK_EXPORT_API
#else
	#define LIBFILETESTSUITE_FWK_API LIBFILETESTSUITE_FWK_IMPORT_API
#endif

#ifdef LIBFILETESTSUITE_BACKEND_EXPORTS
	#define LIBFILETESTSUITE_BACKEND_API LIBFILETESTSUITE_FWK_EXPORT_API
#else
	#define LIBFILETESTSUITE_BACKEND_API LIBFILETESTSUITE_FWK_IMPORT_API
#endif

#include <TestAbs/TestAbs.hpp>

#include <string>
#include <span>
#include <cstdint>
#include <cstdlib>
#include <functional>

using FTSSpanT = std::span<uint8_t>;

struct LIBFILETESTSUITE_FWK_API FTSTestToolkit{
	ITestToolkit* tk;

	FTSTestToolkit(ITestToolkit* tk);
	~FTSTestToolkit() = default;

	void processOutOfBound(const char * ptrDescription, const char * whichBound, uint8_t * bound_ptr);

	void processInequality(size_t chunkSpanRelativeBegin, size_t chunkSpanRelativeEnd);
};

struct LIBFILETESTSUITE_FWK_API pseudospan: public FTSSpanT{
	pseudospan(uint8_t *first, uint8_t *last);
	pseudospan(uint8_t *buf, size_t len);

	bool check(FTSTestToolkit * tk, const char * ptrDescription, uint8_t *ptr);

	bool check(FTSTestToolkit * tk, const char * ptrDescription, pseudospan &checked);
};

struct LIBFILETESTSUITE_FWK_API OutputBufferTester{
	size_t cumLen = 0;
	pseudospan &etalonSpan;
	FTSTestToolkit * context;

	OutputBufferTester(pseudospan &etalonSpan, FTSTestToolkit * ctx);
	virtual ~OutputBufferTester();

	virtual void reset();
	virtual pseudospan getEtalonChunkBoundariesAndUpdateCumLen(size_t len);
	virtual bool testChunkProcessing(unsigned char *producedChunk, unsigned len);
};


struct LIBFILETESTSUITE_FWK_API IFileNamePairGen{
	using CallbackT = std::function<void(std::filesystem::path, std::filesystem::path)>;
	std::filesystem::path processedExt;

	IFileNamePairGen(std::string const & processedExt);
	virtual ~IFileNamePairGen() = default;

	bool isRespFile(std::filesystem::path cand) const;

	void processChallengeResponseFilePairs(std::filesystem::path subDataSetDir, CallbackT callback);

	virtual std::filesystem::path getChallFilePathFromResp(std::filesystem::path &fn) = 0;

};

struct LIBFILETESTSUITE_FWK_API FileNamePairGenCompPostfix: public IFileNamePairGen{
	FileNamePairGenCompPostfix(std::string const & processedExt):IFileNamePairGen(processedExt){}

	virtual std::filesystem::path getChallFilePathFromResp(std::filesystem::path &fn) override;
};

struct LIBFILETESTSUITE_FWK_API FileNamePairGenCompReplace: public IFileNamePairGen{
	std::filesystem::path rawExt;

	FileNamePairGenCompReplace(std::string const & processedExt, std::string const & rawExt): IFileNamePairGen(processedExt), rawExt("." + rawExt){}

	virtual std::filesystem::path getChallFilePathFromResp(std::filesystem::path &fn) override;
};

LIBFILETESTSUITE_FWK_API void crawlTestSuite(std::filesystem::path testSuiteDirPath, IFileNamePairGen::CallbackT callback);


struct LIBFILETESTSUITE_FWK_API TesteeSpecificContext{
	virtual ~TesteeSpecificContext() = default;

	virtual std::unique_ptr<OutputBufferTester> makeOutputBufferTester(pseudospan &etalonSpan, FTSTestToolkit * ctx);
	virtual void verifyAxillaryResults(FTSTestToolkit* ctx) = 0;
	virtual void executeTestee(pseudospan &challenge_data_span, OutputBufferTester &outBuffTester) = 0;
	bool shouldSwapChallengeResponse;
};


extern "C" {
LIBFILETESTSUITE_BACKEND_API TesteeSpecificContext* testeeSpecificContextFactory();
};

typedef decltype(testeeSpecificContextFactory) TesteeSpecificContextFactoryT;

